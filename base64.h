#pragma once

#include "stdafx.h"
#include <string>

std::string base64_encode(const unsigned char *, unsigned int len);
std::string base64_decode(const std::string &str);
