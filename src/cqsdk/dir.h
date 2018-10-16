#pragma once

#include "./common.h"

namespace cq::dir {
    std::string root();
    std::string app(const std::string &sub_dir_name = "");
    std::string app_per_account(const std::string &sub_dir_name);
} // namespace cq::dir
