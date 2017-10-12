#pragma once

#include <WinSDKVer.h>
#define _WIN32_WINNT 0x0502
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>
#include <optional>
#include <cstdint>
#include <boost/algorithm/string.hpp>
#include "utils/encoding.h"
#include "helpers.h"
