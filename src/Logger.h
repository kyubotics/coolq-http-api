#pragma once

#include "common.h"

#include "CQApp.h"

extern CQApp *CQ;

class Logger {
public:
    Logger(CQApp *&app = CQ) : app_(app) {}

    void i(const str &tag, const str &msg) const {
        this->app_->addLog(CQLOG_INFO, tag, msg);
    }

    void i_succ(const str &tag, const str &msg) const {
        this->app_->addLog(CQLOG_INFOSUCCESS, tag, msg);
    }

    void i_recv(const str &tag, const str &msg) const {
        this->app_->addLog(CQLOG_INFORECV, tag, msg);
    }

    void i_send(const str &tag, const str &msg) const {
        this->app_->addLog(CQLOG_INFOSEND, tag, msg);
    }

    void d(const str &tag, const str &msg) const {
        this->app_->addLog(CQLOG_DEBUG, tag, msg);
    }

    void w(const str &tag, const str &msg) const {
        this->app_->addLog(CQLOG_WARNING, tag, msg);
    }

    void e(const str &tag, const str &msg) const {
        this->app_->addLog(CQLOG_ERROR, tag, msg);
    }

    void f(const str &tag, const str &msg) const {
        this->app_->addLog(CQLOG_FATAL, tag, msg);
    }

private:
    CQApp *&app_;
};
