#pragma once

#include "cqhttp/core/common.h"

namespace cqhttp::helpers {
    std::string get_update_source_url(std::string rel_path = "");
    inline std::string get_asset_url(std::string asset_name) { return get_update_source_url("assets/" + asset_name); }
} // namespace cqhttp::helpers
