// 
// Logger.h : Wrap "add_log" method of CQApp.
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

#include "common.h"

#include "CQApp.h"

extern CQApp *CQ;

class Logger {
public:
    Logger(CQApp *&app = CQ) : app_(app) {}

    void i(const str &tag, const str &msg) const {
        if (this->app_) {
            this->app_->add_log(CQLOG_INFO, tag, msg);
        }
    }

    void i_succ(const str &tag, const str &msg) const {
        if (this->app_) {
            this->app_->add_log(CQLOG_INFOSUCCESS, tag, msg);
        }
    }

    void i_recv(const str &tag, const str &msg) const {
        if (this->app_) {
            this->app_->add_log(CQLOG_INFORECV, tag, msg);
        }
    }

    void i_send(const str &tag, const str &msg) const {
        if (this->app_) {
            this->app_->add_log(CQLOG_INFOSEND, tag, msg);
        }
    }

    void d(const str &tag, const str &msg) const {
        if (this->app_) {
            this->app_->add_log(CQLOG_DEBUG, tag, msg);
        }
    }

    void w(const str &tag, const str &msg) const {
        if (this->app_) {
            this->app_->add_log(CQLOG_WARNING, tag, msg);
        }
    }

    void e(const str &tag, const str &msg) const {
        if (this->app_) {
            this->app_->add_log(CQLOG_ERROR, tag, msg);
        }
    }

    void f(const str &tag, const str &msg) const {
        if (this->app_) {
            this->app_->add_log(CQLOG_FATAL, tag, msg);
        }
    }

private:
    CQApp *&app_;
};
