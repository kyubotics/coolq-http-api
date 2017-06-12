#pragma once

#include "common.h"

struct Config {
    str host;
    int port;
    str post_url;
    str token;
    std::regex pattern;

    Config() : host("0.0.0.0"), port(5700), post_url(""), token(""), pattern(std::regex("")) {}
};
