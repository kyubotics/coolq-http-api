/**
 * CoolQ HTTP API core.
 */

#include "stdafx.h"

#include <string>
#include <sstream>
#include <fstream>
#include <regex>
#include <curl/curl.h>
#include <jansson.h>
#include <event2/event.h>
#include <event2/http.h>
#include <WinSock2.h>

#include "encoding.h"
#include "misc_functions.h"
#include "request.h"
#include "ini.h"
#include "cqcode.h"

using namespace std;

int ac = -1; // AuthCode
bool enabled = false;

HANDLE httpd_thread_handle = NULL;
struct event_base *httpd_event_base = NULL;
struct evhttp *httpd_event = NULL;

struct cqhttp_config {
    string host;
    int port;
    string post_url;
    string token;
    regex pattern;

    cqhttp_config() : host("0.0.0.0"), port(5700),
                      post_url(""), token(""), pattern(regex("")) {}
} httpd_config;

/**
 * For other files to get token.
 */
string get_httpd_config_token() {
    return httpd_config.token;
}

/*
* Return add info.
*/
CQEVENT(const char *, AppInfo, 0)
() {
    return CQAPPINFO;
}

/**
 * Get AuthCode.
 */
CQEVENT(int32_t, Initialize, 4)
(int32_t AuthCode) {
    ac = AuthCode;
    return 0;
}

static int parse_conf_handler(void *user, const char *section, const char *name, const char *value) {
    static string login_qq_atr = itos(CQ_getLoginQQ(ac));

    struct cqhttp_config *config = (struct cqhttp_config *)user;
    if (string(section) == "general" || (isnumber(section) && login_qq_atr == section)) {
        string field = name;
        if (field == "host")
            config->host = value;
        else if (field == "port")
            config->port = atoi(value);
        else if (field == "post_url")
            config->post_url = value;
        else if (field == "token")
            config->token = value;
        else if (field == "pattern")
            config->pattern = regex(value);
        else
            return 0; /* unknown name, error */
    } else
        return 0; /* unknown section, error */
    return 1;
}

/**
 * Initialize plugin, called immediately when plugin is enabled.
 */
static void init() {
    LOG_D("启用", "初始化");

    string conf_path = string(CQ_getAppDirectory(ac)) + "config.cfg";
    FILE *conf_file = NULL;
    fopen_s(&conf_file, conf_path.c_str(), "r");
    if (!conf_file) {
        // first init, save default config
        LOG_D("启用", "没有找到配置文件，写入默认配置");
        ofstream file(conf_path);
        file << "[general]\nhost=0.0.0.0\nport=5700\npost_url=\ntoken=\n";
    } else {
        // load from config file
        LOG_D("启用", "读取配置文件");
        ini_parse_file(conf_file, parse_conf_handler, &httpd_config);
        fclose(conf_file);
    }
}

/**
 * Cleanup plugin, called after all other operations when plugin is disabled.
 */
static void cleanup() {
    // do nothing currently
}

/**
 * Portal function of HTTP daemon thread.
 */
static DWORD WINAPI httpd_thread_func(LPVOID lpParam) {
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    httpd_event_base = event_base_new();
    httpd_event = evhttp_new(httpd_event_base);

    evhttp_set_gencb(httpd_event, cqhttp_main_handler, NULL);
    evhttp_bind_socket(httpd_event, httpd_config.host.c_str(), httpd_config.port);

    stringstream ss;
    ss << "开始监听 " << httpd_config.host << ":" << httpd_config.port;
    LOG_D("HTTP线程", ss.str());

    event_base_dispatch(httpd_event_base);
    return 0;
}

/**
 * Start HTTP daemon thread.
 */
static void start_httpd() {
    httpd_thread_handle = CreateThread(NULL, // default security attributes
                                           0, // use default stack size
                                           httpd_thread_func, // thread function name
                                           NULL, // argument to thread function
                                           0, // use default creation flags
                                           NULL); // returns the thread identifier
    if (!httpd_thread_handle) {
        LOG_E("启用", "启动 HTTP 守护线程失败");
    } else {
        LOG_D("启用", "启动 HTTP 守护线程成功");
    }
}

/**
 * Stop HTTP daemon thread.
 */
static void stop_httpd() {
    if (httpd_thread_handle) {
        if (httpd_event_base) {
            event_base_loopbreak(httpd_event_base);
        }
        if (httpd_event) {
            evhttp_free(httpd_event);
        }
        // if (httpd_event_base)
        // {
        //     event_base_free(httpd_event_base);
        // }
        WSACleanup();
        CloseHandle(httpd_thread_handle);
        httpd_thread_handle = NULL;
        httpd_event_base = NULL;
        httpd_event = NULL;
        LOG_D("停用", "已关闭后台 HTTP 守护线程")
    }
}

/**
 * Event: plugin is enabled.
 */
CQEVENT(int32_t, __eventEnable, 0)
() {
    enabled = true;
    init();
    start_httpd();
    return 0;
}

/**
 * Event: plugin is disabled.
 */
CQEVENT(int32_t, __eventDisable, 0)
() {
    enabled = false;
    stop_httpd();
    cleanup();
    return 0;
}

#define SHOULD_POST (httpd_config.post_url.length() > 0)
#define MATCH_PATTERN(utf8_msg) regex_search(utf8_msg, httpd_config.pattern)

struct cqhttp_post_response {
    bool succeeded; // post event succeeded or not (the server returning 2xx means success)
    json_t *json; // response json of the post request, is NULL if response body is empty
    cqhttp_post_response() : succeeded(false), json(NULL) {};
};

static cqhttp_post_response post_event(json_t *json, const string &event_name) {
    char *json_str = json_dumps(json, 0);
    CURL *curl = curl_easy_init();
    cqhttp_post_response response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, httpd_config.post_url.c_str());

        stringstream resp_stream;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_stringstream_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp_stream);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);

        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "User-Agent: " CQAPPFULLNAME);
        chunk = curl_slist_append(chunk, "Content-Type: application/json");
        if (httpd_config.token != "")
            chunk = curl_slist_append(chunk, (string("Authorization: token ") + httpd_config.token).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            long status_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
            if (status_code >= 200 && status_code < 300) {
                response.succeeded = true;
                response.json = json_loads(resp_stream.str().c_str(), 0, NULL);
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(chunk);
    }
    free(json_str);
    LOG_D("HTTP上报", string(event_name) + " 事件上报" + (response.succeeded ? "成功" : "失败"));

    if (response.json != NULL) {
        char *tmp = json_dumps(response.json, 0);
        if (tmp != NULL) {
            LOG_D("HTTP上报", string("收到响应数据：") + utf8_to_gbk(tmp));
            free(tmp);
        }
    }

    return response;
}

static int release_response(cqhttp_post_response &response) {
    bool block = json_is_true(json_object_get(response.json, "block"));
    json_decref(response.json);
    return block ? EVENT_BLOCK : EVENT_IGNORE;
}

/**
 * Type=21 私聊消息
 * sub_type 子类型，11/来自好友 1/来自在线状态 2/来自群 3/来自讨论组
 */
CQEVENT(int32_t, __eventPrivateMsg, 24)
(int32_t sub_type, int32_t send_time, int64_t from_qq, const char *msg, int32_t font) {
    string utf8_msg = gbk_to_utf8(msg);
    if (SHOULD_POST && MATCH_PATTERN(utf8_msg)) {
        const char *sub_type_str = "unknown";
        switch (sub_type) {
        case 11:
            sub_type_str = "friend";
            break;
        case 1:
            sub_type_str = "other";
            break;
        case 2:
            sub_type_str = "group";
            break;
        case 3:
            sub_type_str = "discuss";
            break;
        }
        utf8_msg = enhance_cq_code(utf8_msg, CQCODE_ENHANCE_INCOMING);
        json_t *json = json_pack("{s:s, s:s, s:s, s:i, s:I, s:s}",
                                 "post_type", "message",
                                 "message_type", "private",
                                 "sub_type", sub_type_str,
                                 "time", send_time,
                                 "user_id", from_qq,
                                 "message", utf8_msg.c_str());
        cqhttp_post_response response = post_event(json, "私聊消息");
        json_decref(json);

        if (response.json != NULL) {
            const char *reply_cstr = json_string_value(json_object_get(response.json, "reply"));
            if (reply_cstr != NULL) {
                string reply_gbk = utf8_to_gbk(reply_cstr);
                CQ_sendPrivateMsg(ac, from_qq, reply_gbk.c_str());
            }

            return release_response(response);
        }
    }
    return EVENT_IGNORE;
}

/**
 * Type=2 群消息
 */
CQEVENT(int32_t, __eventGroupMsg, 36)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const char *from_anonymous, const char *msg, int32_t font) {
    string utf8_msg = gbk_to_utf8(msg);
    if (SHOULD_POST && MATCH_PATTERN(utf8_msg)) {
        string utf8_anonymous = "";
        bool is_anonymous = false;
        if (from_anonymous && strlen(from_anonymous) > 0) {
            is_anonymous = true;
            smatch match;
            if (regex_match(utf8_msg, match, regex("&#91;(.+?)&#93;:(.*)"))) {
                utf8_anonymous = match.str(1);
                utf8_msg = match.str(2);
            }
        }
        utf8_msg = enhance_cq_code(utf8_msg, CQCODE_ENHANCE_INCOMING);
        json_t *json = json_pack("{s:s, s:s, s:i, s:I, s:I, s:s, s:s, s:s}",
                                 "post_type", "message",
                                 "message_type", "group",
                                 "time", send_time,
                                 "group_id", from_group,
                                 "user_id", from_qq,
                                 "anonymous", utf8_anonymous.c_str(),
                                 "anonymous_flag", gbk_to_utf8(from_anonymous).c_str(),
                                 "message", utf8_msg.c_str());
        cqhttp_post_response response = post_event(json, "群消息");
        json_decref(json);

        if (response.json != NULL) {
            const char *reply_cstr = json_string_value(json_object_get(response.json, "reply"));
            string reply = reply_cstr ? reply_cstr : "";

            // check if should at sender
            json_t *at_sender_json = json_object_get(response.json, "at_sender");
            bool at_sender = true;
            if (json_is_boolean(at_sender_json) && !json_boolean_value(at_sender_json))
                at_sender = false;
            if (at_sender && !is_anonymous)
                reply = "[CQ:at,qq=" + itos(from_qq) + "] " + reply;

            // send reply if needed
            if (reply_cstr != NULL) {
                string reply_gbk = utf8_to_gbk(reply.c_str());
                CQ_sendGroupMsg(ac, from_group, reply_gbk.c_str());
            }

            // kick sender if needed
            bool kick = json_is_true(json_object_get(response.json, "kick"));
            if (kick && !is_anonymous)
                CQ_setGroupKick(ac, from_group, from_qq, 0);

            // ban sender if needed
            bool ban = json_is_true(json_object_get(response.json, "ban"));
            if (ban) {
                if (is_anonymous)
                    CQ_setGroupAnonymousBan(ac, from_group, from_anonymous, 30 * 60);
                else
                    CQ_setGroupBan(ac, from_group, from_qq, 30 * 60);
            }

            return release_response(response);
        }
    }
    return EVENT_IGNORE;
}

/**
 * Type=4 讨论组消息
 */
CQEVENT(int32_t, __eventDiscussMsg, 32)
(int32_t sub_Type, int32_t send_time, int64_t from_discuss, int64_t from_qq, const char *msg, int32_t font) {
    string utf8_msg = gbk_to_utf8(msg);
    if (SHOULD_POST && MATCH_PATTERN(utf8_msg)) {
        utf8_msg = enhance_cq_code(utf8_msg, CQCODE_ENHANCE_INCOMING);
        json_t *json = json_pack("{s:s, s:s, s:i, s:I, s:I, s:s}",
                                 "post_type", "message",
                                 "message_type", "discuss",
                                 "time", send_time,
                                 "discuss_id", from_discuss,
                                 "user_id", from_qq,
                                 "message", utf8_msg.c_str());
        cqhttp_post_response response = post_event(json, "讨论组消息");
        json_decref(json);

        if (response.json != NULL) {
            const char *reply_cstr = json_string_value(json_object_get(response.json, "reply"));
            string reply = reply_cstr ? reply_cstr : "";

            // check if should at sender
            json_t *at_sender_json = json_object_get(response.json, "at_sender");
            bool at_sender = true;
            if (json_is_boolean(at_sender_json) && !json_boolean_value(at_sender_json))
                at_sender = false;
            if (at_sender)
                reply = "[CQ:at,qq=" + itos(from_qq) + "] " + reply;

            // send reply if needed
            if (reply_cstr != NULL) {
                string reply_gbk = utf8_to_gbk(reply.c_str());
                CQ_sendDiscussMsg(ac, from_discuss, reply_gbk.c_str());
            }

            return release_response(response);
        }
    }
    return EVENT_IGNORE;
}

/**
 * Type=101 群事件-管理员变动
 * sub_type 子类型，1/被取消管理员 2/被设置管理员
 */
CQEVENT(int32_t, __eventSystem_GroupAdmin, 24)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t being_operate_qq) {
    if (SHOULD_POST) {
        const char *sub_type_str = "unknown";
        switch (sub_type) {
        case 1:
            sub_type_str = "unset";
            break;
        case 2:
            sub_type_str = "set";
            break;
        }
        json_t *json = json_pack("{s:s, s:s, s:s, s:i, s:I, s:I}",
                                 "post_type", "event",
                                 "event", "group_admin",
                                 "sub_type", sub_type_str,
                                 "time", send_time,
                                 "group_id", from_group,
                                 "user_id", being_operate_qq);
        cqhttp_post_response response = post_event(json, "群管理员变动");
        json_decref(json);

        if (response.json != NULL) {
            return release_response(response);
        }
    }
    return EVENT_IGNORE;
}

/**
 * Type=102 群事件-群成员减少
 * sub_type 子类型，1/群员离开 2/群员被踢 3/自己(即登录号)被踢
 * from_qq 操作者QQ(仅subType为2、3时存在)
 * being_operate_qq 被操作QQ
 */
CQEVENT(int32_t, __eventSystem_GroupMemberDecrease, 32)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, int64_t being_operate_qq) {
    if (SHOULD_POST) {
        const char *sub_type_str = "unknown";
        switch (sub_type) {
        case 1:
            sub_type_str = "leave";
            break;
        case 2:
            if (being_operate_qq != CQ_getLoginQQ(ac)) {
                // the one been kicked out is not me
                sub_type_str = "kick";
                break;
            }
        case 3:
            sub_type_str = "kick_me";
            break;
        }
        json_t *json = json_pack("{s:s, s:s, s:s, s:i, s:I, s:I, s:I}",
                                 "post_type", "event",
                                 "event", "group_decrease",
                                 "sub_type", sub_type_str,
                                 "time", send_time,
                                 "group_id", from_group,
                                 "operator_id", sub_type == 1 ? being_operate_qq /* leave by him/herself */ : from_qq,
                                 "user_id", being_operate_qq);
        cqhttp_post_response response = post_event(json, "群成员减少");
        json_decref(json);

        if (response.json != NULL) {
            return release_response(response);
        }
    }
    return EVENT_IGNORE;
}

/**
 * Type=103 群事件-群成员增加
 * sub_type 子类型，1/管理员已同意 2/管理员邀请
 * from_qq 操作者QQ(即管理员QQ)
 * being_operate_qq 被操作QQ(即加群的QQ)
 */
CQEVENT(int32_t, __eventSystem_GroupMemberIncrease, 32)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, int64_t being_operate_qq) {
    if (SHOULD_POST) {
        const char *sub_type_str = "unknown";
        switch (sub_type) {
        case 1:
            sub_type_str = "approve";
            break;
        case 2:
            sub_type_str = "invite";
            break;
        }
        json_t *json = json_pack("{s:s, s:s, s:s, s:i, s:I, s:I, s:I}",
                                 "post_type", "event",
                                 "event", "group_increase",
                                 "sub_type", sub_type_str,
                                 "time", send_time,
                                 "group_id", from_group,
                                 "operator_id", from_qq,
                                 "user_id", being_operate_qq);
        cqhttp_post_response response = post_event(json, "群成员增加");
        json_decref(json);

        if (response.json != NULL) {
            return release_response(response);
        }
    }
    return EVENT_IGNORE;
}

/**
 * Type=201 好友事件-好友已添加
 */
CQEVENT(int32_t, __eventFriend_Add, 16)
(int32_t sub_type, int32_t send_time, int64_t from_qq) {
    if (SHOULD_POST) {
        json_t *json = json_pack("{s:s, s:s, s:i, s:I}",
                                 "post_type", "event",
                                 "event", "friend_added",
                                 "time", send_time,
                                 "user_id", from_qq);
        cqhttp_post_response response = post_event(json, "好友已添加");
        json_decref(json);

        if (response.json != NULL) {
            return release_response(response);
        }
    }
    return EVENT_IGNORE;
}

/**
 * Type=301 请求-好友添加
 * msg 附言
 * response_flag 反馈标识(处理请求用)
 */
CQEVENT(int32_t, __eventRequest_AddFriend, 24)
(int32_t sub_type, int32_t send_time, int64_t from_qq, const char *msg, const char *response_flag) {
    if (SHOULD_POST) {
        json_t *json = json_pack("{s:s, s:s, s:i, s:I, s:s, s:s}",
                                 "post_type", "request",
                                 "request_type", "friend",
                                 "time", send_time,
                                 "user_id", from_qq,
                                 "message", gbk_to_utf8(msg).c_str(),
                                 "flag", gbk_to_utf8(response_flag).c_str());
        cqhttp_post_response response = post_event(json, "好友添加请求");
        json_decref(json);

        if (response.json != NULL) {
            // approve or reject request if needed
            json_t *approve_json = json_object_get(response.json, "approve");
            if (json_is_boolean(approve_json)) {
                // the action is specified
                bool approve = json_boolean_value(approve_json);
                const char *remark_cstr = json_string_value(json_object_get(response.json, "remark"));
                string remark = remark_cstr ? remark_cstr : "";
                string remark_gbk = utf8_to_gbk(remark.c_str());
                CQ_setFriendAddRequest(ac, response_flag, approve ? REQUEST_ALLOW : REQUEST_DENY, remark_gbk.c_str());
            }

            return release_response(response);
        }
    }
    return EVENT_IGNORE;
}

/**
 * Type=302 请求-群添加
 * sub_type 子类型，1/他人申请入群 2/自己(即登录号)受邀入群
 * msg 附言
 * response_flag 反馈标识(处理请求用)
 */
CQEVENT(int32_t, __eventRequest_AddGroup, 32)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const char *msg, const char *response_flag) {
    if (SHOULD_POST) {
        const char *sub_type_str = "unknown";
        switch (sub_type) {
        case 1:
            sub_type_str = "add";
            break;
        case 2:
            sub_type_str = "invite";
            break;
        }
        json_t *json = json_pack("{s:s, s:s, s:s, s:i, s:I, s:I, s:s, s:s}",
                                 "post_type", "request",
                                 "request_type", "group",
                                 "sub_type", sub_type_str,
                                 "time", send_time,
                                 "group_id", from_group,
                                 "user_id", from_qq,
                                 "message", gbk_to_utf8(msg).c_str(),
                                 "flag", gbk_to_utf8(response_flag).c_str());
        cqhttp_post_response response = post_event(json, "群添加请求");
        json_decref(json);

        if (response.json != NULL) {
            // approve or reject request if needed
            json_t *approve_json = json_object_get(response.json, "approve");
            if (json_is_boolean(approve_json)) {
                // the action is specified
                bool approve = json_boolean_value(approve_json);
                const char *remark_cstr = json_string_value(json_object_get(response.json, "remark"));
                string remark = remark_cstr ? remark_cstr : "";
                string remark_gbk = utf8_to_gbk(remark.c_str());
                CQ_setGroupAddRequestV2(ac, response_flag, sub_type, approve ? REQUEST_ALLOW : REQUEST_DENY, remark_gbk.c_str());
            }

            return release_response(response);
        }
    }
    return EVENT_IGNORE;
}
