#pragma once

#include "common.h"

// tuple<is_newer, version, build_number, description>
std::optional<std::tuple<bool, std::string, int, std::string>> check_update();
