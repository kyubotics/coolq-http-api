#pragma once

struct Config {
    std::string host;
    int port;
    std::string post_url;
    std::string token;
    std::regex pattern;

    Config() : host("0.0.0.0"), port(5700),
        post_url(""), token(""), pattern(std::regex("")) {}
};
