#pragma once

#include <string>

#include "cqp.h"
#include "encoding.h"

extern int ac;

#define LOG8(level, tag, msg) CQ_addLog(ac, level, tag, utf8_to_gbk(std::string(msg).c_str()).c_str());
#define LOG8_I(tag, msg) CQ_addLog(ac, CQLOG_INFO, tag, utf8_to_gbk(std::string(msg).c_str()).c_str());
#define LOG8_D(tag, msg) CQ_addLog(ac, CQLOG_DEBUG, tag, utf8_to_gbk(std::string(msg).c_str()).c_str());
#define LOG8_W(tag, msg) CQ_addLog(ac, CQLOG_WARNING, tag, utf8_to_gbk(std::string(msg).c_str()).c_str());
#define LOG8_E(tag, msg) CQ_addLog(ac, CQLOG_ERROR, tag, utf8_to_gbk(std::string(msg).c_str()).c_str());

#define LOG(level, tag, msg) CQ_addLog(ac, level, tag, std::string(msg).c_str());
#define LOG_I(tag, msg) CQ_addLog(ac, CQLOG_INFO, tag, std::string(msg).c_str());
#define LOG_D(tag, msg) CQ_addLog(ac, CQLOG_DEBUG, tag, std::string(msg).c_str());
#define LOG_W(tag, msg) CQ_addLog(ac, CQLOG_WARNING, tag, std::string(msg).c_str());
#define LOG_E(tag, msg) CQ_addLog(ac, CQLOG_ERROR, tag, std::string(msg).c_str());
