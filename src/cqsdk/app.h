#pragma once

#include "./common.h"

namespace cq {
    struct Config {
        bool convert_unicode_emoji = true;
    };

    extern Config config;

    namespace app {
        extern int32_t auth_code;

        extern std::function<void(int32_t auth_code)> on_initialize;
        extern std::function<void()> on_enable;
        extern std::function<void()> on_disable;
        extern std::function<void()> on_start;
        extern std::function<void()> on_exit;

        extern std::function<const char *()> __app_info;
        extern std::function<void()> __main;
    }
}

#define CQ_INITIALIZE(AppId) \
    static const char *__cq_app_info(); \
    static bool __cq_set_app_info_function() { cq::app::__app_info = __cq_app_info; return true; } \
    static bool __cq_app_info_dummy = __cq_set_app_info_function(); \
    static const char *__cq_app_info() { return "9," AppId; }

#define CQ_MAIN \
    static void __cq_main(); \
    static bool __cq_set_main_function() { cq::app::__main = __cq_main; return true; } \
    static bool __cq_main_dummy = __cq_set_main_function(); \
    static void __cq_main()
