#include "./app.h"

#include "./def.h"
#include "./utils/function.h"
#include "./api.h"

namespace cq {
    Config config;

    namespace app {
        int32_t auth_code = 0;
        std::string id = "";

        std::function<void(int32_t)> on_initialize;
        std::function<void()> on_enable;
        std::function<void()> on_disable;
        std::function<void()> on_start;
        std::function<void()> on_exit;

        std::function<void()> __main;
    }
}

using namespace std;
using namespace cq;
using cq::utils::call_if_valid;

/**
 * Return app info.
 */
CQEVENT(const char *, AppInfo, 0)
() {
    // CoolQ API version: 9
    static auto info = "9," + app::id;
    return info.c_str();
}

/**
 * Initialize SDK using the auth code given by CoolQ.
 */
CQEVENT(int32_t, Initialize, 4)
(const int32_t auth_code) {
    app::auth_code = auth_code;
    api::init();
    call_if_valid(app::on_initialize, auth_code);
    return 0;
}

/**
 * Event: Plugin is enabled.
 */
CQEVENT(int32_t, cq_app_enable, 0)
() {
    call_if_valid(app::on_enable);
    return 0;
}

/**
 * Event: Plugin is disabled.
 */
CQEVENT(int32_t, cq_app_disable, 0)
() {
    call_if_valid(app::on_disable);
    return 0;
}

/**
 * Event: CoolQ is starting.
 */
CQEVENT(int32_t, cq_app_start, 0)
() {
    call_if_valid(app::on_start);
    return 0;
}

/**
 * Event: CoolQ is exiting.
 */
CQEVENT(int32_t, cq_app_exit, 0)
() {
    call_if_valid(app::on_exit);
    return 0;
}
