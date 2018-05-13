#include "./default_config_generator.h"

#include <fstream>

using namespace std;

static constexpr auto DEFAULT_CONFIG = R"({
    "host": "0.0.0.0",
    "port": 5700,
    "use_http": true,
    "ws_host": "0.0.0.0",
    "ws_port": 6700,
    "use_ws": false,
    "post_url": "",
    "access_token": "",
    "secret": "",
    "post_message_format": "string",
    "serve_data_files": false,
    "update_source": "",
    "update_channel": "stable",
    "auto_check_update": false,
    "auto_perform_update": false,
    "thread_pool_size": 4,
    "server_thread_pool_size": 1,
    "show_log_console": false,
    "enable_backward_compatibility": true
})";

namespace cqhttp::plugins {
    static const auto TAG = u8"配置";

    void DefaultConfigGenerator::hook_enable(Context &ctx) {
        if (!ctx.config->get_bool("_LOADED", false)) {
            // there is no config file now, we should generate a default one
            logging::info(TAG, u8"没有找到配置文件，写入默认配置");

            const auto login_id_str = to_string(cq::api::get_login_user_id());
            if (ofstream file(ansi(cq::dir::app("config") + login_id_str + ".json")); file.is_open()) {
                file << DEFAULT_CONFIG;
            } else {
                logging::error(TAG, u8"默认配置写入失败，请检查文件系统权限");
            }

            try {
                ctx.config->raw = json::parse(DEFAULT_CONFIG);
            } catch (...) {
            }
        }

        ctx.config->raw.erase("_LOADED");
        logging::info(TAG, u8"已加载的配置：\n" + ctx.config->raw.dump(2));

        ctx.next();
    }
} // namespace cqhttp::plugins
