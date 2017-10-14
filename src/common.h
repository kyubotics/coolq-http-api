#pragma once

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <algorithm>
#include <string>
#include <optional>
#include <cstdint>
#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "utils/encoding.h"
#include "helpers.h"
