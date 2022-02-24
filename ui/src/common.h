#pragma once

#include <memory>
#include <variant>
#include <string>
#include <fstream>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
//#include <SDL_mixer.h>
#include <nlohmann/json.hpp>
//#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <imgui.h>
#include <magic_enum.hpp>

#include <range/v3/algorithm/find.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/algorithm/sort.hpp>

#include "uilib/ui_fwd.h"
#include "uilib/utils/vec2.h"
#include "uilib/utils/rect.h"
#include "uilib/utils/math_utils.h"

#include "uilib/layout/layout_types.h"

#include "uilib/utils/imgui_ext.h"
#include "uilib/utils/smart_sdl.h"
#include "uilib/utils/imgui_ext_inspect.h"
