#pragma once

#include "cqhttp/core/common.h"

namespace cqhttp::utils {
    std::string data_file_full_path(const std::string &data_dir, const std::string &filename) {
        return cq::dir::root() + "data\\" + data_dir + "\\" + filename;
    }
} // namespace cqhttp::utils
