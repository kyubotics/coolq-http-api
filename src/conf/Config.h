#pragma once

#include "common.h"

struct Config {
    str host;
    int port;
    str post_url;
    str token;
    std::regex pattern;
    str post_message_format;
};
