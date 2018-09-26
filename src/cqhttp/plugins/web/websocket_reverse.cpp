#include "./websocket_reverse.h"

#include <filesystem>
#include <regex>

#include "cqhttp/core/core.h"
#include "cqhttp/core/helpers.h"
#include "cqhttp/utils/http.h"
#include "cqhttp/utils/mutex.h"

using namespace std;
namespace fs = std::filesystem;

namespace cqhttp::plugins {
    static const auto TAG = "反向WS";

    using utils::http::download_file;
    using utils::mutex::with_file_lock;
    using helpers::get_asset_url;

    static string check_ws_url(const string &url) {
        if (!url.empty() && !regex_search(url, regex("^wss?://", regex::icase))) {
            // bad websocket url, we warn the user, and ignore the url
            logging::warning(TAG, u8"反向 WebSocket 服务端地址 " + url + u8" 不是合法地址，将被忽略");
            return "";
        }
        if (boost::istarts_with(url, "wss://")) {
            const auto cacert_file = cq::dir::app("tmp") + "cacert.pem";
            auto ok = true;

            if (!fs::exists(ansi(cacert_file))) {
                try {
                    with_file_lock(cq::dir::app("tmp") + "~cacert.pem", [&] {
                        logging::info(
                            TAG,
                            u8"正在下载 CA 证书文件，这只会在第一次启动时进行，如果耗时较长，请在配置文件中更换更新源");
                        if (download_file(get_asset_url("cacert.pem"), cacert_file, true)) {
                            logging::info_success(TAG, u8"下载 CA 证书文件成功");
                        } else {
                            logging::warning(TAG, u8"下载 CA 证书文件失败，可能导致无法 WSS");
                            ok = false;
                        }
                    });
                } catch (runtime_error &) {
                    ok = false;
                }
            }

            if (ok) {
                app.store().put("cacert_file", cacert_file);
            }
        }
        return url;
    }

    void WebSocketReverse::hook_enable(Context &ctx) {
        use_ws_reverse_ = ctx.config->get_bool("use_ws_reverse", false);

        if (use_ws_reverse_) {
            const auto access_token = ctx.config->get_string("access_token", "");
            const auto reconnect_interval = ctx.config->get_integer("ws_reverse_reconnect_interval", 3000);
            const auto reconnect_on_code_1000 = ctx.config->get_bool("ws_reverse_reconnect_on_code_1000", true);

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
