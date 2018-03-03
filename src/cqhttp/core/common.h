#pragma once

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "cqsdk/cqsdk.h"

using cq::utils::ansi;
using cq::utils::s2ws;
using cq::utils::ws2s;

namespace logging = cq::logging;

#include "cqhttp/core/def.h"
#include "cqhttp/core/json_convertor.h"
