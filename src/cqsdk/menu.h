#pragma once

#include "./def.h"

#define CQ_MENU(MenuName)                \
    static void __cq_menu_##MenuName();  \
    __CQ_EVENT(int32_t, MenuName, 0)() { \
        __cq_menu_##MenuName();          \
        return 0;                        \
    }                                    \
    static void __cq_menu_##MenuName()
