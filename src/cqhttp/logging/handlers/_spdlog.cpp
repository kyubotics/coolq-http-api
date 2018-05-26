#include "./_spdlog.h"

#undef ERROR

using namespace std;

namespace cqhttp::logging {
    using cq::logging::Level;

    void SpdlogHandler::log(const Level level, const string &tag, const string &msg) const {
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
