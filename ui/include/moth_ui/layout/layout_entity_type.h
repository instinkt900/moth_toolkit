#pragma once

namespace moth_ui {
    /**
     * @brief Discriminator tag stored with every LayoutEntity to identify its concrete type.
     */
    enum class LayoutEntityType {
        Unknown, ///< Unrecognised or uninitialised entity.
        Layout,  ///< Root Layout entity.
        Group,   ///< LayoutEntityGroup (container).
        Ref,     ///< LayoutEntityRef (reference to an external layout).
        Entity,  ///< Generic LayoutEntity base.
        Rect,    ///< LayoutEntityRect (coloured rectangle).
        Image,   ///< LayoutEntityImage.
        Text,    ///< LayoutEntityText.
        Clip,    ///< LayoutEntityClip (scissor region).
        Flipbook,///< LayoutEntityFlipbook.
    };
}
