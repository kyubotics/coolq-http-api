#pragma once

#include "./common.h"

namespace cq::dir {
    std::string root();
    std::string app(const std::string &sub = "");
} // namespace cq::dir
