#pragma once

#include "moth_ui/context.h"
#include "moth_ui/node_factory.h"
#include "moth_ui/nodes/group.h"

#include <memory>

namespace moth_ui {
    /**
     * @brief CRTP base that auto-registers a custom widget class with NodeFactory.
     *
     * Derive from @c Widget<MyWidget> (or @c Widget<MyWidget, MyBaseGroup>) to
     * have the widget automatically registered under @c MyWidget::ClassName when
     * the translation unit is loaded.
     *
     * @tparam T        The concrete widget type being defined.
     * @tparam BaseType The Group subclass to inherit from (defaults to Group).
     */
    template <typename T, typename BaseType = Group>
    class Widget : public BaseType {
    public:
        /**
         * @brief Constructs the widget from a layout entity group.
         * @param context Active rendering context.
         * @param entity  Layout entity describing this widget.
         */
        Widget(Context& context, std::shared_ptr<LayoutEntityGroup> entity)
            : BaseType(context, entity) {
            (void)s_widgetIsRegistered_;
        }

        Widget(Widget const& other) = delete;
        Widget(Widget&& other) = default;
        Widget& operator=(Widget const&) = delete;
        Widget& operator=(Widget&&) = delete;
        ~Widget() override = default;

        /**
         * @brief Registers this widget type with the global NodeFactory.
         * @return @c true; used only as an initialiser return value.
         */
        static bool SelfRegister() {
            NodeFactory::Get().RegisterWidget(T::ClassName, [](Context& context, auto entity) -> std::shared_ptr<Group> { return std::make_shared<T>(context, entity); });
            return true;
        }

        /// @brief Static flag that triggers SelfRegister() at program startup.
        static bool s_widgetIsRegistered_;
    };

    template <typename T, typename BaseType>
    bool Widget<T, BaseType>::s_widgetIsRegistered_ = Widget<T, BaseType>::SelfRegister();
}
