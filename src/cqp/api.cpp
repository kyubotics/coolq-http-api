//
// Created by richard on 2017/8/5.
//

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <vector>
#include <functional>

using namespace std;

vector<function<void(HMODULE)>> api_initiators;

#define API(ReturnType, ApiName, ...) \
    typedef __declspec(dllimport) ReturnType (__stdcall *CQ_##ApiName##_T)(__VA_ARGS__); \
    CQ_##ApiName##_T CQ_##ApiName; \
    bool __dummy_CQ_##ApiName = __add_api_initiator([](auto dll) { \
        CQ_##ApiName = reinterpret_cast<CQ_##ApiName##_T>(GetProcAddress(dll, "CQ_"#ApiName)); \
    });

#include "./api.h"

#undef API

void init_dll() {
    const auto dll = LoadLibraryW(L"CQP.dll");
    for (const auto &initiator : api_initiators) {
        initiator(dll);
    }
}
