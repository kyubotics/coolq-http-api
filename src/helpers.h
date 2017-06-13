#pragma once

#include "common.h"

typedef size_t (*CURLWriteFunctionPtr)(char *, size_t, size_t, void *);

size_t curl_write_file_callback(char *buf, size_t size, size_t nmemb, FILE *fp);

bool isnumber(const std::string &s); // act as "is_positive_integer", actually

void string_replace(std::string &str, const std::string &search, const std::string &replace);

str get_coolq_root();
