#include "common.h"
#include "vulkan_graphics.h"
#include "vulkan_command_buffer.h"
#include "vulkan_font.h"
#include "vulkan_texture.h"
#include "vulkan_utils.h"
#include "stb_image_write.h"

#include "moth_graphics/utils/circle_tessellation.h"

namespace moth_graphics::graphics::vulkan {
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
        m_defaultContext.m_target = m_swapchain->GetNextFramebuffer();
        if (m_defaultContext.m_target == nullptr) {
            VkSurfaceCapabilitiesKHR caps{};
            VkResult const capsResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                m_surfaceContext.GetVkPhysicalDevice(), m_vkSurface, &caps);
            if (capsResult != VK_SUCCESS) {
                spdlog::warn("Vulkan: vkGetPhysicalDeviceSurfaceCapabilitiesKHR returned {} — starting null frame",
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
            spdlog::warn("Vulkan: swapchain present returned {} — swapchain will be recreated on next frame",
                         static_cast<int>(presentResult));
        } else if (presentResult != VK_SUCCESS) {
            spdlog::error("Vulkan: vkQueuePresentKHR failed: {}", static_cast<int>(presentResult));
        }
    }

    void Graphics::SetBlendMode(BlendMode mode) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        context->m_currentBlendMode = mode;
    }

    void Graphics::SetColor(Color const& color) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        context->m_currentColor = color;
    }

    void Graphics::Clear() {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        DrawFillRectF({ { 0, 0 }, { static_cast<float>(context->m_logicalExtent.width), static_cast<float>(context->m_logicalExtent.height) } });
    }

    FloatMat4x4 Graphics::CurrentTransform() const {
        return m_currentTransform;
    }

    void Graphics::SetTransform(FloatMat4x4 const& transform) {
        m_currentTransform = transform;
    }

    void Graphics::DrawImage(Image const& image, IntVec2 const& pos, FloatVec2 const& pivot) {
        auto const imageWidth = image.GetWidth();
        auto const imageHeight = image.GetHeight();
        auto const offsetX = static_cast<int>(static_cast<float>(imageWidth) * pivot.x);
        auto const offsetY = static_cast<int>(static_cast<float>(imageHeight) * pivot.y);
        IntRect destRect = MakeRect(pos.x, pos.y, imageWidth, imageHeight);
        DrawImage(image, destRect - IntVec2{ offsetX, offsetY }, nullptr);
    }

    void Graphics::DrawImage(Image const& image, IntRect const& destRect, IntRect const* sourceRect) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        auto texture = std::dynamic_pointer_cast<Texture>(image.GetTexture());
        if (!texture) {
            return;
        }

        FloatRect fDestRect = static_cast<FloatRect>(destRect);

        FloatRect imageRect;
        if (sourceRect != nullptr) {
            imageRect = static_cast<FloatRect>(*sourceRect);
        } else {
            imageRect = MakeRect(0.0f, 0.0f, static_cast<float>(image.GetWidth()), static_cast<float>(image.GetHeight()));
        }

        FloatVec2 textureDimensions = FloatVec2{ texture->GetVkExtent().width, texture->GetVkExtent().height };
        imageRect += static_cast<FloatVec2>(image.GetSourceRect().topLeft);
        imageRect /= textureDimensions;

        auto const t = CurrentTransform();
        Vertex vertices[6];

        vertices[0].xy = t.TransformPoint({ fDestRect.topLeft.x, fDestRect.topLeft.y });
        vertices[0].uv = { imageRect.topLeft.x, imageRect.topLeft.y };
        vertices[0].color = context->m_currentColor;
        vertices[1].xy = t.TransformPoint({ fDestRect.bottomRight.x, fDestRect.topLeft.y });
        vertices[1].uv = { imageRect.bottomRight.x, imageRect.topLeft.y };
        vertices[1].color = context->m_currentColor;
        vertices[2].xy = t.TransformPoint({ fDestRect.topLeft.x, fDestRect.bottomRight.y });
        vertices[2].uv = { imageRect.topLeft.x, imageRect.bottomRight.y };
        vertices[2].color = context->m_currentColor;

        vertices[3].xy = t.TransformPoint({ fDestRect.topLeft.x, fDestRect.bottomRight.y });
        vertices[3].uv = { imageRect.topLeft.x, imageRect.bottomRight.y };
        vertices[3].color = context->m_currentColor;
        vertices[4].xy = t.TransformPoint({ fDestRect.bottomRight.x, fDestRect.bottomRight.y });
        vertices[4].uv = { imageRect.bottomRight.x, imageRect.bottomRight.y };
        vertices[4].color = context->m_currentColor;
        vertices[5].xy = t.TransformPoint({ fDestRect.bottomRight.x, fDestRect.topLeft.y });
        vertices[5].uv = { imageRect.bottomRight.x, imageRect.topLeft.y };
        vertices[5].color = context->m_currentColor;

        VkDescriptorSet descriptorSet = m_drawingShader->GetDescriptorSet(*texture);
        SubmitVertices(vertices, 6, ETopologyType::Triangles, descriptorSet);
    }

    void Graphics::DrawImageTiled(Image const& image, IntRect const& destRect, IntRect const* sourceRect, float scale) {
        IntRect const imageRect = MakeRect(0, 0, image.GetWidth(), image.GetHeight());
        if (sourceRect == nullptr) {
            sourceRect = &imageRect;
        }
        auto const imageWidth = static_cast<int>(static_cast<float>(sourceRect->w()) * scale);
        auto const imageHeight = static_cast<int>(static_cast<float>(sourceRect->h()) * scale);
        if (imageWidth <= 0 || imageHeight <= 0) {
            return;
        }
        for (auto y = destRect.topLeft.y; y < destRect.bottomRight.y; y += imageHeight) {
            for (auto x = destRect.topLeft.x; x < destRect.bottomRight.x; x += imageWidth) {
                IntRect const tiledDstRect{ { x, y }, { x + imageWidth, y + imageHeight } };
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
        vertices[1].uv = { 0, 0 };
        vertices[1].color = context->m_currentColor;
        vertices[2].xy = tr;
        vertices[2].uv = { 0, 0 };
        vertices[2].color = context->m_currentColor;
        vertices[3].xy = br;
        vertices[3].uv = { 0, 0 };
        vertices[3].color = context->m_currentColor;
        vertices[4].xy = br;
        vertices[4].uv = { 0, 0 };
        vertices[4].color = context->m_currentColor;
        vertices[5].xy = bl;
        vertices[5].uv = { 0, 0 };
        vertices[5].color = context->m_currentColor;
        vertices[6].xy = bl;
        vertices[6].uv = { 0, 0 };
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
        vertices[1].uv = { 0, 0 };
        vertices[1].color = context->m_currentColor;
        vertices[2].xy = bl;
        vertices[2].uv = { 0, 0 };
        vertices[2].color = context->m_currentColor;
        vertices[3].xy = bl;
        vertices[3].uv = { 0, 0 };
        vertices[3].color = context->m_currentColor;
        vertices[4].xy = br;
        vertices[4].uv = { 0, 0 };
        vertices[4].color = context->m_currentColor;
        vertices[5].xy = tr;
        vertices[5].uv = { 0, 0 };
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

        std::vector<Vertex> vertices(static_cast<size_t>(segments) * 3);
        FloatVec2 prev = t.TransformPoint({ center.x + radius, center.y });
        for (int i = 0; i < segments; ++i) {
            float const a = (kTwoPi * static_cast<float>(i + 1)) / static_cast<float>(segments);
            FloatVec2 const next = t.TransformPoint({
                center.x + (std::cos(a) * radius),
                center.y + (std::sin(a) * radius),
            });
            auto const base = static_cast<size_t>(i) * 3;
            vertices[base + 0].xy = centerW;
            vertices[base + 0].uv = { 0, 0 };
            vertices[base + 0].color = context->m_currentColor;
            vertices[base + 1].xy = prev;
            vertices[base + 1].uv = { 0, 0 };
            vertices[base + 1].color = context->m_currentColor;
            vertices[base + 2].xy = next;
            vertices[base + 2].uv = { 0, 0 };
            vertices[base + 2].color = context->m_currentColor;
            prev = next;
        }
        SubmitVertices(vertices.data(), static_cast<uint32_t>(vertices.size()), ETopologyType::Triangles);
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

        VkDescriptorSet const descriptorSet = m_drawingShader->GetDescriptorSet(*texture);
        SubmitVertices(vertices.data(), static_cast<uint32_t>(vertices.size()), ETopologyType::Triangles, descriptorSet);
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
        vertices[1].uv = { 0, 0 };
        vertices[1].color = context->m_currentColor;

        SubmitVertices(vertices, 2, ETopologyType::Lines);
    }

    void Graphics::DrawText(std::string_view text, IFont& font, IntRect const& destRect, TextHorizAlignment horizontalAlignment, TextVertAlignment verticalAlignment) {
        std::string const textStr(text);
        auto* vulkanFontPtr = dynamic_cast<Font*>(&font);
        if (vulkanFontPtr == nullptr) {
            spdlog::warn("DrawText: font is not a Vulkan Font; skipping");
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
                spdlog::warn("DrawText: glyph buffer full (1024 limit); remaining glyphs will not be rendered");
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
        auto& commandBuffer = context->m_target->GetCommandBuffer();
        if (clipRect != nullptr) {
            VkRect2D scissor;
            scissor.offset.x = clipRect->x();
            scissor.offset.y = clipRect->y();
            scissor.extent.width = clipRect->w();
            scissor.extent.height = clipRect->h();
            commandBuffer.SetScissor(scissor);
        } else {
            VkRect2D scissor;
            scissor.offset.x = 0;
            scissor.offset.y = 0;
            scissor.extent = context->m_target->GetVkExtent();
            commandBuffer.SetScissor(scissor);
        }
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
        // centre it. Bars outside the viewport stay black via the render pass
        // clear.
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

        // Store the projection on the context so StartCommands re-applies it
        // after a mid-frame RestartContext; ApplyProjection records it into the
        // command buffer here.
        context->m_logicalExtent = VkExtent2D{ static_cast<uint32_t>(logicalSize.x),
                                               static_cast<uint32_t>(logicalSize.y) };
        context->m_viewport = VkViewport{ offsetX, offsetY, fitWidth, fitHeight, 0.0f, 1.0f };
        context->m_scissor = VkRect2D{
            { static_cast<int32_t>(offsetX), static_cast<int32_t>(offsetY) },
            { static_cast<uint32_t>(fitWidth), static_cast<uint32_t>(fitHeight) } };
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
