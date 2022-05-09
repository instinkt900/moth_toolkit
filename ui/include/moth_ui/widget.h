#pragma once

#include "moth_ui/node_factory.h"
#include "moth_ui/group.h"

namespace moth_ui {
    template <typename T>
    class Widget : public Group {
    public:
        Widget(std::shared_ptr<LayoutEntityGroup> entity)
            : Group(entity) {
            (void)s_widgetIsRegistered_;
        }

        static bool SelfRegister() {
            NodeFactory::Get().RegisterWidget(T::ClassName, [](auto entity) -> std::unique_ptr<Group> { return std::make_unique<T>(entity); });
            return true;
        }

        static bool s_widgetIsRegistered_;
    };

    template <typename T>
    bool Widget<T>::s_widgetIsRegistered_ = Widget<T>::SelfRegister();
}
