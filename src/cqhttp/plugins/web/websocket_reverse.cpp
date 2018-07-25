#include "./websocket_reverse.h"

using namespace std;

namespace cqhttp::plugins {
    static const auto TAG = "反向WS";

    static string check_ws_url(const string &url) {
        if (!url.empty() && !regex_search(url, regex("^wss?://"))) {
            // bad websocket url, we warn the user, and ignore the url
            logging::warning(TAG, u8"反向 WebSocket 服务端地址 " + url + u8" 不是合法地址，将被忽略");
            return "";
        }
        return url;
    }

    void WebSocketReverse::hook_enable(Context &ctx) {
        use_ws_reverse_ = ctx.config->get_bool("use_ws_reverse", false);

        if (use_ws_reverse_) {
            const auto access_token = ctx.config->get_string("access_token", "");
            const auto reconnect_interval = ctx.config->get_integer("ws_reverse_reconnect_interval", 3000);
            const auto reconnect_on_code_1000 = ctx.config->get_bool("ws_reverse_reconnect_on_code_1000", false);

            auto url = check_ws_url(
                ctx.config->get_string("ws_reverse_api_url", ctx.config->get_string("ws_reverse_url", "")));
            if (!url.empty()) {
                api_ = make_shared<ApiClient>(url, access_token, reconnect_interval, reconnect_on_code_1000);
                api_->start();
            } else {
                api_ = nullptr;
            }

            url = check_ws_url(
                ctx.config->get_string("ws_reverse_event_url", ctx.config->get_string("ws_reverse_url", "")));
            if (!url.empty()) {
                event_ = make_shared<EventClient>(url, access_token, reconnect_interval, reconnect_on_code_1000);
                event_->start();
            } else {
                event_ = nullptr;
            }
        }

        ctx.next();
    }

    void WebSocketReverse::hook_disable(Context &ctx) {
        if (api_) {
            api_->stop();
            api_ = nullptr;
        }
        if (event_) {
            event_->stop();
            event_ = nullptr;
        }

        ctx.next();
    }

    void WebSocketReverse::hook_after_event(EventContext<cq::Event> &ctx) {
        if (event_) {
            event_->push_event(ctx.data);
        }
        ctx.next();
    }
} // namespace cqhttp::plugins
