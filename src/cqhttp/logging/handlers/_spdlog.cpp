#include "./_spdlog.h"

#undef ERROR

using namespace std;

namespace cqhttp::logging {
    using cq::logging::Level;

    spdlog::level::level_enum SpdlogHandler::convert_level(const Level level) {
        switch (level) {
        case Level::DEBUG:
            return spdlog::level::debug;
        case Level::INFO:
        case Level::INFOSUCCESS:
        case Level::INFORECV:
        case Level::INFOSEND:
            return spdlog::level::info;
        case Level::WARNING:
            return spdlog::level::warn;
        case Level::ERROR:
            return spdlog::level::err;
        case Level::FATAL:
            return spdlog::level::critical;
        default:
            return spdlog::level::debug;
        }
    }
} // namespace cqhttp::logging
