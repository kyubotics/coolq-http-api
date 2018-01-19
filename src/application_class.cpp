// 
// application_class.cpp : Implement Application class
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

#include "./application_class.h"

#include "app.h"

#include <boost/filesystem.hpp>

#include "conf/loader.h"
#include "service/hub_class.h"
#include "event/filter.h"

using namespace std;
namespace fs = boost::filesystem;

void Application::initialize(const int32_t auth_code) {
    init_sdk(auth_code);
    initialized_ = true;

    restart_worker_running_ = true;
    restart_worker_thread_ = thread([&]() {
        static const auto tag = u8"重启";
        while (restart_worker_running_) {
            if (should_restart_) {
                // this may not be thread-safe, but currently it's ok
                should_restart_ = false;
                if (restart_delay_ > 0) {
                    Log::i(tag, u8"HTTP API 插件将在 " + to_string(restart_delay_) + u8" 毫秒后重启");
                }
                Sleep(restart_delay_);
                disable();
                enable();
                Log::i(tag, u8"HTTP API 插件重启成功");
            }
            Sleep(500); // wait 500 ms for the next check
        }
    });
}

void Application::enable() {
    static const auto TAG = u8"启用";

    if (!initialized_ || enabled_) {
        return;
    }

    Log::d(TAG, CQAPP_FULLNAME);
    Log::d(TAG, u8"开始初始化");

    if (const auto c = load_configuration(sdk->directories().app() + "config.cfg")) {
        config = c.value();
    }

    ServiceHub::instance().start();

    GlobalFilter::reset();
    if (config.use_filter) {
        GlobalFilter::load(sdk->directories().app() + "filter.json");
    }

    if (!pool) {
        Log::d(TAG, u8"工作线程池创建成功");
        pool = make_shared<ctpl::thread_pool>(
            config.thread_pool_size > 0 ? config.thread_pool_size : thread::hardware_concurrency() * 2 + 1
        );
    }

    enabled_ = true;
    Log::i(TAG, u8"HTTP API 插件已启用");
}

void Application::disable() {
    static const auto TAG = u8"停用";

    if (!enabled_) {
        return;
    }

    ServiceHub::instance().stop();

    if (pool) {
        pool->stop();
        pool = nullptr;
        Log::d(TAG, u8"工作线程池关闭成功");
    }

    enabled_ = false;
    Log::i(TAG, u8"HTTP API 插件已停用");
}

void Application::exit() {
    disable();

    restart_worker_running_ = false;
    if (restart_worker_thread_.joinable()) {
        restart_worker_thread_.join();
    }
}

void Application::restart_async(const unsigned long delay_millisecond) {
    restart_delay_ = delay_millisecond;
    should_restart_ = true; // this will let the restart worker do it
}

bool Application::is_locked() const {
    return fs::exists(ansi(sdk->directories().app() + "app.lock"));
}
