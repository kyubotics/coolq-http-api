#pragma once

#include "./common.h"

namespace cq {
    struct Config {
        bool convert_unicode_emoji = true;
    };

    extern Config config;

    namespace app {
        extern int32_t auth_code;
        // extern std::string id;

        /**
         * Lifecycle:
         *
         * +-----------------------------------------+
         * |             Enabled At Start            |
         * +-----------------------------------------+
         * | on_initialize                           |
         * |       +                                 |
         * |       |                                 |
         * |       v                                 |
         * | on_coolq_start                          |
         * |       +                                 |
         * |       |                                 |
         * |       v     disabled by user            |
         * |   on_enable +--------------> on_disable |
         * |       +                           +     |
         * |       |                           |     |
         * |       v                           |     |
         * | on_coolq_exit <-------------------+     |
         * +-----------------------------------------+
         *
         * +---------------------------------------+
         * |            Disabled At Start          |
         * +---------------------------------------+
         * | on_initialize +------+                |
         * |       +              |enabled by user |
         * |       |              |                |
         * |       |              v                |
         * |       |       on_coolq_start          |
         * |       |              +                |
         * |       |              |                |
         * |       |              v                |
         * |       |          on_enable            |
         * |       |              +                |
         * |       |              |                |
         * |       v              |                |
         * | on_coolq_exit <------+                |
         * +---------------------------------------+
         */
        extern std::function<void()> on_initialize;
        extern std::function<void()> on_enable;
        extern std::function<void()> on_disable;
        extern std::function<void()> on_coolq_start;
        extern std::function<void()> on_coolq_exit;

        extern std::function<void()> __main;
    } // namespace app
} // namespace cq

#define CQ_MAIN                                             \
    static void __cq_main();                                \
    static bool __cq_set_main_function() {                  \
        cq::app::__main = __cq_main;                        \
        return true;                                        \
    }                                                       \
    static bool __cq_main_dummy = __cq_set_main_function(); \
    static void __cq_main()
