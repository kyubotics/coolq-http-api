#pragma once

#include "./common.h"

namespace cq {
    struct Config {
        bool convert_unicode_emoji = true;
    };

    extern Config config;

    namespace app {
        extern int32_t auth_code;
        extern std::string id;

        extern std::function<void(int32_t auth_code)> on_initialize;
        extern std::function<void()> on_enable;
        extern std::function<void()> on_disable;
        extern std::function<void()> on_start;
        extern std::function<void()> on_exit;

        extern std::function<void()> __main;
    }
}

#define CQ_INITIALIZE(AppId) \
    static bool __cq_set_id() { cq::app::id = AppId; return true; } \
    static bool __cq_set_id_dummy = __cq_set_id()

#define CQ_MAIN \
    static void __cq_main(); \
    static bool __cq_set_main_function() { cq::app::__main = __cq_main; return true; } \
    static bool __cq_main_dummy = __cq_set_main_function(); \
    static void __cq_main()
