#pragma once

#include "group.h"
#include "image_scale_type.h"
#include "moth_ui/layout/layout_entity_9slice.h"

namespace moth_ui {
    class Node9Slice : public Group {
    public:
        Node9Slice();
        Node9Slice(std::shared_ptr<LayoutEntity9Slice> layoutEntity);
        virtual ~Node9Slice();

        void Load(char const* path);

        void ReloadEntity() override;

        IImage const* GetImage() const { return m_image.get(); }

        void DebugDraw();

    protected:
        std::shared_ptr<IImage> m_image;
    };
}
