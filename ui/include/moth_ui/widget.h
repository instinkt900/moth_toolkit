#pragma once

#include "moth_ui/context.h"
#include "moth_ui/node_factory.h"
#include "moth_ui/group.h"

#include <memory>

namespace moth_ui {
    template <typename T, typename BaseType = Group>
    class Widget : public BaseType {
    public:
        Widget(Context& context, std::shared_ptr<LayoutEntityGroup> entity)
            : BaseType(context, entity) {
            (void)s_widgetIsRegistered_;
        }

        static bool SelfRegister() {
            NodeFactory::Get().RegisterWidget(T::ClassName, [](Context& context, auto entity) -> std::unique_ptr<Group> { return std::make_unique<T>(context, entity); });
            return true;
        }

        static bool s_widgetIsRegistered_;
    };

    template <typename T, typename BaseType>
    bool Widget<T, BaseType>::s_widgetIsRegistered_ = Widget<T, BaseType>::SelfRegister();
}
