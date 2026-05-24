#pragma once

#include <functional>

namespace moth_ui {
    /**
     * @brief Abstract interface for nodes that can be wired to a click action.
     *
     * Implemented by @ref UIButton and by any application-defined widget that
     * wishes to be addressable from a flow graph's @c button trigger. The
     * @ref flow::Flow runtime uses @c dynamic_cast<IClickable*> to locate a
     * clickable node by id on a layout and install a callback that triggers
     * the corresponding transition.
     */
    class IClickable {
    public:
        IClickable() = default;
        IClickable(IClickable const&) = default;
        IClickable(IClickable&&) = default;
        IClickable& operator=(IClickable const&) = default;
        IClickable& operator=(IClickable&&) = default;
        virtual ~IClickable() = default;

        /**
         * @brief Installs the callback invoked when the widget is activated.
         * @param action Callable to invoke; pass an empty @c std::function to clear.
         */
        virtual void SetClickAction(std::function<void()> action) = 0;
    };
}
