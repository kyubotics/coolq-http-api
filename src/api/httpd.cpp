#include "httpd.h"

#include "app.h"

#include <event2/event.h>
#include <event2/http.h>
#include <thread>
#include <atomic>

#include "main_handler.h"
#include "conf/Config.h"

using namespace std;

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

    httpd_thread = thread([]() {
            // NOTE: due to an unknown issue of CoolQ, it's impossible to call any log functions in this lambda

            httpd_thread_running = true;

            auto config = CQ->config;

            httpd_event_base = event_base_new();
            httpd_event = evhttp_new(httpd_event_base);

            evhttp_set_gencb(httpd_event, api_main_handler, nullptr);
            evhttp_bind_socket(httpd_event, config.host.c_str(), config.port);

            event_base_dispatch(httpd_event_base); // infinite event loop

            httpd_thread_running = false;
        });

    L.d("开启", "开启 HTTP 守护线程成功，开始监听 http://" + CQ->config.host + ":" + str(CQ->config.port));
}

/**
 * Stop HTTP daemon thread.
 */
void stop_httpd() {
    if (httpd_thread_running) {
        if (httpd_event_base) {
            event_base_loopbreak(httpd_event_base);
        }
        if (httpd_event) {
            evhttp_free(httpd_event);
            httpd_event = nullptr;
        }
        if (httpd_event_base) {
            event_base_free(httpd_event_base);
            httpd_event_base = nullptr;
        }
        httpd_thread.join();
        L.d("关闭", "已关闭后台 HTTP 守护线程");
    }
}
