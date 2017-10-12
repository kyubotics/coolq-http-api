//
// evententry.cpp : Export event entry functions to DLL.
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

#include <cpprest/http_client.h>
#include <nlohmann/json.hpp>

#include "conf/loader.h"

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
    sdk = Sdk(auth_code);
    return 0;
}

/**
 * Event: Plugin is enabled.
 */
CQEVENT(int32_t, __event_enable, 0)
() {
    const auto tag = u8"启用";
    Log::d(tag, CQAPP_FULLNAME);
    Log::d(tag, u8"开始初始化");
    sdk->enabled = true;

    if (const auto config = load_configuration(sdk->get_app_directory() + "config.cfg")) {
        sdk->config = config.value();
    }

    //start_httpd();
    Log::i(tag, u8"HTTP API 插件已启用");

    //    if (sdk->config.auto_check_update) {
    //        check_update(false);
    //    }
    return 0;
}

/**
 * Event: Plugin is disabled.
 */
CQEVENT(int32_t, __event_disable, 0)
() {
    return 0;
}

/**
* Event: CoolQ is starting.
*/
CQEVENT(int32_t, __event_start, 0)
() {
    return 0;
}

/**
* Event: CoolQ is exiting.
*/
CQEVENT(int32_t, __event_exit, 0)
() {
    return 0;
}

using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;
using json = nlohmann::json;

/**
 * Type=21 私聊消息
 * sub_type 子类型，11/来自好友 1/来自在线状态 2/来自群 3/来自讨论组
 */
CQEVENT(int32_t, __event_private_msg, 24)
(int32_t sub_type, int32_t send_time, int64_t from_qq, const char *msg, int32_t font) {
    sdk->send_private_msg(1002647525, u8"你好");

    //    http_client client(L"http://127.0.0.1:8080/");
    //    client.request(methods::GET).then([](http_response resp) {
    //        if (resp.status_code() == status_codes::OK) {
    //            return resp.extract_json();
    //        }
    //        return pplx::task_from_result(json::value());
    //    }).then([](pplx::task<json::value> prev_task) {
    //        const auto &obj = prev_task.get();
    //        sdk->send_private_msg(1002647525, ws2s(obj.at(L"c").as_string()));
    //    }).wait();

    http_client client(L"http://127.0.0.1:8080/");
    client.request(methods::GET).then([](http_response resp) -> pplx::task<wstring> {
        if (resp.status_code() == status_codes::OK) {
            return resp.extract_string(true);
        }
        return pplx::task_from_result(wstring());
    }).then([](pplx::task<wstring> task) {
        string json_string = ws2s(task.get());
        sdk->send_private_msg(1002647525, json_string);

        auto j = json::parse(json_string);
        sdk->send_private_msg(1002647525, j["c"].get<string>());
    }).wait();

    return CQEVENT_IGNORE;
    //return event_private_msg(sub_type, send_time, from_qq, string_decode(msg, Encoding::ANSI), font);
}

///**
// * Type=2 群消息
// */
//CQEVENT(int32_t, __event_group_msg, 36)
//(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const char *from_anonymous, const char *msg, int32_t font) {
//    return event_group_msg(sub_type, send_time, from_group, from_qq, string_decode(from_anonymous, Encoding::ANSI), string_decode(msg, Encoding::ANSI), font);
//}
//
///**
// * Type=4 讨论组消息
// */
//CQEVENT(int32_t, __event_discuss_msg, 32)
//(int32_t sub_type, int32_t send_time, int64_t from_discuss, int64_t from_qq, const char *msg, int32_t font) {
//    return event_discuss_msg(sub_type, send_time, from_discuss, from_qq, string_decode(msg, Encoding::ANSI), font);
//}
//
///**
// * Type=11 群事件-文件上传
// */
//CQEVENT(int32_t, __event_group_upload, 28)
//(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const char *file) {
//    return event_group_upload(sub_type, send_time, from_group, from_qq, string_decode(file, Encoding::ANSI));
//}
//
///**
// * Type=101 群事件-管理员变动
// * sub_type 子类型，1/被取消管理员 2/被设置管理员
// */
//CQEVENT(int32_t, __event_group_admin, 24)
//(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t being_operate_qq) {
//    return event_group_admin(sub_type, send_time, from_group, being_operate_qq);
//}
//
///**
// * Type=102 群事件-群成员减少
// * sub_type 子类型，1/群员离开 2/群员被踢 3/自己(即登录号)被踢
// * from_qq 操作者QQ(仅subType为2、3时存在)
// * being_operate_qq 被操作QQ
// */
//CQEVENT(int32_t, __event_group_member_decrease, 32)
//(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, int64_t being_operate_qq) {
//    return event_group_member_decrease(sub_type, send_time, from_group, from_qq, being_operate_qq);
//}
//
///**
// * Type=103 群事件-群成员增加
// * sub_type 子类型，1/管理员已同意 2/管理员邀请
// * from_qq 操作者QQ(即管理员QQ)
// * being_operate_qq 被操作QQ(即加群的QQ)
// */
//CQEVENT(int32_t, __event_group_member_increase, 32)
//(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, int64_t being_operate_qq) {
//    return event_group_member_increase(sub_type, send_time, from_group, from_qq, being_operate_qq);
//}
//
///**
// * Type=201 好友事件-好友已添加
// */
//CQEVENT(int32_t, __event_friend_add, 16)
//(int32_t sub_type, int32_t send_time, int64_t from_qq) {
//    return event_friend_add(sub_type, send_time, from_qq);
//}
//
///**
// * Type=301 请求-好友添加
// * msg 附言
// * response_flag 反馈标识(处理请求用)
// */
//CQEVENT(int32_t, __event_add_friend_request, 24)
//(int32_t sub_type, int32_t send_time, int64_t from_qq, const char *msg, const char *response_flag) {
//    return event_add_friend_request(sub_type, send_time, from_qq, string_decode(msg, Encoding::ANSI), string_decode(response_flag, Encoding::ANSI));
//}
//
///**
// * Type=302 请求-群添加
// * sub_type 子类型，1/他人申请入群 2/自己(即登录号)受邀入群
// * msg 附言
// * response_flag 反馈标识(处理请求用)
// */
//CQEVENT(int32_t, __event_add_group_request, 32)
//(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const char *msg, const char *response_flag) {
//    return event_add_group_request(sub_type, send_time, from_group, from_qq, string_decode(msg, Encoding::ANSI), string_decode(response_flag, Encoding::ANSI));
//}
//
///**
// * 检查更新菜单项
// */
//CQEVENT(int32_t, __menu_check_update, 0)() {
//    check_update(true);
//    return 0;
//}
