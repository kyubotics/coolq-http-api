//
// appentry.cpp : Export app entry functions to DLL.
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

#include "app.h"

#include "conf/loader.h"
#include "api/server_class.h"

using namespace std;

/*
* Return app info.
*/
CQEVENT(const char *, AppInfo, 0)
() {
    // CoolQ API version: 9
    return "9," CQAPP_ID;
}

/**
* Get auth code.
*/
CQEVENT(int32_t, Initialize, 4)
(const int32_t auth_code) {
    init_sdk(auth_code);
    return 0;
}

/**
* Event: Plugin is enabled.
*/
CQEVENT(int32_t, Enable, 0)
() {
    static const auto TAG = u8"启用";

    Log::d(TAG, CQAPP_FULLNAME);
    Log::d(TAG, u8"开始初始化");
    sdk->enabled = true;

    if (const auto c = load_configuration(sdk->get_app_directory() + "config.cfg")) {
        config = c.value();
    }

    ApiServer::instance().start(config.host, config.port);

    if (!pool) {
        Log::d(TAG, u8"工作线程池创建成功");
        pool = make_shared<ctpl::thread_pool>(config.thread_pool_size);
    }

    Log::i(TAG, u8"HTTP API 插件已启用");

    //if (sdk->config.auto_check_update) {
    //    check_update(false);
    //}
    return 0;
}

/**
* Event: Plugin is disabled.
*/
CQEVENT(int32_t, Disable, 0)
() {
    static const auto TAG = u8"启用";

    sdk->enabled = false;
    ApiServer::instance().stop();
    if (pool) {
        pool->stop();
        pool = nullptr;
    }

    Log::i(TAG, u8"HTTP API 插件已停用");
    return 0;
}

/**
* Event: CoolQ is starting.
*/
CQEVENT(int32_t, Start, 0)
() {
    return 0;
}

/**
* Event: CoolQ is exiting.
*/
CQEVENT(int32_t, Exit, 0)
() {
    ApiServer::instance().stop();
    if (pool) pool->stop();
    return 0;
}

/**
* 检查更新菜单项
*/
CQEVENT(int32_t, __menu_check_update, 0)() {
    //check_update(true);
    return 0;
}
