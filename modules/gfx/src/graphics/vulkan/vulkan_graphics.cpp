#include "common.h"
#include "vulkan_graphics.h"
#include "vulkan_command_buffer.h"
#include "vulkan_font.h"
#include "vulkan_texture.h"
#include "vulkan_utils.h"
#include "stb_image_write.h"

#include "graphics/circle_tessellation.h"
#include "moth_graphics/utils/polygon_triangulation.h"

#include <algorithm>
#include <cmath>

namespace moth::gfx::vulkan {
    namespace {
        // Nested-clip intersection: clamp b inside a, collapsing to a zero-area
        // rect when the two don't overlap (so the resulting scissor clips away).
        IntRect IntersectRects(IntRect const& a, IntRect const& b) {
            IntRect r;
            r.topLeft.x = std::max(a.topLeft.x, b.topLeft.x);
            r.topLeft.y = std::max(a.topLeft.y, b.topLeft.y);
            r.bottomRight.x = std::min(a.bottomRight.x, b.bottomRight.x);
            r.bottomRight.y = std::min(a.bottomRight.y, b.bottomRight.y);
            r.bottomRight.x = std::max(r.topLeft.x, r.bottomRight.x);
            r.bottomRight.y = std::max(r.topLeft.y, r.bottomRight.y);
            return r;
        }
    }

    Graphics::Graphics(SurfaceContext& context, VkSurfaceKHR surface, uint32_t surfaceWidth, uint32_t surfaceHeight)
        : m_surfaceContext(context)
        , m_vkSurface(surface) {
        CreateRenderPass();
        CreateShaders();
        CreateDefaultImage();

        VkPipelineCacheCreateInfo cacheInfo{};
        cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        VkPipelineCache cache = VK_NULL_HANDLE;
        CHECK_VK_RESULT(vkCreatePipelineCache(m_surfaceContext.GetVkDevice(), &cacheInfo, nullptr, &cache));
        VkDevice const device = m_surfaceContext.GetVkDevice();
        m_vkPipelineCache = UniqueHandle<VkPipelineCache>(cache, [device](VkPipelineCache h) {
            vkDestroyPipelineCache(device, h, nullptr);
        });

        m_swapchain = std::make_unique<Swapchain>(m_surfaceContext, *m_renderPass, surface, VkExtent2D{ surfaceWidth, surfaceHeight });

        m_contextStack.push(nullptr);
    }

    Graphics::~Graphics() {
        vkDeviceWaitIdle(m_surfaceContext.GetVkDevice());

        if (m_overrideContext.m_vertexBuffer != nullptr && m_overrideContext.m_vertexBufferData != nullptr) {
            m_overrideContext.m_vertexBuffer->Unmap();
            m_overrideContext.m_vertexBufferData = nullptr;
        }
        if (m_defaultContext.m_vertexBuffer != nullptr && m_defaultContext.m_vertexBufferData != nullptr) {
            m_defaultContext.m_vertexBuffer->Unmap();
            m_defaultContext.m_vertexBufferData = nullptr;
        }
    }

    void Graphics::Begin() {
        auto const now = std::chrono::steady_clock::now();
        m_shaderLastDelta = std::chrono::duration<float>(now - m_shaderLastFrameTime).count();
        m_shaderLastFrameTime = now;
        ++m_frameCount;

        m_defaultContext.m_target = m_swapchain->GetNextFramebuffer();
        if (m_defaultContext.m_target == nullptr) {
            VkSurfaceCapabilitiesKHR caps{};
            VkResult const capsResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                m_surfaceContext.GetVkPhysicalDevice(), m_vkSurface, &caps);
            if (capsResult != VK_SUCCESS) {
                moth::core::log::warn("Vulkan: vkGetPhysicalDeviceSurfaceCapabilitiesKHR returned {} — starting null frame",
                             static_cast<int>(capsResult));
                m_contextStack.push(nullptr);
                return;
            }
            if (caps.currentExtent.width == 0 || caps.currentExtent.height == 0) {
                // Window minimised — start a null frame.
                m_contextStack.push(nullptr);
                return;
            }
            OnResize(m_vkSurface, caps.currentExtent.width, caps.currentExtent.height);
            m_defaultContext.m_target = m_swapchain->GetNextFramebuffer();
            if (m_defaultContext.m_target == nullptr) {
                m_contextStack.push(nullptr);
                return;
            }
        }

        VkFence cmdFence = m_defaultContext.m_target->GetFence().GetVkFence();
        vkResetFences(m_surfaceContext.GetVkDevice(), 1, &cmdFence);

        BeginContext(&m_defaultContext);

        // Wipe the full physical surface each frame.
        // The render pass uses LOAD_OP_LOAD and per-layer SetLogicalSize narrows
        // the viewport, so without this the region outside the logical viewport
        // keeps stale content from the previous frame in this swapchain image.
        // BeginContext leaves viewport/scissor/m_logicalExtent at the full target
        // extent, so this clears everything before any letterboxing applies.
        // Clear(Color) leaves the draw color untouched, so the frame's first draw
        // call doesn't inherit a stray black tint.
        Clear(BasicColors::Black);
    }

    void Graphics::End() {
        if (CurrentContext() == nullptr) {
            m_contextStack.pop();
            return;
        }
        EndContext();

        VkSemaphore waitSemaphores[] = { m_defaultContext.m_target->GetRenderFinishedSemaphore() };
        VkSwapchainKHR swapChains[] = { m_swapchain->GetVkSwapchain() };
        uint32_t swapchainIndices[] = { m_defaultContext.m_target->GetSwapchainIndex() };

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = waitSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = swapchainIndices;
        VkResult presentResult = vkQueuePresentKHR(m_surfaceContext.GetVkQueue(), &presentInfo);
        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
            // Swapchain is no longer optimal.  The next Begin() will detect the
            // out-of-date condition via vkAcquireNextImageKHR and recreate it.
            moth::core::log::warn("Vulkan: swapchain present returned {} — swapchain will be recreated on next frame",
                         static_cast<int>(presentResult));
        } else if (presentResult != VK_SUCCESS) {
            moth::core::log::error("Vulkan: vkQueuePresentKHR failed: {}", static_cast<int>(presentResult));
        }
    }

    void Graphics::SetBlendMode(BlendMode mode) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        context->m_currentBlendMode = mode;
    }

    void Graphics::PushBlendMode(BlendMode mode) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        context->m_blendModeStack.push(context->m_currentBlendMode);
        context->m_currentBlendMode = mode;
    }

    void Graphics::PopBlendMode() {
        auto* context = CurrentContext();
        if (context == nullptr || context->m_blendModeStack.empty()) {
            return;
        }
        context->m_currentBlendMode = context->m_blendModeStack.top();
        context->m_blendModeStack.pop();
    }

    void Graphics::SetColor(Color const& color) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        context->m_currentColor = color;
    }

    void Graphics::PushColor(Color const& color) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        context->m_colorStack.push(context->m_currentColor);
        context->m_currentColor = color;
    }

    void Graphics::PopColor() {
        auto* context = CurrentContext();
        if (context == nullptr || context->m_colorStack.empty()) {
            return;
        }
        context->m_currentColor = context->m_colorStack.top();
        context->m_colorStack.pop();
    }

    void Graphics::SetShader(moth::gfx::Shader const* shader) {
        if (shader == nullptr || !shader->IsValid()) {
            m_activeShader = moth::gfx::Shader{};
            return;
        }
        if (!std::dynamic_pointer_cast<VulkanShader>(shader->GetImpl())) {
            moth::core::log::warn("SetShader: shader has no Vulkan backend; ignoring");
            return;
        }
        m_activeShader = *shader;
    }

    void Graphics::Clear() {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        Clear(context->m_currentColor);
    }

    void Graphics::Clear(Color const& color) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        // Clearing must not mutate the active draw color, so save/restore around
        // the fill (DrawFillRectF tints by the current color).
        Color const savedColor = context->m_currentColor;
        context->m_currentColor = color;
        DrawFillRectF({ { 0, 0 }, { static_cast<float>(context->m_logicalExtent.width), static_cast<float>(context->m_logicalExtent.height) } });
        context->m_currentColor = savedColor;
    }

    FloatMat4x4 Graphics::CurrentTransform() const {
        return m_currentTransform;
    }

    void Graphics::SetTransform(FloatMat4x4 const& transform) {
        m_currentTransform = transform;
    }

    void Graphics::PushTransform(FloatMat4x4 const& transform) {
        m_transformStack.push(m_currentTransform);
        m_currentTransform = m_currentTransform * transform;
    }

    void Graphics::PopTransform() {
        if (m_transformStack.empty()) {
            return;
        }
        m_currentTransform = m_transformStack.top();
        m_transformStack.pop();
    }

    void Graphics::DrawImage(Image const& image, Transform2D const& transform, FloatVec2 const& pivot, bool flipX, bool flipY) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        auto texture = std::dynamic_pointer_cast<Texture>(image.GetTexture());
        if (!texture) {
            return;
        }

        auto const imageWidth = static_cast<float>(image.GetWidth());
        auto const imageHeight = static_cast<float>(image.GetHeight());
        FloatVec2 const pivotOffset{ imageWidth * pivot.x, imageHeight * pivot.y };

        // Sprite local -> world, composed on top of the current (e.g. camera) transform.
        auto const combined = CurrentTransform() * transform.ToMatrix(pivotOffset);

        FloatRect imageRect = MakeRect(0.0f, 0.0f, imageWidth, imageHeight);
        FloatVec2 const textureDimensions{
            static_cast<float>(texture->GetVkExtent().width),
            static_cast<float>(texture->GetVkExtent().height),
        };
        imageRect += static_cast<FloatVec2>(image.GetSourceRect().topLeft);
        imageRect /= textureDimensions;

        float u0 = imageRect.topLeft.x;
        float u1 = imageRect.bottomRight.x;
        float v0 = imageRect.topLeft.y;
        float v1 = imageRect.bottomRight.y;
        if (flipX) {
            std::swap(u0, u1);
        }
        if (flipY) {
            std::swap(v0, v1);
        }

        Vertex vertices[6];
        vertices[0].xy = combined.TransformPoint({ 0.0f, 0.0f });
        vertices[0].uv = { u0, v0 };
        vertices[0].color = context->m_currentColor;
        vertices[1].xy = combined.TransformPoint({ imageWidth, 0.0f });
        vertices[1].uv = { u1, v0 };
        vertices[1].color = context->m_currentColor;
        vertices[2].xy = combined.TransformPoint({ 0.0f, imageHeight });
        vertices[2].uv = { u0, v1 };
        vertices[2].color = context->m_currentColor;

        vertices[3].xy = combined.TransformPoint({ 0.0f, imageHeight });
        vertices[3].uv = { u0, v1 };
        vertices[3].color = context->m_currentColor;
        vertices[4].xy = combined.TransformPoint({ imageWidth, imageHeight });
        vertices[4].uv = { u1, v1 };
        vertices[4].color = context->m_currentColor;
        vertices[5].xy = combined.TransformPoint({ imageWidth, 0.0f });
        vertices[5].uv = { u1, v0 };
        vertices[5].color = context->m_currentColor;

        SubmitVertices(vertices, 6, ETopologyType::Triangles, texture);    }

    void Graphics::DrawImage(Image const& image, IntVec2 const& pos, FloatVec2 const& pivot) {
        auto const imageWidth = static_cast<float>(image.GetWidth());
        auto const imageHeight = static_cast<float>(image.GetHeight());
        auto const offsetX = imageWidth * pivot.x;
        auto const offsetY = imageHeight * pivot.y;
        DrawImage(image, MakeRect(static_cast<float>(pos.x) - offsetX, static_cast<float>(pos.y) - offsetY, imageWidth, imageHeight), nullptr);
    }

    void Graphics::DrawImage(Image const& image, FloatVec2 const& pos, FloatVec2 const& pivot) {
        auto const imageWidth = static_cast<float>(image.GetWidth());
        auto const imageHeight = static_cast<float>(image.GetHeight());
        auto const offsetX = imageWidth * pivot.x;
        auto const offsetY = imageHeight * pivot.y;
        DrawImage(image, MakeRect(pos.x - offsetX, pos.y - offsetY, imageWidth, imageHeight), nullptr);
    }

    void Graphics::DrawImage(Image const& image, IntRect const& destRect, IntRect const* sourceRect) {
        DrawImage(image, static_cast<FloatRect>(destRect), sourceRect);
    }

    void Graphics::DrawImage(Image const& image, FloatRect const& destRect, IntRect const* sourceRect) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        auto texture = std::dynamic_pointer_cast<Texture>(image.GetTexture());
        if (!texture) {
            return;
        }

        FloatRect imageRect;
        if (sourceRect != nullptr) {
            imageRect = static_cast<FloatRect>(*sourceRect);
        } else {
            imageRect = MakeRect(0.0f, 0.0f, static_cast<float>(image.GetWidth()), static_cast<float>(image.GetHeight()));
        }

        FloatVec2 textureDimensions = FloatVec2{ static_cast<float>(texture->GetVkExtent().width), static_cast<float>(texture->GetVkExtent().height) };
        imageRect += static_cast<FloatVec2>(image.GetSourceRect().topLeft);
        imageRect /= textureDimensions;

        auto const t = CurrentTransform();
        Vertex vertices[6];

        vertices[0].xy = t.TransformPoint({ destRect.topLeft.x, destRect.topLeft.y });
        vertices[0].uv = { imageRect.topLeft.x, imageRect.topLeft.y };
        vertices[0].color = context->m_currentColor;
        vertices[1].xy = t.TransformPoint({ destRect.bottomRight.x, destRect.topLeft.y });
        vertices[1].uv = { imageRect.bottomRight.x, imageRect.topLeft.y };
        vertices[1].color = context->m_currentColor;
        vertices[2].xy = t.TransformPoint({ destRect.topLeft.x, destRect.bottomRight.y });
        vertices[2].uv = { imageRect.topLeft.x, imageRect.bottomRight.y };
        vertices[2].color = context->m_currentColor;

        vertices[3].xy = t.TransformPoint({ destRect.topLeft.x, destRect.bottomRight.y });
        vertices[3].uv = { imageRect.topLeft.x, imageRect.bottomRight.y };
        vertices[3].color = context->m_currentColor;
        vertices[4].xy = t.TransformPoint({ destRect.bottomRight.x, destRect.bottomRight.y });
        vertices[4].uv = { imageRect.bottomRight.x, imageRect.bottomRight.y };
        vertices[4].color = context->m_currentColor;
        vertices[5].xy = t.TransformPoint({ destRect.bottomRight.x, destRect.topLeft.y });
        vertices[5].uv = { imageRect.bottomRight.x, imageRect.topLeft.y };
        vertices[5].color = context->m_currentColor;

        SubmitVertices(vertices, 6, ETopologyType::Triangles, texture);
    }

    void Graphics::DrawImageTiled(Image const& image, IntRect const& destRect, IntRect const* sourceRect, float scale) {
        DrawImageTiled(image, static_cast<FloatRect>(destRect), sourceRect, scale);
    }

    void Graphics::DrawImageTiled(Image const& image, FloatRect const& destRect, IntRect const* sourceRect, float scale) {
        IntRect const imageRect = MakeRect(0, 0, image.GetWidth(), image.GetHeight());
        if (sourceRect == nullptr) {
            sourceRect = &imageRect;
        }
        auto const imageWidth = static_cast<float>(sourceRect->w()) * scale;
        auto const imageHeight = static_cast<float>(sourceRect->h()) * scale;
        if (imageWidth <= 0.0f || imageHeight <= 0.0f) {
            return;
        }
        for (auto y = destRect.topLeft.y; y < destRect.bottomRight.y; y += imageHeight) {
            for (auto x = destRect.topLeft.x; x < destRect.bottomRight.x; x += imageWidth) {
                FloatRect const tiledDstRect{ { x, y }, { x + imageWidth, y + imageHeight } };
                DrawImage(image, tiledDstRect, sourceRect);
            }
        }
    }

    void Graphics::DrawRectF(FloatRect const& rect) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        auto const t = CurrentTransform();
        auto const tl = t.TransformPoint({ rect.topLeft.x, rect.topLeft.y });
        auto const tr = t.TransformPoint({ rect.bottomRight.x, rect.topLeft.y });
        auto const br = t.TransformPoint({ rect.bottomRight.x, rect.bottomRight.y });
        auto const bl = t.TransformPoint({ rect.topLeft.x, rect.bottomRight.y });
        Vertex vertices[8];

        vertices[0].xy = tl;
        vertices[0].uv = { 0, 0 };
        vertices[0].color = context->m_currentColor;
        vertices[1].xy = tr;
        vertices[1].uv = { 1, 0 };
        vertices[1].color = context->m_currentColor;
        vertices[2].xy = tr;
        vertices[2].uv = { 1, 0 };
        vertices[2].color = context->m_currentColor;
        vertices[3].xy = br;
        vertices[3].uv = { 1, 1 };
        vertices[3].color = context->m_currentColor;
        vertices[4].xy = br;
        vertices[4].uv = { 1, 1 };
        vertices[4].color = context->m_currentColor;
        vertices[5].xy = bl;
        vertices[5].uv = { 0, 1 };
        vertices[5].color = context->m_currentColor;
        vertices[6].xy = bl;
        vertices[6].uv = { 0, 1 };
        vertices[6].color = context->m_currentColor;
        vertices[7].xy = tl;
        vertices[7].uv = { 0, 0 };
        vertices[7].color = context->m_currentColor;

        SubmitVertices(vertices, 8, ETopologyType::Lines);
    }

    void Graphics::DrawFillRectF(FloatRect const& rect) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        auto const t = CurrentTransform();
        auto const tl = t.TransformPoint({ rect.topLeft.x, rect.topLeft.y });
        auto const tr = t.TransformPoint({ rect.bottomRight.x, rect.topLeft.y });
        auto const br = t.TransformPoint({ rect.bottomRight.x, rect.bottomRight.y });
        auto const bl = t.TransformPoint({ rect.topLeft.x, rect.bottomRight.y });
        Vertex vertices[6];

        vertices[0].xy = tl;
        vertices[0].uv = { 0, 0 };
        vertices[0].color = context->m_currentColor;
        vertices[1].xy = tr;
        vertices[1].uv = { 1, 0 };
        vertices[1].color = context->m_currentColor;
        vertices[2].xy = bl;
        vertices[2].uv = { 0, 1 };
        vertices[2].color = context->m_currentColor;
        vertices[3].xy = bl;
        vertices[3].uv = { 0, 1 };
        vertices[3].color = context->m_currentColor;
        vertices[4].xy = br;
        vertices[4].uv = { 1, 1 };
        vertices[4].color = context->m_currentColor;
        vertices[5].xy = tr;
        vertices[5].uv = { 1, 0 };
        vertices[5].color = context->m_currentColor;

        SubmitVertices(vertices, 6, ETopologyType::Triangles);
    }

    void Graphics::DrawFillCircleF(FloatVec2 const& center, float radius) {
        auto* context = CurrentContext();
        if (context == nullptr || radius <= 0.0f) {
            return;
        }
        int const segments = detail::CircleSegmentCount(radius);
        auto const t = CurrentTransform();
        auto const centerW = t.TransformPoint(center);
        constexpr float kTwoPi = 6.28318530718f;

        auto const uvForAngle = [](float a) {
            return FloatVec2{ (std::cos(a) + 1.0f) * 0.5f, (std::sin(a) + 1.0f) * 0.5f };
        };

        std::vector<Vertex> vertices(static_cast<size_t>(segments) * 3);
        float prevAngle = 0.0f;
        FloatVec2 prev = t.TransformPoint({ center.x + radius, center.y });
        for (int i = 0; i < segments; ++i) {
            float const a = (kTwoPi * static_cast<float>(i + 1)) / static_cast<float>(segments);
            FloatVec2 const next = t.TransformPoint({
                center.x + (std::cos(a) * radius),
                center.y + (std::sin(a) * radius),
            });
            auto const base = static_cast<size_t>(i) * 3;
            vertices[base + 0].xy = centerW;
            vertices[base + 0].uv = { 0.5f, 0.5f };
            vertices[base + 0].color = context->m_currentColor;
            vertices[base + 1].xy = prev;
            vertices[base + 1].uv = uvForAngle(prevAngle);
            vertices[base + 1].color = context->m_currentColor;
            vertices[base + 2].xy = next;
            vertices[base + 2].uv = uvForAngle(a);
            vertices[base + 2].color = context->m_currentColor;
            prev = next;
            prevAngle = a;
        }
        SubmitVertices(vertices.data(), static_cast<uint32_t>(vertices.size()), ETopologyType::Triangles);
    }

    void Graphics::DrawFillEllipseF(FloatVec2 const& center, float radiusX, float radiusY) {
        auto* context = CurrentContext();
        if (context == nullptr || radiusX <= 0.0f || radiusY <= 0.0f) {
            return;
        }
        int const segments = detail::CircleSegmentCount(std::max(radiusX, radiusY));
        auto const t = CurrentTransform();
        auto const centerW = t.TransformPoint(center);
        constexpr float kTwoPi = 6.28318530718f;

        auto const uvForAngle = [](float a) {
            return FloatVec2{ (std::cos(a) + 1.0f) * 0.5f, (std::sin(a) + 1.0f) * 0.5f };
        };

        std::vector<Vertex> vertices(static_cast<size_t>(segments) * 3);
        float prevAngle = 0.0f;
        FloatVec2 prev = t.TransformPoint({ center.x + radiusX, center.y });
        for (int i = 0; i < segments; ++i) {
            float const a = (kTwoPi * static_cast<float>(i + 1)) / static_cast<float>(segments);
            FloatVec2 const next = t.TransformPoint({
                center.x + (std::cos(a) * radiusX),
                center.y + (std::sin(a) * radiusY),
            });
            auto const base = static_cast<size_t>(i) * 3;
            vertices[base + 0].xy = centerW;
            vertices[base + 0].uv = { 0.5f, 0.5f };
            vertices[base + 0].color = context->m_currentColor;
            vertices[base + 1].xy = prev;
            vertices[base + 1].uv = uvForAngle(prevAngle);
            vertices[base + 1].color = context->m_currentColor;
            vertices[base + 2].xy = next;
            vertices[base + 2].uv = uvForAngle(a);
            vertices[base + 2].color = context->m_currentColor;
            prev = next;
            prevAngle = a;
        }
        SubmitVertices(vertices.data(), static_cast<uint32_t>(vertices.size()), ETopologyType::Triangles);
    }

    void Graphics::DrawFillPolygonF(FloatVec2 const* points, size_t count) {
        auto const vertices = TriangulatePolygon(points, count);
        DrawTrianglesF(vertices.data(), vertices.size());
    }

    void Graphics::DrawTrianglesF(FloatVec2 const* vertices, size_t count) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        size_t const triVerts = count - (count % 3);
        if (vertices == nullptr || triVerts < 3) {
            return;
        }

        // Normalize UVs across the triangle set's bounding box (0..1).
        FloatVec2 minPoint = vertices[0];
        FloatVec2 maxPoint = vertices[0];
        for (size_t i = 1; i < triVerts; ++i) {
            minPoint.x = std::min(minPoint.x, vertices[i].x);
            minPoint.y = std::min(minPoint.y, vertices[i].y);
            maxPoint.x = std::max(maxPoint.x, vertices[i].x);
            maxPoint.y = std::max(maxPoint.y, vertices[i].y);
        }
        FloatVec2 const extent = maxPoint - minPoint;

        auto const t = CurrentTransform();
        std::vector<Vertex> verts(triVerts);
        for (size_t i = 0; i < triVerts; ++i) {
            verts[i].xy = t.TransformPoint(vertices[i]);
            verts[i].uv = (extent.x > 0.0f && extent.y > 0.0f)
                ? (vertices[i] - minPoint) / extent
                : FloatVec2{ 0.0f, 0.0f };
            verts[i].color = context->m_currentColor;
        }
        SubmitVertices(verts.data(), static_cast<uint32_t>(verts.size()), ETopologyType::Triangles);
    }

    void Graphics::DrawTexturedTrianglesF(ITexture& texture, TexturedVertex const* vertices, size_t count) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        auto* vkTexture = dynamic_cast<Texture*>(&texture);
        if (vkTexture == nullptr) {
            return;
        }
        size_t const triVerts = count - (count % 3);
        if (vertices == nullptr || triVerts < 3) {
            return;
        }

        auto const t = CurrentTransform();
        std::vector<Vertex> verts(triVerts);
        for (size_t i = 0; i < triVerts; ++i) {
            verts[i].xy = t.TransformPoint(vertices[i].position);
            verts[i].uv = vertices[i].uv;
            verts[i].color = vertices[i].color;
        }

        // Non-owning alias — the caller keeps the texture alive for the frame.
        std::shared_ptr<Texture> alias(vkTexture, [](Texture*) {});
        SubmitVertices(verts.data(), static_cast<uint32_t>(verts.size()), ETopologyType::Triangles, alias);
    }

    void Graphics::DrawImageCircle(Image const& image, FloatVec2 const& center, float radius, IntRect const* sourceRect) {
        auto* context = CurrentContext();
        if (context == nullptr || radius <= 0.0f) {
            return;
        }
        auto texture = std::dynamic_pointer_cast<Texture>(image.GetTexture());
        if (!texture) {
            return;
        }

        FloatRect imageRect;
        if (sourceRect != nullptr) {
            imageRect = static_cast<FloatRect>(*sourceRect);
        } else {
            imageRect = MakeRect(0.0f, 0.0f, static_cast<float>(image.GetWidth()), static_cast<float>(image.GetHeight()));
        }
        FloatVec2 const textureDimensions{
            static_cast<float>(texture->GetVkExtent().width),
            static_cast<float>(texture->GetVkExtent().height),
        };
        imageRect += static_cast<FloatVec2>(image.GetSourceRect().topLeft);
        imageRect /= textureDimensions;

        int const segments = detail::CircleSegmentCount(radius);
        auto const t = CurrentTransform();
        constexpr float kTwoPi = 6.28318530718f;

        auto computeUv = [&](float lx, float ly) -> FloatVec2 {
            float const u = (lx - (center.x - radius)) / (2.0f * radius);
            float const v = (ly - (center.y - radius)) / (2.0f * radius);
            return {
                imageRect.topLeft.x + (u * (imageRect.bottomRight.x - imageRect.topLeft.x)),
                imageRect.topLeft.y + (v * (imageRect.bottomRight.y - imageRect.topLeft.y)),
            };
        };

        auto const centerW = t.TransformPoint(center);
        FloatVec2 const centerUv = computeUv(center.x, center.y);

        std::vector<Vertex> vertices(static_cast<size_t>(segments) * 3);
        float prevLx = center.x + radius;
        float prevLy = center.y;
        FloatVec2 prevW = t.TransformPoint({ prevLx, prevLy });
        FloatVec2 prevUv = computeUv(prevLx, prevLy);
        for (int i = 0; i < segments; ++i) {
            float const a = (kTwoPi * static_cast<float>(i + 1)) / static_cast<float>(segments);
            float const nextLx = center.x + (std::cos(a) * radius);
            float const nextLy = center.y + (std::sin(a) * radius);
            FloatVec2 const nextW = t.TransformPoint({ nextLx, nextLy });
            FloatVec2 const nextUv = computeUv(nextLx, nextLy);
            auto const base = static_cast<size_t>(i) * 3;
            vertices[base + 0].xy = centerW;
            vertices[base + 0].uv = centerUv;
            vertices[base + 0].color = context->m_currentColor;
            vertices[base + 1].xy = prevW;
            vertices[base + 1].uv = prevUv;
            vertices[base + 1].color = context->m_currentColor;
            vertices[base + 2].xy = nextW;
            vertices[base + 2].uv = nextUv;
            vertices[base + 2].color = context->m_currentColor;
            prevW = nextW;
            prevUv = nextUv;
        }

        SubmitVertices(vertices.data(), static_cast<uint32_t>(vertices.size()), ETopologyType::Triangles, texture);
    }

    void Graphics::DrawImage9Slice(Image const& image, FloatRect const& destRect, NineSliceBorders const& borders) {
        float const srcW = static_cast<float>(image.GetWidth());
        float const srcH = static_cast<float>(image.GetHeight());
        if (srcW <= 0.0f || srcH <= 0.0f) {
            return;
        }

        // Clamp borders so they stay within the source rect.
        float const left = std::clamp(borders.left, 0.0f, srcW);
        float const top = std::clamp(borders.top, 0.0f, srcH);
        float const right = std::clamp(borders.right, 0.0f, srcW - left);
        float const bottom = std::clamp(borders.bottom, 0.0f, srcH - top);

        float const dstW = destRect.bottomRight.x - destRect.topLeft.x;
        float const dstH = destRect.bottomRight.y - destRect.topLeft.y;

        // Destination border sizes: shrink proportionally if the destination is
        // smaller than the borders, so the corners never overlap.
        float leftDst = left;
        float rightDst = right;
        if (left + right > dstW) {
            float const factor = dstW / (left + right);
            leftDst *= factor;
            rightDst *= factor;
        }
        float topDst = top;
        float bottomDst = bottom;
        if (top + bottom > dstH) {
            float const factor = dstH / (top + bottom);
            topDst *= factor;
            bottomDst *= factor;
        }

        float const srcXs[4] = { 0.0f, left, srcW - right, srcW };
        float const srcYs[4] = { 0.0f, top, srcH - bottom, srcH };
        float const dstXs[4] = { 0.0f, leftDst, dstW - rightDst, dstW };
        float const dstYs[4] = { 0.0f, topDst, dstH - bottomDst, dstH };

        for (int iy = 0; iy < 3; ++iy) {
            for (int ix = 0; ix < 3; ++ix) {
                float const sw = srcXs[ix + 1] - srcXs[ix];
                float const sh = srcYs[iy + 1] - srcYs[iy];
                float const dw = dstXs[ix + 1] - dstXs[ix];
                float const dh = dstYs[iy + 1] - dstYs[iy];
                if (sw <= 0.0f || sh <= 0.0f || dw <= 0.0f || dh <= 0.0f) {
                    continue;
                }
                IntRect const src = MakeRect(static_cast<int>(srcXs[ix]), static_cast<int>(srcYs[iy]),
                                             static_cast<int>(sw), static_cast<int>(sh));
                FloatRect const dst = MakeRect(destRect.topLeft.x + dstXs[ix], destRect.topLeft.y + dstYs[iy], dw, dh);
                DrawImage(image, dst, &src);
            }
        }
    }

    void Graphics::DrawGradientRect(FloatRect const& destRect,
                                    Color startColor, Color endColor,
                                    FloatVec2 midpoint,
                                    float angle,
                                    float transitionLength) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        float const w = destRect.bottomRight.x - destRect.topLeft.x;
        float const h = destRect.bottomRight.y - destRect.topLeft.y;
        if (w <= 0.0f || h <= 0.0f) {
            return;
        }

        FloatVec2 const mp{
            destRect.topLeft.x + (midpoint.x * w),
            destRect.topLeft.y + (midpoint.y * h),
        };
        float const c = std::cos(angle);
        float const s = std::sin(angle);
        FloatVec2 const dir{ c, s };
        FloatVec2 const perp{ -s, c };

        float const projExtent = (std::abs(w * c) + std::abs(h * s));
        float const transitionPixels = std::max(0.0f, transitionLength) * projExtent;
        float const halfL = transitionPixels * 0.5f;

        float const cover = std::sqrt((w * w) + (h * h));

        auto const t = CurrentTransform();
        auto toWorld = [&](float lx, float ly) {
            FloatVec2 const local{
                mp.x + (dir.x * lx) + (perp.x * ly),
                mp.y + (dir.y * lx) + (perp.y * ly),
            };
            return t.TransformPoint(local);
        };

        auto submitQuad = [&](float x0, float x1, Color const& c0, Color const& c1) {
            if (x0 >= x1) {
                return;
            }
            auto const tl = toWorld(x0, -cover);
            auto const tr = toWorld(x1, -cover);
            auto const bl = toWorld(x0, +cover);
            auto const br = toWorld(x1, +cover);

            Vertex vertices[6];
            vertices[0].xy = tl;
            vertices[0].uv = { 0, 0 };
            vertices[0].color = c0;
            vertices[1].xy = tr;
            vertices[1].uv = { 0, 0 };
            vertices[1].color = c1;
            vertices[2].xy = bl;
            vertices[2].uv = { 0, 0 };
            vertices[2].color = c0;
            vertices[3].xy = bl;
            vertices[3].uv = { 0, 0 };
            vertices[3].color = c0;
            vertices[4].xy = tr;
            vertices[4].uv = { 0, 0 };
            vertices[4].color = c1;
            vertices[5].xy = br;
            vertices[5].uv = { 0, 0 };
            vertices[5].color = c1;

            SubmitVertices(vertices, 6, ETopologyType::Triangles);
        };

        submitQuad(-cover, -halfL, startColor, startColor);
        submitQuad(-halfL, +halfL, startColor, endColor);
        submitQuad(+halfL, +cover, endColor, endColor);
    }

    void Graphics::DrawLineF(FloatVec2 const& p0, FloatVec2 const& p1) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        auto const t = CurrentTransform();
        Vertex vertices[2];

        vertices[0].xy = t.TransformPoint(p0);
        vertices[0].uv = { 0, 0 };
        vertices[0].color = context->m_currentColor;
        vertices[1].xy = t.TransformPoint(p1);
        vertices[1].uv = { 1, 0 };
        vertices[1].color = context->m_currentColor;

        SubmitVertices(vertices, 2, ETopologyType::Lines);
    }

    void Graphics::DrawLineF(FloatVec2 const& p0, FloatVec2 const& p1, float thickness) {
        auto* context = CurrentContext();
        if (context == nullptr || thickness <= 0.0f) {
            return;
        }
        FloatVec2 const delta = p1 - p0;
        float const length = std::sqrt((delta.x * delta.x) + (delta.y * delta.y));
        if (length <= 0.0f) {
            return;
        }
        // Perpendicular unit vector scaled by half the thickness.
        FloatVec2 const normal{ (-delta.y / length) * (thickness * 0.5f), (delta.x / length) * (thickness * 0.5f) };

        auto const t = CurrentTransform();
        FloatVec2 const q0 = t.TransformPoint(p0 + normal);
        FloatVec2 const q1 = t.TransformPoint(p1 + normal);
        FloatVec2 const q2 = t.TransformPoint(p0 - normal);
        FloatVec2 const q3 = t.TransformPoint(p1 - normal);

        Vertex vertices[6];
        vertices[0].xy = q0;
        vertices[0].uv = { 0, 1 };
        vertices[1].xy = q1;
        vertices[1].uv = { 1, 1 };
        vertices[2].xy = q2;
        vertices[2].uv = { 0, 0 };
        vertices[3].xy = q2;
        vertices[3].uv = { 0, 0 };
        vertices[4].xy = q1;
        vertices[4].uv = { 1, 1 };
        vertices[5].xy = q3;
        vertices[5].uv = { 1, 0 };
        for (auto& vertex : vertices) {
            vertex.color = context->m_currentColor;
        }

        SubmitVertices(vertices, 6, ETopologyType::Triangles);
    }

    void Graphics::DrawText(std::string_view text, IFont& font, IntRect const& destRect, TextHorizAlignment horizontalAlignment, TextVertAlignment verticalAlignment) {
        std::string const textStr(text);
        auto* vulkanFontPtr = dynamic_cast<Font*>(&font);
        if (vulkanFontPtr == nullptr) {
            moth::core::log::warn("DrawText: font is not a Vulkan Font; skipping");
            return;
        }
        Font& vulkanFont = *vulkanFontPtr;
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        context->m_currentBlendMode = BlendMode::Alpha; // force alpha blending for text

        uint32_t const glyphStart = context->m_glyphCount;
        FontGlyphInstance* glyphInstances = static_cast<FontGlyphInstance*>(context->m_fontInstanceStagingBuffer->Map());

        auto const t = CurrentTransform();
        float const rotationRad = t.GetRotationDegrees() * kDegToRad;
        // use this to actually submit characters at a position
        auto SubmitCharacter = [&](uint32_t glyphIndex, FloatVec2 const& pos) {
            if (context->m_glyphCount >= 1024) {
                moth::core::log::warn("DrawText: glyph buffer full (1024 limit); remaining glyphs will not be rendered");
                return;
            }

            FontGlyphInstance* inst = &glyphInstances[context->m_glyphCount];
            inst->pos = pos;
            inst->glyphIndex = glyphIndex;
            inst->rotation = rotationRad;
            inst->color = context->m_currentColor;

            context->m_glyphCount++;
        };

        auto const lines = vulkanFont.WrapString(textStr, destRect.w());
        auto const singleLineHeight = vulkanFont.GetLineHeight();
        auto const singleLineDescent = vulkanFont.GetDescent();
        auto const linesHeight = static_cast<int32_t>(lines.size() * singleLineHeight);

        FloatVec2 penPos = static_cast<FloatVec2>(destRect.topLeft);

        switch (verticalAlignment) {
        case TextVertAlignment::Top:
            break;
        case TextVertAlignment::Middle:
            penPos.y += static_cast<float>(destRect.h() - linesHeight) / 2.0f;
            break;
        case TextVertAlignment::Bottom:
            penPos.y += static_cast<float>(destRect.h() - linesHeight);
            break;
        }

        // move down to the bottom of the line, minus the descent value (so the descent of the glyphs dont extend past the whole line)
        penPos.y += static_cast<float>(singleLineHeight + singleLineDescent);

        // render lines one by one
        for (const auto& line : lines) {
            auto const shapeInfo = vulkanFont.ShapeString(line.text);

            switch (horizontalAlignment) {
            case TextHorizAlignment::Left:
                penPos.x = static_cast<float>(destRect.topLeft.x);
                break;
            case TextHorizAlignment::Center:
                penPos.x = static_cast<float>(destRect.topLeft.x) + (static_cast<float>(destRect.w() - line.lineWidth) / 2.0f);
                break;
            case TextHorizAlignment::Right:
                penPos.x = static_cast<float>(destRect.bottomRight.x) - static_cast<float>(line.lineWidth);
                break;
            }

            for (auto const& info : shapeInfo) {
                if (info.glyphIndex >= 0) {
                    auto const bearing = static_cast<FloatVec2>(vulkanFont.GetGlyphBearing(info.glyphIndex));
                    auto const offset = static_cast<FloatVec2>(info.offset);
                    auto const glyphPos = t.TransformPoint(penPos + bearing + offset);
                    SubmitCharacter(static_cast<uint32_t>(info.glyphIndex), glyphPos);
                }
                penPos.x += static_cast<float>(info.advance.x);
            }

            penPos.y += static_cast<float>(singleLineHeight);
        }

        context->m_fontInstanceStagingBuffer->Unmap();

        uint32_t const glyphCount = context->m_glyphCount - glyphStart;
        if (glyphCount != 0u) {
            auto& commandBuffer = context->m_target->GetCommandBuffer();

            FlushPendingBatch();
            commandBuffer.BindVertexBuffer(*context->m_fontInstanceBuffer, 0);

            auto const& pipeline = GetCurrentFontPipeline();
            if (context->m_currentPipelineId != pipeline.m_hash) {
                commandBuffer.BindPipeline(pipeline);
                context->m_currentPipelineId = pipeline.m_hash;
            }

            commandBuffer.BindDescriptorSet(*m_fontShader, vulkanFont.GetVKDescriptorSetForShader(*m_fontShader), 0);
            commandBuffer.Draw(4, 0, glyphCount, glyphStart);
        }
    }

    void Graphics::SetClip(IntRect const* clipRect) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        FlushPendingBatch();
        // Absolute clip: discard any nested clip state and set (or clear) the clip.
        while (!context->m_clipStack.empty()) {
            context->m_clipStack.pop();
        }
        context->m_clipRect = clipRect != nullptr ? std::optional<IntRect>(*clipRect) : std::nullopt;
        ApplyClipScissor();
    }

    void Graphics::PushClip(IntRect const& rect) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        FlushPendingBatch();
        context->m_clipStack.push(context->m_clipRect);
        context->m_clipRect = context->m_clipRect
            ? std::optional<IntRect>(IntersectRects(*context->m_clipRect, rect))
            : std::optional<IntRect>(rect);
        ApplyClipScissor();
    }

    void Graphics::PopClip() {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        FlushPendingBatch();
        if (context->m_clipStack.empty()) {
            context->m_clipRect.reset();
        } else {
            context->m_clipRect = context->m_clipStack.top();
            context->m_clipStack.pop();
        }
        ApplyClipScissor();
    }

    void Graphics::ApplyClipScissor() {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        auto& commandBuffer = context->m_target->GetCommandBuffer();
        if (!context->m_clipRect) {
            // Restore the letterbox scissor (the no-clip state set by
            // SetLogicalSize / BeginContext), not the raw physical extent.
            commandBuffer.SetScissor(context->m_scissor);
            return;
        }
        IntRect const& clipRect = *context->m_clipRect;

        // The clip rect arrives in logical coordinates. Geometry is mapped
        // logical -> physical through the letterbox viewport (scale + offset
        // from SetLogicalSize); the scissor must follow the same mapping, or
        // it lands in the wrong place at the wrong size whenever the logical
        // size differs from the physical extent. Then clamp to the letterbox
        // scissor so it never spills into the black bars.
        float const scaleX = context->m_logicalExtent.width > 0
            ? context->m_viewport.width / static_cast<float>(context->m_logicalExtent.width)
            : 1.0f;
        float const scaleY = context->m_logicalExtent.height > 0
            ? context->m_viewport.height / static_cast<float>(context->m_logicalExtent.height)
            : 1.0f;

        float const left = context->m_viewport.x + (static_cast<float>(clipRect.x()) * scaleX);
        float const top = context->m_viewport.y + (static_cast<float>(clipRect.y()) * scaleY);
        float const right = left + (static_cast<float>(clipRect.w()) * scaleX);
        float const bottom = top + (static_cast<float>(clipRect.h()) * scaleY);

        VkRect2D const& bounds = context->m_scissor;
        int32_t const boundsLeft = bounds.offset.x;
        int32_t const boundsTop = bounds.offset.y;
        int32_t const boundsRight = bounds.offset.x + static_cast<int32_t>(bounds.extent.width);
        int32_t const boundsBottom = bounds.offset.y + static_cast<int32_t>(bounds.extent.height);

        int32_t const clampedLeft = std::min(std::max(static_cast<int32_t>(std::floor(left)), boundsLeft), boundsRight);
        int32_t const clampedTop = std::min(std::max(static_cast<int32_t>(std::floor(top)), boundsTop), boundsBottom);
        int32_t const clampedRight = std::min(std::max(static_cast<int32_t>(std::ceil(right)), boundsLeft), boundsRight);
        int32_t const clampedBottom = std::min(std::max(static_cast<int32_t>(std::ceil(bottom)), boundsTop), boundsBottom);

        VkRect2D scissor;
        scissor.offset.x = clampedLeft;
        scissor.offset.y = clampedTop;
        scissor.extent.width = static_cast<uint32_t>(clampedRight - clampedLeft);
        scissor.extent.height = static_cast<uint32_t>(clampedBottom - clampedTop);
        commandBuffer.SetScissor(scissor);
    }

    std::unique_ptr<ITarget> Graphics::CreateTarget(int width, int height) {
        return std::make_unique<Framebuffer>(m_surfaceContext, width, height, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, m_rtRenderPass->GetRenderPass());
    }

    void Graphics::Drain() {
        vkDeviceWaitIdle(m_surfaceContext.GetVkDevice());
        if (m_swapchain) {
            m_swapchain->ResetCommandBuffers();
        }
    }

    void Graphics::Flush() {
        if (m_contextStack.empty() || m_contextStack.top() == nullptr) {
            return;
        }
        FlushPendingBatch();
        // Foreign code is about to bind its own pipeline; forget our cached
        // pipeline id so the next moth draw rebinds.
        m_contextStack.top()->m_currentPipelineId = 0;
    }

    bool Graphics::IsRenderTarget() const {
        return m_contextStack.top() == &m_overrideContext;
    }

    ITarget* Graphics::GetTarget() {
        return m_overrideContext.m_target;
    }

    void Graphics::SetTarget(ITarget* target) {
        if (IsRenderTarget()) {
            EndContext();
        }

        if (target != nullptr) {
            m_overrideContext.m_target = dynamic_cast<Framebuffer*>(target);
            assert(m_overrideContext.m_target);
            VkFence fence = m_overrideContext.m_target->GetFence().GetVkFence();
            vkWaitForFences(m_surfaceContext.GetVkDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
            vkResetFences(m_surfaceContext.GetVkDevice(), 1, &fence);
            BeginContext(&m_overrideContext);
        }
    }

    void Graphics::SetLogicalSize(IntVec2 const& logicalSize) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        if (logicalSize.x <= 0 || logicalSize.y <= 0) {
            return;
        }

        // Flush any queued geometry under the previous viewport/scissor state
        // before mutating it, so existing batches still render with the
        // settings they were authored against.
        FlushPendingBatch();

        // Letterbox: fit the logical aspect inside the physical extent and
        // centre it. Bars outside the viewport stay black via the full-surface
        // clear in Begin() (the render pass itself uses LOAD_OP_LOAD).
        VkExtent2D const physical = context->m_target->GetVkExtent();
        float const logicalAspect = static_cast<float>(logicalSize.x) / static_cast<float>(logicalSize.y);
        float const physicalAspect = static_cast<float>(physical.width) / static_cast<float>(physical.height);

        float fitWidth = static_cast<float>(physical.width);
        float fitHeight = static_cast<float>(physical.height);
        if (physicalAspect > logicalAspect) {
            fitWidth = fitHeight * logicalAspect;
        } else {
            fitHeight = fitWidth / logicalAspect;
        }
        float const offsetX = (static_cast<float>(physical.width) - fitWidth) * 0.5f;
        float const offsetY = (static_cast<float>(physical.height) - fitHeight) * 0.5f;

        // Snap the letterbox rect to whole pixels and derive both the viewport
        // and the scissor from the same integers. The viewport is float and the
        // scissor is int; if they disagree by a sub-pixel at the edge, the
        // content rasterises against one grid while the scissor clips against
        // another, leaving a one-pixel sliver of content detached at the
        // letterbox boundary. Clamp the extent so offset + size never exceeds
        // the physical surface after rounding.
        int32_t const intOffsetX = static_cast<int32_t>(std::lround(offsetX));
        int32_t const intOffsetY = static_cast<int32_t>(std::lround(offsetY));
        int32_t const intFitWidth = std::min(static_cast<int32_t>(std::lround(fitWidth)),
                                             static_cast<int32_t>(physical.width) - intOffsetX);
        int32_t const intFitHeight = std::min(static_cast<int32_t>(std::lround(fitHeight)),
                                              static_cast<int32_t>(physical.height) - intOffsetY);

        // Store the projection on the context so StartCommands re-applies it
        // after a mid-frame RestartContext; ApplyProjection records it into the
        // command buffer here.
        context->m_logicalExtent = VkExtent2D{ static_cast<uint32_t>(logicalSize.x),
                                               static_cast<uint32_t>(logicalSize.y) };
        context->m_viewport = VkViewport{ static_cast<float>(intOffsetX), static_cast<float>(intOffsetY),
                                          static_cast<float>(intFitWidth), static_cast<float>(intFitHeight),
                                          0.0f, 1.0f };
        context->m_scissor = VkRect2D{
            { intOffsetX, intOffsetY },
            { static_cast<uint32_t>(intFitWidth), static_cast<uint32_t>(intFitHeight) } };
        ApplyProjection();
    }

    void Graphics::OnResize(VkSurfaceKHR surface, uint32_t surfaceWidth, uint32_t surfaceHeight) {
        vkDeviceWaitIdle(m_surfaceContext.GetVkDevice());
        m_swapchain.reset();
        m_swapchain = std::make_unique<Swapchain>(m_surfaceContext, *m_renderPass, surface, VkExtent2D{ surfaceWidth, surfaceHeight });
    }

    std::unique_ptr<IGraphics> CreateGraphics(
        SurfaceContext& surfaceContext, VkSurfaceKHR surface,
        uint32_t surfaceWidth, uint32_t surfaceHeight) {
        return std::make_unique<Graphics>(surfaceContext, surface, surfaceWidth, surfaceHeight);
    }
}
