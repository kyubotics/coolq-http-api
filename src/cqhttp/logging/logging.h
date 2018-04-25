#pragma once

#include "cqsdk/cqsdk.h"

namespace cqhttp::logging {
    using Level = cq::logging::Level;

    inline int32_t _log(const Level level, const std::string &tag, const std::string &msg) {
        return cq::logging::log(level, tag, msg);
    }

    inline void debug(const std::string &tag, const std::string &msg) { _log(Level::DEBUG, tag, msg); }

    inline void info(const std::string &tag, const std::string &msg) { _log(Level::INFO, tag, msg); }

    inline void info_success(const std::string &tag, const std::string &msg) { _log(Level::INFOSUCCESS, tag, msg); }

    inline void info_recv(const std::string &tag, const std::string &msg) { _log(Level::INFORECV, tag, msg); }

    inline void info_send(const std::string &tag, const std::string &msg) { _log(Level::INFOSEND, tag, msg); }

    inline void warning(const std::string &tag, const std::string &msg) { _log(Level::WARNING, tag, msg); }

    inline void error(const std::string &tag, const std::string &msg) { _log(Level::ERROR, tag, msg); }

    inline void fatal(const std::string &tag, const std::string &msg) { _log(Level::FATAL, tag, msg); }
} // namespace cqhttp::logging
