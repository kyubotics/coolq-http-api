#include "./websocket_reverse.h"

using namespace std;

namespace cqhttp::plugins {
    void WebSocketReverse::hook_enable(Context &ctx) {
        use_ws_reverse_ = ctx.config->get_bool("use_ws_reverse", false);

        if (use_ws_reverse_) {
            const auto access_token = ctx.config->get_string("access_token", "");
            const auto reconnect_interval = ctx.config->get_integer("ws_reverse_reconnect_interval", 3000);
            const auto reconnect_on_code_1000 = ctx.config->get_bool("ws_reverse_reconnect_on_code_1000", false);

            auto url = ctx.config->get_string("ws_reverse_api_url", "");
            if (!url.empty()) {
                api_ = make_shared<ApiEndpoint>(url, access_token, reconnect_interval, reconnect_on_code_1000);
                api_->start();
            } else {
                api_ = nullptr;
            }

            url = ctx.config->get_string("ws_reverse_event_url", "");
            if (!url.empty()) {
                event_ = make_shared<EventEndpoint>(url, access_token, reconnect_interval, reconnect_on_code_1000);
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
