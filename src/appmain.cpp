/**
 * CoolQ HTTP API core.
 */

#include "app.h"

#include <regex>
#include <curl/curl.h>
#include <jansson/jansson.h>

#include "encoding/encoding.h"
#include "helpers.h"
#include "httpd.h"
#include "conf/loader.h"
#include "events.h"

using namespace std;

/*
 * Return app info.
 */
CQEVENT(const char *, AppInfo, 0)
() {
    return CQ_APP_INFO;
}

/**
 * Get auth code.
 */
CQEVENT(int32_t, Initialize, 4)
(int32_t auth_code) {
    CQ = new CQApp(auth_code);
    return 0;
}

/**
 * Initialize plugin, called immediately when plugin is enabled.
 */
static void init() {
    L.d("初始化", "尝试加载配置文件");
    if (load_configuration(CQ->getAppDirectory() + "config.cfg", CQ->config)) {
        L.d("初始化", "加载配置文件成功");
    } else {
        L.e("初始化", "加载配置文件失败，请确定配置文件格式和访问权限是否正确");
    }
}

/**
 * Event: Plugin is enabled.
 */
CQEVENT(int32_t, __eventEnable, 0)
() {
    CQ->enabled = true;
    L.d("启用", "开始初始化");
    init();
    start_httpd();
    L.i("启用", "HTTP API 插件已启用");
    return 0;
}

/**
 * Event: Plugin is disabled.
 */
CQEVENT(int32_t, __eventDisable, 0)
() {
    CQ->enabled = false;
    stop_httpd();
    L.i("停用", "HTTP API 插件已停用");
    return 0;
}

/**
* Event: CoolQ is starting.
*/
CQEVENT(int32_t, __eventStartup, 0)
() {
    // do nothing
    return 0;
}

/**
* Event: CoolQ is exiting.
*/
CQEVENT(int32_t, __eventExit, 0)
() {
    stop_httpd();
    delete CQ;
    CQ = nullptr;
    L.i("停止", "HTTP API 插件已停止");
    return 0;
}

/**
 * Type=21 私聊消息
 * sub_type 子类型，11/来自好友 1/来自在线状态 2/来自群 3/来自讨论组
 */
CQEVENT(int32_t, __eventPrivateMsg, 24)
(int32_t sub_type, int32_t send_time, int64_t from_qq, const char *msg, int32_t font) {
    return event_private_msg(sub_type, send_time, from_qq, decode(msg, Encoding::GBK), font);
}

/**
 * Type=2 群消息
 */
CQEVENT(int32_t, __eventGroupMsg, 36)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const char *from_anonymous, const char *msg, int32_t font) {
    return event_group_msg(sub_type, send_time, from_group, from_qq, decode(from_anonymous, Encoding::GBK), decode(msg, Encoding::GBK), font);
}

/**
 * Type=4 讨论组消息
 */
CQEVENT(int32_t, __eventDiscussMsg, 32)
(int32_t sub_type, int32_t send_time, int64_t from_discuss, int64_t from_qq, const char *msg, int32_t font) {
    return event_discuss_msg(sub_type, send_time, from_discuss, from_qq, decode(msg, Encoding::GBK), font);
}

/**
 * Type=101 群事件-管理员变动
 * sub_type 子类型，1/被取消管理员 2/被设置管理员
 */
CQEVENT(int32_t, __eventSystem_GroupAdmin, 24)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t being_operate_qq) {
    return event_group_admin(sub_type, send_time, from_group, being_operate_qq);
}

/**
 * Type=102 群事件-群成员减少
 * sub_type 子类型，1/群员离开 2/群员被踢 3/自己(即登录号)被踢
 * from_qq 操作者QQ(仅subType为2、3时存在)
 * being_operate_qq 被操作QQ
 */
CQEVENT(int32_t, __eventSystem_GroupMemberDecrease, 32)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, int64_t being_operate_qq) {
    return event_group_member_decrease(sub_type, send_time, from_group, from_qq, being_operate_qq);
}

/**
 * Type=103 群事件-群成员增加
 * sub_type 子类型，1/管理员已同意 2/管理员邀请
 * from_qq 操作者QQ(即管理员QQ)
 * being_operate_qq 被操作QQ(即加群的QQ)
 */
CQEVENT(int32_t, __eventSystem_GroupMemberIncrease, 32)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, int64_t being_operate_qq) {
    return event_group_member_increase(sub_type, send_time, from_group, from_qq, being_operate_qq);
}

/**
 * Type=201 好友事件-好友已添加
 */
CQEVENT(int32_t, __eventFriend_Add, 16)
(int32_t sub_type, int32_t send_time, int64_t from_qq) {
    return event_friend_added(sub_type, send_time, from_qq);
}

/**
 * Type=301 请求-好友添加
 * msg 附言
 * response_flag 反馈标识(处理请求用)
 */
CQEVENT(int32_t, __eventRequest_AddFriend, 24)
(int32_t sub_type, int32_t send_time, int64_t from_qq, const char *msg, const char *response_flag) {
    return event_add_friend_request(sub_type, send_time, from_qq, decode(msg, Encoding::GBK), decode(response_flag, Encoding::GBK));
}

/**
 * Type=302 请求-群添加
 * sub_type 子类型，1/他人申请入群 2/自己(即登录号)受邀入群
 * msg 附言
 * response_flag 反馈标识(处理请求用)
 */
CQEVENT(int32_t, __eventRequest_AddGroup, 32)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const char *msg, const char *response_flag) {
    return event_add_group_request(sub_type, send_time, from_group, from_qq, decode(msg, Encoding::GBK), decode(response_flag, Encoding::GBK));
}
