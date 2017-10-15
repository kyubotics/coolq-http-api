// 
// sdk.cpp : Define CoolQ SDK functions and initializer.
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

#include "app.h"

#include <vector>
#include <functional>

using namespace std;

vector<function<void(HMODULE)>> sdk_func_initializers;

static bool __add_func_initializer(const std::function<void(HMODULE)> initiator) {
    sdk_func_initializers.push_back(initiator);
    return true;
}

#define FUNC(ReturnType, FuncName, ...) \
    typedef __declspec(dllimport) ReturnType (__stdcall *__CQ_##FuncName##_T)(__VA_ARGS__); \
    __CQ_##FuncName##_T CQ_##FuncName; \
    static bool __dummy_CQ_##FuncName = __add_func_initializer([](auto dll) { \
        CQ_##FuncName = reinterpret_cast<__CQ_##FuncName##_T>(GetProcAddress(dll, "CQ_"#FuncName)); \
    });

// due to the FUNC macro defined above, the following include statement will DEFINE all those functions
#include "./funcs.h"

#undef FUNC

void init_sdk(const int32_t auth_code) {
    const auto dll = LoadLibraryW(L"CQP.dll");
    for (const auto &initializer : sdk_func_initializers) {
        initializer(dll);
    }

    sdk = Sdk(auth_code);
}
