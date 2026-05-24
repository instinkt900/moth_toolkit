#include "common.h"
#include "moth_ui/widgets/widget.h"

#include "moth_ui/widgets/ui_button.h"

namespace moth_ui {
    void EnsureWidgetsRegistered() {
        // Touching each widget's template static here forces the linker to
        // retain this TU along with every referenced widget's TU, keeping
        // their SelfRegister() initialisers intact. Add a new line for each
        // bundled widget added to moth_ui.
        (void)Widget<UIButton>::s_widgetIsRegistered_;
    }
}
