#include "httpd.h"

#include <event2/event.h>
#include <event2/http.h>
#include <WinSock2.h>
#include <thread>
#include <atomic>

#include "request.h"
#include "helpers.h"
#include "conf/Config.h"

using namespace std;

extern Config httpd_config; // in appmain.cpp

static thread httpd_thread;
static atomic<bool> httpd_thread_running = false;
static struct event_base *httpd_event_base = nullptr;
static struct evhttp *httpd_event = nullptr;

/**
 * Start HTTP daemon thread.
 */
void start_httpd() {
    // try to stop httpd first, in case of error
    stop_httpd();

    auto thread_func = []() {
        httpd_thread_running = true;

        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);

        httpd_event_base = event_base_new();
        httpd_event = evhttp_new(httpd_event_base);

        evhttp_set_gencb(httpd_event, cqhttp_main_handler, nullptr);
        evhttp_bind_socket(httpd_event, httpd_config.host.c_str(), httpd_config.port);

        LOG_D("监听", string("开始监听 http://") + httpd_config.host + ":" + itos(httpd_config.port));

        event_base_dispatch(httpd_event_base);

        httpd_thread_running = false;
    };

    httpd_thread = thread(thread_func);
    LOG_D("开启", "开启 HTTP 守护线程成功");
}

/**
 * Stop HTTP daemon thread.
 */
void stop_httpd() {
    //    if (httpd_thread_handle) {
    if (httpd_thread_running) {
        if (httpd_event_base) {
            event_base_loopbreak(httpd_event_base);
        }
        if (httpd_event) {
            evhttp_free(httpd_event);
        }
        // it seems that the following cannot be done properly, don't know why
        // if (httpd_event_base) {
        //     event_base_free(httpd_event_base);
        // }
        WSACleanup();
        httpd_event_base = nullptr;
        httpd_event = nullptr;
        LOG_D("关闭", "已关闭后台 HTTP 守护线程")
    }
}
