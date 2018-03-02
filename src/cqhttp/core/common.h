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

namespace logging = cq::logging;

#include "cqhttp/core/json_convertor.h"
