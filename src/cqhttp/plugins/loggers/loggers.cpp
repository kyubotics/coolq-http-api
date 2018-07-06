#include "./loggers.h"

#include <boost/filesystem.hpp>

#include "cqhttp/logging/handlers/console.h"
#include "cqhttp/logging/handlers/file.h"

#undef ERROR

using namespace std;
namespace fs = boost::filesystem;

namespace cqhttp::plugins {
    static const auto TAG = u8"日志";

    void Loggers::create_file_logger() {
        const auto file_handler = make_shared<logging::FileHandler>(
            cq::dir::app("log") + to_string(cq::api::get_login_user_id()) + ".log", max_file_size_, max_files_);
        file_handler->init();
        register_handler("file", file_handler);
    }

    static void remove_file_logger() {
        if (auto handler = logging::unregister_handler("file")) {
            handler->destroy();
        }
    }

    void Loggers::hook_enable(Context &ctx) {
        max_file_size_ = ctx.config->get_integer("max_log_file_size", 6 * 1024 * 1024);
        max_files_ = ctx.config->get_integer("max_log_files", 1);

        create_file_logger();
        logging::unregister_handler("default"); // remove default logging handler

        if (ctx.config->get_bool("show_log_console", false)) {
            const auto console_handler = make_shared<logging::ConsoleHandler>();
            console_handler->init();
            register_handler("console", console_handler);
            logging::info_success(TAG, u8"日志控制台开启成功");
        }

        const auto log_level = boost::to_lower_copy(ctx.config->get_string("log_level", "info"));
        const unordered_map<string, int> log_level_map = {
            {"debug", cq::logging::DEBUG},
            {"info", cq::logging::INFO},
            {"warning", cq::logging::WARNING},
            {"error", cq::logging::ERROR},
            {"fatal", cq::logging::FATAL},
        };
        if (log_level_map.count(log_level) != 0) {
            logging::set_level(static_cast<cq::logging::Level>(log_level_map.at(log_level)));
        } else {
            // unrecognized log level
            logging::error(TAG, u8"未知的日志等级 " + log_level);
        }

        cq::logging::info_success(TAG, u8"日志系统初始化成功");
        cq::logging::info(TAG, u8"请在酷 Q 主目录的 app\\io.github.richardchien.coolqhttpapi\\log\\ 中查看日志文件");
        ctx.next();
    }

    void Loggers::hook_disable(Context &ctx) {
        ctx.next();
        if (auto handler = logging::unregister_handler("console")) {
            handler->destroy();
        }
        remove_file_logger();
    }

    void Loggers::hook_missed_action(ActionContext &ctx) {
        if (ctx.action != "clean_plugin_log") {
            ctx.next();
            return;
        }

        remove_file_logger();
        for (auto it = fs::directory_iterator(ansi(cq::dir::app("log"))); it != fs::directory_iterator(); ++it) {
            const auto filename = string_decode(it->path().filename().string(), cq::utils::Encoding::ANSI);
            if (boost::starts_with(filename, to_string(cq::api::get_login_user_id()))
                && boost::ends_with(filename, ".log")) {
                // this is out log file
                fs::remove(it->path());
            }
        }
        create_file_logger();

        ctx.result.code = ActionResult::Codes::OK;
    }
} // namespace cqhttp::plugins
