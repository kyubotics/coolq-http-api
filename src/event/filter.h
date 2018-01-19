#pragma once

#include "common.h"

class FilterSyntexError : public std::exception {
public:
    using exception::exception;
    FilterSyntexError(const std::string &msg) : FilterSyntexError(msg.c_str()) {}
};

class IFilter {
public:
    virtual ~IFilter() = default;
    virtual bool eval(const json &payload) = 0;
};

std::shared_ptr<IFilter> construct_filter(const json &root_filter);

class GlobalFilter {
public:
    static void load(const std::string &path);
    static void reset();
    static bool eval(const json &payload);

private:
    static std::shared_ptr<IFilter> filter_;
};
