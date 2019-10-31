#pragma once

#include "cqhttp/core/common.h"

namespace cqhttp::utils::fs {
    inline std::string data_file_full_path(const std::string &data_dir, const std::string &filename) {
        return cq::dir::root() + "data\\" + data_dir + "\\" + filename;
    }

    inline std::string app_dir_rel_path(const std::string &sub_dir_name = "") {
        return cq::dir::app(sub_dir_name).substr(cq::dir::root().length());
    }
} // namespace cqhttp::utils::fs
