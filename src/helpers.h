#pragma once

#include "common.h"

typedef size_t (*CURLWriteFunctionPtr)(char *, size_t, size_t, void *);

bool isnumber(const std::string &s); // act as "is_positive_integer", actually

void string_replace(std::string &str, const std::string &search, const std::string &replace);

str get_coolq_root();
