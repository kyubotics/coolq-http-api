// 
// common.h : Include frequently used system or project level include files.
// 
// Copyright (C) 2017  Richard Chien <richardchienthebest@gmail.com>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

#pragma once

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include <SDKDDKVer.h>

#include <jansson/jansson.h>

#include <cpputils/cpputils.h>
#include <cpputils/win32_bytes.h>
using namespace rc;
using namespace win32;

#include <json.hpp>
using json = nlohmann::json;

namespace rc {
    void to_json(nlohmann::json &j, const str &s);
    void from_json(const nlohmann::json &j, str &s);
}
