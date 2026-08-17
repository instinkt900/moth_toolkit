#pragma once

#include "moth/graphics/graphics/blend_mode.h"
#include "moth/graphics/graphics/color.h"
#include "moth/graphics/graphics/ifont.h"
#include "moth/graphics/graphics/igraphics.h"
#include "moth/graphics/graphics/igraphics_device.h"
#include "moth/graphics/graphics/image.h"
#include "moth/graphics/graphics/itarget.h"
#include "moth/graphics/graphics/text_alignment.h"
#include "vulkan_buffer.h"
#include "vulkan_command_buffer.h"
#include "vulkan_framebuffer.h"
#include "vulkan_pipeline.h"
#include "vulkan_renderpass.h"
#include "vulkan_shader.h"
#include "vulkan_shader_object.h"
#include "moth/graphics/graphics/vulkan/vulkan_surface_context.h"
#include "vulkan_swapchain.h"
#include "vulkan_texture.h"
#include "vulkan_unique.h"
#include "moth/graphics/platform/window.h"
#include "moth/graphics/utils/rect.h"
#include "moth/graphics/utils/vector.h"

#include <vulkan/vulkan_core.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <stack>
#include <string>

namespace moth::gfx::vulkan {
    class Graphics : public IGraphics, public IGraphicsDevice {
    public:
        Graphics(SurfaceContext& context, VkSurfaceKHR surface, uint32_t surfaceWidth, uint32_t surfaceHeight);
        ~Graphics();

        SurfaceContext& GetSurfaceContext() const { return m_surfaceContext; }

        struct Vertex {
            FloatVec2 xy;
            FloatVec2 uv;
            Color color;
        };

        struct FontRect {
            float min_x;
            float min_y;
            float max_x;
            float max_y;
        };

        struct FontGlyphInstance {
            FloatVec2 pos;
            uint32_t glyphIndex;
            float rotation; // radians, clockwise
            Color color;
        };

        void Begin() override;
        void End() override;

        void SetBlendMode(BlendMode mode) override;
        void PushBlendMode(BlendMode mode) override;
        void PopBlendMode() override;
        void SetColor(Color const& color) override;
        void PushColor(Color const& color) override;
        void PopColor() override;
        void SetShader(moth::gfx::Shader const* shader) override;
        void Clear() override;
        void Clear(Color const& color) override;
        void SetTransform(FloatMat4x4 const& transform) override;
        void PushTransform(FloatMat4x4 const& transform) override;
        void PopTransform() override;
        void DrawImage(Image const& image, IntVec2 const& pos, FloatVec2 const& pivot) override;
        void DrawImage(Image const& image, FloatVec2 const& pos, FloatVec2 const& pivot) override;
        void DrawImage(Image const& image, Transform2D const& transform, FloatVec2 const& pivot, bool flipX, bool flipY) override;
        void DrawImage(Image const& image, IntRect const& destRect, IntRect const* sourceRect) override;
        void DrawImage(Image const& image, FloatRect const& destRect, IntRect const* sourceRect) override;
        void DrawImageTiled(Image const& image, IntRect const& destRect, IntRect const* sourceRect, float scale) override;
        void DrawImageTiled(Image const& image, FloatRect const& destRect, IntRect const* sourceRect, float scale) override;
        void DrawRectF(FloatRect const& rect) override;
        void DrawFillRectF(FloatRect const& rect) override;
        void DrawFillCircleF(FloatVec2 const& center, float radius) override;
        void DrawFillEllipseF(FloatVec2 const& center, float radiusX, float radiusY) override;
        void DrawFillPolygonF(FloatVec2 const* points, size_t count) override;
        void DrawTrianglesF(FloatVec2 const* vertices, size_t count) override;
        void DrawTexturedTrianglesF(ITexture& texture, TexturedVertex const* vertices, size_t count) override;
        void DrawImageCircle(Image const& image, FloatVec2 const& center, float radius, IntRect const* sourceRect) override;
        void DrawImage9Slice(Image const& image, FloatRect const& destRect, NineSliceBorders const& borders) override;
        void DrawGradientRect(FloatRect const& destRect,
                              Color startColor, Color endColor,
                              FloatVec2 midpoint,
                              float angle,
                              float transitionLength) override;
        void DrawLineF(FloatVec2 const& p0, FloatVec2 const& p1) override;
        void DrawLineF(FloatVec2 const& p0, FloatVec2 const& p1, float thickness) override;
        void DrawText(std::string_view text, IFont& font, IntRect const& destRect, TextHorizAlignment horizontalAlignment = TextHorizAlignment::Left, TextVertAlignment verticalAlignment = TextVertAlignment::Top) override;
        void DrawShader(moth::gfx::Shader const& shader) override;
        void DrawShader(moth::gfx::Shader const& shader, FloatRect const& destRect) override;
        void SetClip(IntRect const* clipRect) override;
        void PushClip(IntRect const& rect) override;
        void PopClip() override;

        ITarget* GetTarget() override;
        void SetTarget(ITarget* target) override;

        void SetLogicalSize(IntVec2 const& logicalSize) override;

        // ---- IGraphicsDevice -------------------------------------------------
        std::unique_ptr<ITarget> CreateTarget(int width, int height) override;

        void Drain();

        Swapchain& GetSwapchain() const { return *m_swapchain; }
        RenderPass& GetRenderPass() const { return *m_renderPass; }
        CommandBuffer* GetCurrentCommandBuffer() {
            auto context = m_contextStack.top();
            if (context) {
                return &context->m_target->GetCommandBuffer();
            }
            return nullptr;
        }
        VkDescriptorSet GetDescriptorSet(Texture& image);

        Shader& GetFontShader() { return *m_fontShader; }

        void OnResize(VkSurfaceKHR surface, uint32_t surfaceWidth, uint32_t surfaceHeight);

        /// @brief Flush any pending vertex batch into the active command buffer.
        ///
        /// Call this before issuing draw commands that bind a different pipeline
        /// (e.g. ImGui's Vulkan backend). Without it, moth's pending batch would
        /// be flushed at @c End() against whatever pipeline the foreign code
        /// last bound, producing garbage geometry.
        void Flush();

    private:
        FloatMat4x4 CurrentTransform() const;

        SurfaceContext& m_surfaceContext;
        VkSurfaceKHR m_vkSurface = VK_NULL_HANDLE;
        FloatMat4x4 m_currentTransform = FloatMat4x4::Identity();
        std::stack<FloatMat4x4> m_transformStack;

        struct PushConstants {
            FloatVec2 xyScale;
            FloatVec2 xyOffset;
        };

        enum class ETopologyType {
            Invalid,
            Lines,
            Triangles
        };

        struct DrawContext {
            Framebuffer* m_target = nullptr;
            VkExtent2D m_logicalExtent;

            BlendMode m_currentBlendMode = BlendMode::Replace;
            Color m_currentColor = BasicColors::White;

            std::stack<BlendMode> m_blendModeStack;
            std::stack<Color> m_colorStack;

            // Current clip in logical coordinates (nullopt = no clip), plus the
            // stack saved by PushClip/PopClip.
            std::optional<IntRect> m_clipRect;
            std::stack<std::optional<IntRect>> m_clipStack;

            std::unique_ptr<Buffer> m_vertexBuffer;
            Vertex* m_vertexBufferData = nullptr;

            std::unique_ptr<Buffer> m_fontInstanceBuffer;
            std::unique_ptr<Buffer> m_fontInstanceStagingBuffer;
            uint32_t m_glyphCount = 0;

            uint32_t m_vertexCount = 0;
            uint32_t m_currentPipelineId = 0;

            // True until this context's first submit of the current frame. Only
            // that submit may wait on the target's acquire (imageAvailable)
            // semaphore; later mid-frame submits from RestartContext must not,
            // or they wait on an already-consumed semaphore that nothing will
            // re-signal (GPU deadlock + validation error).
            bool m_acquireWaitPending = false;

            // Letterbox projection state from SetLogicalSize (m_logicalExtent is
            // the logical coordinate space; these are the physical viewport and
            // scissor it maps into). Persisted on the context so StartCommands
            // re-applies it after a mid-frame RestartContext — otherwise a draw
            // that overflows the vertex buffer (e.g. a long aim path) reverts to
            // the target's native extent and the HUD renders at the wrong size.
            VkViewport m_viewport{};
            VkRect2D m_scissor{};

            struct PendingBatch {
                uint32_t m_firstVertex = 0;
                uint32_t m_vertexCount = 0;
                VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
                VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
            };
            std::optional<PendingBatch> m_pendingBatch;
        };

        UniqueHandle<VkPipelineCache> m_vkPipelineCache;
        std::map<uint32_t, std::shared_ptr<Pipeline>> m_pipelines;
        std::map<uint32_t, std::shared_ptr<Pipeline>> m_fontPipelines;
        std::map<uint32_t, std::shared_ptr<Pipeline>> m_shaderPipelines;
        std::unique_ptr<RenderPass> m_renderPass;
        std::unique_ptr<RenderPass> m_rtRenderPass;
        std::unique_ptr<Swapchain> m_swapchain;
        std::shared_ptr<Shader> m_drawingShader;
        std::shared_ptr<Shader> m_fontShader;
        std::unique_ptr<Texture> m_defaultImage;

        // Active custom shader (SetShader); invalid when reset.
        moth::gfx::Shader m_activeShader;

        // Shader clock + frame counter feeding iTime/iTimeDelta/iFrame.
        std::chrono::steady_clock::time_point m_shaderStartTime = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point m_shaderLastFrameTime = std::chrono::steady_clock::now();
        float m_shaderLastDelta = 0.0f;
        std::uint64_t m_frameCount = 0;

        DrawContext m_defaultContext;
        DrawContext m_overrideContext;
        std::stack<DrawContext*> m_contextStack;

        static VkPrimitiveTopology ToVulkan(ETopologyType type);
        static VkPipelineColorBlendAttachmentState ToVulkan(BlendMode mode);

        void CreateRenderPass();
        void CreateShaders();
        void CreateDefaultImage();
        RenderPass& GetCurrentRenderPass();
        Pipeline& GetCurrentPipeline(ETopologyType topology);
        Pipeline& GetCurrentFontPipeline();
        Pipeline& GetShaderPipeline(VulkanShader& shader, ETopologyType topology);
        void UpdateShaderBuiltins(VulkanShader& shader);

        /// @brief Returns the current draw context, or @c nullptr for a null frame.
        ///
        /// @note The constructor pushes a nullptr sentinel onto m_contextStack.
        ///       Begin()/End() push/pop real contexts; a null frame pushes an
        ///       additional nullptr. The stack is never empty.
        DrawContext* CurrentContext() {
            assert(!m_contextStack.empty());
            return m_contextStack.top();
        }

        void BeginContext(DrawContext* context);
        void ApplyProjection();
        void ApplyClipScissor();
        void RestartContext();
        void EndContext();
        void StartCommands();
        void FlushCommands(bool isFinal);
        void FlushPendingBatch();
        void SubmitVertices(Vertex* vertices, uint32_t vertCount, ETopologyType topology, std::shared_ptr<Texture> texture = nullptr);

        bool IsRenderTarget() const;
    };
}
