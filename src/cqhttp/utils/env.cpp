#include "./env.h"

#include <Windows.h>

using namespace std;

namespace cqhttp::utils::env {
    bool is_in_wine() {
        static optional<bool> result;

        if (result.has_value()) {
            return result.value();
        }

        auto tmp = false;
        const auto ntdll = GetModuleHandleW(L"ntdll.dll");
        if (ntdll) {
            if (GetProcAddress(ntdll, "wine_get_version")) {
                tmp = true;
            }
        } else {
            tmp = true;
        }

        result = tmp;
        return tmp;
    }
} // namespace cqhttp::utils::env
