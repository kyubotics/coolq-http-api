#include "./file.h"

#include <spdlog/spdlog.h>

#include "cqsdk/dir.h"
#include "cqsdk/utils/string.h"

#undef ERROR

using namespace std;

namespace cqhttp::logging {
    using cq::logging::Level;

    static const auto LOGGER_NAME = "file";

    void FileHandler::init() {
        logger_ = spdlog::daily_logger_mt(LOGGER_NAME, cq::utils::ansi(cq::dir::app() + "log\\cqhttp.log"));
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%L] %v");
        logger_->flush_on(spdlog::level::trace);
        logger_->set_level(spdlog::level::debug);
    }

    void FileHandler::destroy() { spdlog::drop(LOGGER_NAME); }

    void FileHandler::log(const Level level, const string &tag, const string &msg) const {
        if (logger_) {
            spdlog::level::level_enum spd_level;

            switch (level) {
            case Level::DEBUG:
                spd_level = spdlog::level::debug;
                break;
            case Level::INFO:
            case Level::INFOSUCCESS:
            case Level::INFORECV:
            case Level::INFOSEND:
                spd_level = spdlog::level::info;
                break;
            case Level::WARNING:
                spd_level = spdlog::level::warn;
                break;
            case Level::ERROR:
                spd_level = spdlog::level::err;
                break;
            case Level::FATAL:
                spd_level = spdlog::level::critical;
                break;
            default:
                spd_level = spdlog::level::debug;
                break;
            }

            logger_->log(spd_level, "[{}] {}", tag, msg);
        }
    }
} // namespace cqhttp::logging
