#pragma once

#include <string>

std::string message_escape(const std::string& msg);

std::string message_unescape(const std::string& msg);

std::string enhance_cq_code(const std::string& msg);
