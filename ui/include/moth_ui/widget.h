#pragma once

#include "moth_ui/node_factory.h"
#include "moth_ui/group.h"

namespace moth_ui {
    template <typename T, typename BaseType = Group>
    class Widget : public BaseType {
    public:
        Widget(std::shared_ptr<LayoutEntityGroup> entity)
            : BaseType(entity) {
            (void)s_widgetIsRegistered_;
        }

        static bool SelfRegister() {
            NodeFactory::Get().RegisterWidget(T::ClassName, [](auto entity) -> std::unique_ptr<Group> { return std::make_unique<T>(entity); });
            return true;
        }

        static bool s_widgetIsRegistered_;
    };

    template <typename T, typename BaseType = Group>
    bool Widget<T, BaseType>::s_widgetIsRegistered_ = Widget<T, BaseType>::SelfRegister();
}
