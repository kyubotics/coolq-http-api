#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    class Filter {
    public:
        virtual ~Filter() = default;
        virtual bool eval(const json &payload) = 0;
    };

    struct FilterSyntexError : std::invalid_argument {
        using invalid_argument::invalid_argument;
    };

    std::shared_ptr<Filter> construct_filter(const json &root_filter);
} // namespace cqhttp::plugins
