#pragma once

#include "web_server/server_http.hpp"

class ApiServer {
public:
    static ApiServer &instance() {
        static ApiServer instance;
        if (!instance.initiated_) {
            instance.init();
        }
        return instance;
    }

    void init();
    void start(const std::string &host, const unsigned short port);
    void stop();

private:
    ApiServer() {}
    ~ApiServer() { server_.stop(); }
    ApiServer(const ApiServer &) = delete;
    void operator=(const ApiServer &) = delete;

    SimpleWeb::Server<SimpleWeb::HTTP> server_;
    std::thread thread_;
    bool initiated_ = false;
};
