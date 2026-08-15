#pragma once

#include "moth_ui/graphics/image_scale_type.h"
#include "moth_ui/graphics/texture_filter.h"
#include "moth_ui/nodes/node.h"

#include <array>
#include <filesystem>
#include <memory>

namespace moth_ui {
    /**
     * @brief A Node that renders an image with configurable scale type.
     *
     * Supports stretch, tile, and nine-slice rendering modes.
     */
    class NodeImage : public Node {
    public:
        NodeImage(NodeImage const& other) = delete;
        NodeImage(NodeImage&& other) = default;
        NodeImage& operator=(NodeImage const&) = delete;
        NodeImage& operator=(NodeImage&&) = delete;
        ~NodeImage() override = default;

        /**
         * @brief Creates a NodeImage with no layout entity.
         * @param context Active rendering context.
         * @return A shared_ptr managing the new node.
         */
        static std::shared_ptr<NodeImage> Create(Context& context);

        /**
         * @brief Creates a NodeImage from a serialised layout entity.
         * @param context      Active rendering context.
         * @param layoutEntity Deserialised image description.
         * @return A shared_ptr managing the new node.
         */
        static std::shared_ptr<NodeImage> Create(Context& context, std::shared_ptr<LayoutEntityImage> layoutEntity);

        void UpdateChildBounds() override;

        /**
         * @brief Loads an image from a file path.
         * @param path Path to the image file.
         */
        void Load(std::filesystem::path const& path);

        /// @brief Returns the currently loaded image, or @c nullptr.
        IImage const* GetImage() const { return m_image.get(); }

        /// @brief Returns a mutable reference to the source sampling rectangle.
        IntRect& GetSourceRect() { return m_sourceRect; }

        /// @brief Returns a const reference to the source sampling rectangle.
        IntRect const& GetSourceRect() const { return m_sourceRect; }

        /**
         * @brief Returns the four source slice points used in nine-slice rendering.
         * @return Pointer to the first of four IntVec2 slice points (always non-null).
         */
        IntVec2 const* GetSourceSlices() const { return m_sourceSlices.data(); }

        /**
         * @brief Returns the four destination slice points used in nine-slice rendering.
         * @return Pointer to the first of four IntVec2 slice points (always non-null).
         */
        IntVec2 const* GetTargetSlices() const { return m_targetSlices.data(); }

        /// @brief Returns the current image scale type.
        ImageScaleType GetImageScaleType() const { return m_imageScaleType; }

        /// @brief Returns the uniform scale factor used in tile/nine-slice modes.
        float GetImageScale() const { return m_imageScale; }

        /// @brief Returns the texture sampling filter applied when the image is scaled.
        TextureFilter GetTextureFilter() const { return m_textureFilter; }

        /**
         * @brief Sets the image scale type.
         * @param type Scale type to use when rendering.
         */
        void SetImageScaleType(ImageScaleType type) { m_imageScaleType = type; }

        /**
         * @brief Sets the texture sampling filter.
         * @param filter @c TextureFilter::Linear for smooth scaling,
         *               @c TextureFilter::Nearest for crisp pixel art.
         */
        void SetTextureFilter(TextureFilter filter) { m_textureFilter = (filter == TextureFilter::Invalid) ? TextureFilter::Linear : filter; }

    protected:
        /**
         * @brief Constructs a NodeImage with no layout entity.
         * @param context Active rendering context.
         */
        NodeImage(Context& context);

        /**
         * @brief Constructs a NodeImage from a serialised layout entity.
         * @param context      Active rendering context.
         * @param layoutEntity Deserialised image description.
         */
        NodeImage(Context& context, std::shared_ptr<LayoutEntityImage> layoutEntity);

        std::unique_ptr<IImage> m_image;
        IntRect m_sourceRect;
        ImageScaleType m_imageScaleType = ImageScaleType::Stretch;
        float m_imageScale = 1.0f;
        TextureFilter m_textureFilter = TextureFilter::Linear;

        IntRect m_sourceBorders;
        LayoutRect m_targetBorders;
        std::array<IntVec2,4> m_sourceSlices;
        std::array<IntVec2,4> m_targetSlices;

        void ReloadEntityInternal() override;
        void DrawInternal() override;

        void Slice();

    private:
        LayoutEntityImage* m_typedLayout = nullptr;
    };
}
