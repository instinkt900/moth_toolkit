#pragma once

#include <memory>
#include <variant>
#include <string>
#include <fstream>

#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <imgui.h>
#include <magic_enum.hpp>

#include <range/v3/algorithm/find.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/algorithm/sort.hpp>

#include "moth_ui/utils/serialize_utils.h"

#undef min
#undef max
