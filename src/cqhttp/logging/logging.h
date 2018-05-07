#pragma once

#include "cqsdk/logging.h"

namespace cqhttp::logging {
    using Level = cq::logging::Level;

    void log(Level level, const std::string &tag, const std::string &msg);

    inline void debug(const std::string &tag, const std::string &msg) { logging::log(Level::DEBUG, tag, msg); }
    inline void info(const std::string &tag, const std::string &msg) { logging::log(Level::INFO, tag, msg); }
    inline void info_success(const std::string &tag, const std::string &msg) {
        logging::log(Level::INFOSUCCESS, tag, msg);
    }
    inline void info_recv(const std::string &tag, const std::string &msg) { logging::log(Level::INFORECV, tag, msg); }
    inline void info_send(const std::string &tag, const std::string &msg) { logging::log(Level::INFOSEND, tag, msg); }
    inline void warning(const std::string &tag, const std::string &msg) { logging::log(Level::WARNING, tag, msg); }
    inline void error(const std::string &tag, const std::string &msg) { logging::log(Level::ERROR, tag, msg); }
    inline void fatal(const std::string &tag, const std::string &msg) { logging::log(Level::FATAL, tag, msg); }
} // namespace cqhttp::logging
