#pragma once

#include "moth_ui/ui_fwd.h"

namespace ui {
    std::shared_ptr<LayoutEntityGroup> LoadLayout(std::string const& layoutPath);
}
