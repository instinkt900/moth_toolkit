#include "common.h"
#include "vulkan_graphics.h"
#include "vulkan_command_buffer.h"
#include "vulkan_shader_object.h"
#include "vulkan_utils.h"
#include "moth/core/input.h"

#include <cstring>

namespace moth::gfx::graphics::vulkan {
    void Graphics::DrawShader(graphics::Shader const& shader) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }
        FloatRect const fullscreen{
            { 0.0f, 0.0f },
            { static_cast<float>(context->m_logicalExtent.width), static_cast<float>(context->m_logicalExtent.height) },
        };
        DrawShader(shader, fullscreen);
    }

    void Graphics::DrawShader(graphics::Shader const& shader, FloatRect const& destRect) {
        auto* context = CurrentContext();
        if (context == nullptr) {
            return;
        }

        auto impl = std::dynamic_pointer_cast<VulkanShader>(shader.GetImpl());
        if (!impl) {
            spdlog::warn("DrawShader: shader has no Vulkan backend; skipping");
            return;
        }

        // Fill the Shadertoy built-ins (iTime/iResolution/iMouse).
        VulkanShader::Builtins builtins{};
        builtins.resolution[0] = static_cast<float>(context->m_logicalExtent.width);
        builtins.resolution[1] = static_cast<float>(context->m_logicalExtent.height);
        builtins.resolution[2] = 1.0f;
        builtins.resolution[3] = 1.0f;
        builtins.time[0] = std::chrono::duration<float>(std::chrono::steady_clock::now() - m_shaderStartTime).count();
        builtins.time[1] = m_shaderLastDelta;
        builtins.time[2] = static_cast<float>(m_frameCount);
        builtins.time[3] = 0.0f;
        auto const& input = moth::core::Input::Get();
        auto const mousePos = input.GetMousePos();
        builtins.mouse[0] = mousePos.x;
        builtins.mouse[1] = mousePos.y;
        builtins.mouse[2] = input.IsMouseButtonDown(moth::core::MouseButton::Left) ? 1.0f : 0.0f;
        builtins.mouse[3] = input.IsMouseButtonDown(moth::core::MouseButton::Right) ? 1.0f : 0.0f;

        void* const mapped = impl->GetBuiltinsBuffer().Map();
        std::memcpy(mapped, &builtins, sizeof(builtins));
        impl->GetBuiltinsBuffer().Unmap();

        // Resolve the bound channel textures (null -> default 1x1 image).
        std::array<std::shared_ptr<Texture>, 4> channels{};
        auto const& shaderChannels = shader.GetChannels();
        for (std::size_t i = 0; i < channels.size(); ++i) {
            channels[i] = std::dynamic_pointer_cast<Texture>(shaderChannels[i]);
        }

        // Build a quad covering destRect, transformed into physical space.
        auto const t = CurrentTransform();
        FloatVec2 const tl = t.TransformPoint({ destRect.topLeft.x, destRect.topLeft.y });
        FloatVec2 const tr = t.TransformPoint({ destRect.bottomRight.x, destRect.topLeft.y });
        FloatVec2 const bl = t.TransformPoint({ destRect.topLeft.x, destRect.bottomRight.y });
        FloatVec2 const br = t.TransformPoint({ destRect.bottomRight.x, destRect.bottomRight.y });

        Vertex vertices[6]{
            { tl, { 0.0f, 0.0f }, BasicColors::White },
            { tr, { 1.0f, 0.0f }, BasicColors::White },
            { bl, { 0.0f, 1.0f }, BasicColors::White },
            { bl, { 0.0f, 1.0f }, BasicColors::White },
            { br, { 1.0f, 1.0f }, BasicColors::White },
            { tr, { 1.0f, 0.0f }, BasicColors::White },
        };

        FlushPendingBatch();

        // Ensure the vertex buffer has room; restart the frame context if not.
        std::size_t const vertexCapacity = context->m_vertexBuffer->GetSize() / sizeof(Vertex);
        if (static_cast<std::size_t>(context->m_vertexCount) + 6 > vertexCapacity) {
            RestartContext();
            context = CurrentContext();
        }

        auto& commandBuffer = context->m_target->GetCommandBuffer();

        auto const& pipeline = GetShaderPipeline(*impl);
        if (context->m_currentPipelineId != pipeline.m_hash) {
            commandBuffer.BindPipeline(pipeline);
            context->m_currentPipelineId = pipeline.m_hash;
        }

        commandBuffer.BindDescriptorSet(*impl->GetShader(), impl->GetDescriptorSet(channels), 0);

        PushConstants pushConstants;
        pushConstants.xyScale = { 2.0f / static_cast<float>(context->m_logicalExtent.width),
                                  2.0f / static_cast<float>(context->m_logicalExtent.height) };
        pushConstants.xyOffset = { -1.0f, -1.0f };
        commandBuffer.PushConstants(*impl->GetShader(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstants), &pushConstants);

        uint32_t const vertexOffset = context->m_vertexCount;
        std::memcpy(context->m_vertexBufferData + vertexOffset, vertices, sizeof(Vertex) * 6);
        context->m_vertexCount += 6;

        commandBuffer.BindVertexBuffer(*context->m_vertexBuffer, 0);
        commandBuffer.Draw(6, vertexOffset);
    }
}
