#pragma once

#include "common.h"

typedef size_t (*CURLWriteFunctionPtr)(char *, size_t, size_t, void *);

size_t curl_write_file_callback(char *buf, size_t size, size_t nmemb, FILE *fp);

bool isnumber(const std::string &s); // act as "is_positive_integer", actually

std::string itos(int64_t i);

void string_replace(std::string &str, const std::string &search, const std::string &replace);

std::string get_cq_root_path();
