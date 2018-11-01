#include "./api.h"

using namespace std;

namespace cq::api {
    static vector<function<void(HMODULE)>> api_func_initializers;

    static bool add_func_initializer(const function<void(HMODULE)> &initializer) {
        api_func_initializers.push_back(initializer);
        return true;
    }

    void __init() {
        const auto dll = GetModuleHandleW(L"CQP.dll");
        for (const auto &initializer : api_func_initializers) {
            initializer(dll);
        }
    }

    namespace raw {
#define FUNC(ReturnType, FuncName, ...)                                                              \
    typedef __declspec(dllimport) ReturnType(__stdcall *__CQ_##FuncName##_T)(__VA_ARGS__);           \
    __CQ_##FuncName##_T CQ_##FuncName;                                                               \
    static bool __dummy_CQ_##FuncName = add_func_initializer([](auto dll) {                          \
        CQ_##FuncName = reinterpret_cast<__CQ_##FuncName##_T>(GetProcAddress(dll, "CQ_" #FuncName)); \
    });

#include "./api_funcs.h"
    } // namespace raw
} // namespace cq::api
