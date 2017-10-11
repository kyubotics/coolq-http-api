// 
// Log.h : Wrap "add_log" method of CQApp.
// 
// Copyright (C) 2017  Richard Chien <richardchienthebest@gmail.com>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

#pragma once

#include "sdk_class.h"
extern std::optional<Sdk> sdk;

struct Log {
    static void i(const std::string &tag, const std::string &msg) {
        log(CQLOG_INFO, tag, msg);
    }

    static void i_succ(const std::string &tag, const std::string &msg) {
        log(CQLOG_INFOSUCCESS, tag, msg);
    }

    static void i_recv(const std::string &tag, const std::string &msg) {
        log(CQLOG_INFORECV, tag, msg);
    }

    static void i_send(const std::string &tag, const std::string &msg) {
        log(CQLOG_INFOSEND, tag, msg);
    }

    static void d(const std::string &tag, const std::string &msg) {
        log(CQLOG_DEBUG, tag, msg);
    }

    static void w(const std::string &tag, const std::string &msg) {
        log(CQLOG_WARNING, tag, msg);
    }

    static void e(const std::string &tag, const std::string &msg) {
        log(CQLOG_ERROR, tag, msg);
    }

    static void f(const std::string &tag, const std::string &msg) {
        log(CQLOG_FATAL, tag, msg);
    }

    static void log(int level, const std::string &tag, const std::string &msg) {
        if (sdk) {
            sdk->add_log(level, tag, msg);
        }
    }
};
