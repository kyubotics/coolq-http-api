#include "cqhttp/core/core.h"

using namespace std;
using namespace cqhttp;

CQ_MENU(cq_menu_restart) { call_action("set_restart_plugin"); }

CQ_MENU(cq_menu_check_update) { call_action(".check_update"); }

CQ_MENU(cq_menu_open_app_dir) {
    const auto cmd = string("explorer.exe \"") + cq::dir::app() + "\"";
    std::system(cq::utils::ansi(cmd).c_str());
}

CQ_MENU(cq_menu_open_coolq_dir) {
    const auto cmd = string("explorer.exe \"") + cq::dir::root() + "\"";
    std::system(cq::utils::ansi(cmd).c_str());
}
