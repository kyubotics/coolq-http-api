#pragma once

#include <regex>

namespace rc {

    static std::string regex_escape(const std::string &s) {
        const std::regex esc("[.^$|()\\[\\]{}*+?\\\\]");
        const std::string rep("\\\\&");
        return regex_replace(s, esc, rep,
                             std::regex_constants::match_default | std::regex_constants::format_sed);
    }
}
