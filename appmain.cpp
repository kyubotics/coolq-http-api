/**
 * CoolQ HTTP API core.
 */

#include "stdafx.h"

#include <string>
#include <sstream>
#include <fstream>
#include <curl/curl.h>
#include <jansson.h>
#include <event2/event.h>
#include <event2/http.h>
#include <WinSock2.h>

#include "encoding.h"
#include "misc_functions.h"
#include "request.h"
#include "ini.h"

using namespace std;

int ac = -1; // AuthCode，调用酷Q的方法时需要用到
bool enabled = false;

HANDLE httpd_thread_handle = NULL;
struct event_base *httpd_event_base = NULL;
struct evhttp *httpd_event = NULL;
struct cqhttp_config
{
    string host;
    int port;
    string post_url;
} httpd_config;

/*
* 返回应用的ApiVer、Appid，打包后将不会调用
*/
CQEVENT(const char *, AppInfo, 0)
()
{
    return CQAPPINFO;
}

/*
* 接收应用AuthCode，酷Q读取应用信息后，如果接受该应用，将会调用这个函数并传递AuthCode。
* 不要在本函数处理其他任何代码，以免发生异常情况。如需执行初始化代码请在Startup事件中执行（Type=1001）。
*/
CQEVENT(int32_t, Initialize, 4)
(int32_t AuthCode)
{
    ac = AuthCode;
    return 0;
}

static int parse_conf_handler(void *user, const char *section, const char *name, const char *value)
{
    struct cqhttp_config *config = (struct cqhttp_config *)user;
    if (string(section) == "general")
    {
        string field = name;
        if (field == "host")
            config->host = value;
        else if (field == "port")
            config->port = atoi(value);
        else if (field == "post_url")
            config->post_url = value;
        else
            return 0;
    }
    else
        return 0; /* unknown section/name, error */
    return 1;
}

/**
 * Initialize plugin, called immediately when plugin is enabled.
 */
void init()
{
    LOG_D("启用", "初始化");

    // default config
    httpd_config.host = "0.0.0.0";
    httpd_config.port = 5700;
    httpd_config.post_url = "";

    string conf_path = string(CQ_getAppDirectory(ac)) + "config.cfg";
    FILE *conf_file = NULL;
    fopen_s(&conf_file, conf_path.c_str(), "r");
    if (!conf_file)
    {
        // first init, save default config
        LOG_D("启用", "没有找到配置文件，写入默认配置");
        ofstream file(conf_path);
        file << "[general]\nhost=0.0.0.0\nport=5700\npost_url=\n";
    }
    else
    {
        // load from config file
        LOG_D("启用", "读取配置文件");
        ini_parse_file(conf_file, parse_conf_handler, &httpd_config);
        fclose(conf_file);
    }
}

/**
 * Cleanup plugin, called after all other operations when plugin is disabled.
 */
void cleanup()
{
    // do nothing currently
}

/**
 * Portal function of HTTP daemon thread.
 */
DWORD WINAPI httpd_thread_func(LPVOID lpParam)
{
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
void start_httpd()
{
    httpd_thread_handle = CreateThread(NULL,              // default security attributes
                                       0,                 // use default stack size
                                       httpd_thread_func, // thread function name
                                       NULL,              // argument to thread function
                                       0,                 // use default creation flags
                                       NULL);             // returns the thread identifier
    if (!httpd_thread_handle)
    {
        LOG_E("启用", "启动 HTTP 守护线程失败");
    }
    else
    {
        LOG_D("启用", "启动 HTTP 守护线程成功");
    }
}

/**
 * Stop HTTP daemon thread.
 */
void stop_httpd()
{
    if (httpd_thread_handle)
    {
        if (httpd_event_base)
        {
            event_base_loopbreak(httpd_event_base);
        }
        if (httpd_event)
        {
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

/*
* Event: plugin is enabled.
*/
CQEVENT(int32_t, __eventEnable, 0)
()
{
    enabled = true;
    init();
    start_httpd();
    return 0;
}

/*
* Event: plugin is disabled.
*/
CQEVENT(int32_t, __eventDisable, 0)
()
{
    enabled = false;
    stop_httpd();
    cleanup();
    return 0;
}

/*
* Type=21 私聊消息
* subType 子类型，11/来自好友 1/来自在线状态 2/来自群 3/来自讨论组
*/
CQEVENT(int32_t, __eventPrivateMsg, 24)
(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, int32_t font)
{

    //如果要回复消息，请调用酷Q方法发送，并且这里 return EVENT_BLOCK - 截断本条消息，不再继续处理  注意：应用优先级设置为"最高"(10000)时，不得使用本返回值
    //如果不回复消息，交由之后的应用/过滤器处理，这里 return EVENT_IGNORE - 忽略本条消息

    string result = "";

    CURL *curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "http://news-at.zhihu.com/api/4/news/latest");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_stringstream_callback);

        stringstream resp_stream;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp_stream);

        CURLcode res;
        res = curl_easy_perform(curl);
        if (res == CURLE_OK)
        {
            string json_string = resp_stream.str();
            LOG8_D("Net>Json", string("Got json string: ") + json_string);
            json_t *data = json_loads(json_string.c_str(), 0, NULL);
            if (data)
            {
                LOG_D("Net>Json", "Succeeded to parse json data");
                stringstream ss;
                const char *date = json_string_value(json_object_get(data, "date"));
                ss << "Date: " << date << "\n\n";
                json_t *stories_jarr = json_object_get(data, "stories");
                for (size_t i = 0; i < json_array_size(stories_jarr); i++)
                {
                    const char *title = json_string_value(json_object_get(json_array_get(stories_jarr, i), "title"));
                    ss << (i == 0 ? "" : "\n") << i << ". " << title;
                }
                json_decref(data);
                result = ss.str();
            }
            else
            {
                LOG_D("Net>Json", "Failed to load json string");
            }
        }
        else
        {
            LOG_D("Net", "Failed to get response");
        }
        curl_easy_cleanup(curl);
    }
    else
    {
        LOG_D("Net", "Failed to init cURL");
    }

    if (result != "")
    {
        CQ_sendPrivateMsg(ac, fromQQ, utf8_to_gbk(result.c_str()).c_str());
        CQ_sendPrivateMsg(ac, fromQQ, msg);
    }

    return EVENT_IGNORE;
}

/*
* Type=2 群消息
*/
CQEVENT(int32_t, __eventGroupMsg, 36)
(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font)
{

    return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}

/*
* Type=4 讨论组消息
*/
CQEVENT(int32_t, __eventDiscussMsg, 32)
(int32_t subType, int32_t sendTime, int64_t fromDiscuss, int64_t fromQQ, const char *msg, int32_t font)
{

    return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}

/*
* Type=101 群事件-管理员变动
* subType 子类型，1/被取消管理员 2/被设置管理员
*/
CQEVENT(int32_t, __eventSystem_GroupAdmin, 24)
(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t beingOperateQQ)
{

    return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}

/*
* Type=102 群事件-群成员减少
* subType 子类型，1/群员离开 2/群员被踢 3/自己(即登录号)被踢
* fromQQ 操作者QQ(仅subType为2、3时存在)
* beingOperateQQ 被操作QQ
*/
CQEVENT(int32_t, __eventSystem_GroupMemberDecrease, 32)
(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ)
{

    return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}

/*
* Type=103 群事件-群成员增加
* subType 子类型，1/管理员已同意 2/管理员邀请
* fromQQ 操作者QQ(即管理员QQ)
* beingOperateQQ 被操作QQ(即加群的QQ)
*/
CQEVENT(int32_t, __eventSystem_GroupMemberIncrease, 32)
(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ)
{

    return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}

/*
* Type=201 好友事件-好友已添加
*/
CQEVENT(int32_t, __eventFriend_Add, 16)
(int32_t subType, int32_t sendTime, int64_t fromQQ)
{

    return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}

/*
* Type=301 请求-好友添加
* msg 附言
* responseFlag 反馈标识(处理请求用)
*/
CQEVENT(int32_t, __eventRequest_AddFriend, 24)
(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, const char *responseFlag)
{

    //CQ_setFriendAddRequest(ac, responseFlag, REQUEST_ALLOW, "");

    return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}

/*
* Type=302 请求-群添加
* subType 子类型，1/他人申请入群 2/自己(即登录号)受邀入群
* msg 附言
* responseFlag 反馈标识(处理请求用)
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)
(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *msg, const char *responseFlag)
{

    //if (subType == 1) {
    //	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPADD, REQUEST_ALLOW, "");
    //} else if (subType == 2) {
    //	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPINVITE, REQUEST_ALLOW, "");
    //}

    return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}
