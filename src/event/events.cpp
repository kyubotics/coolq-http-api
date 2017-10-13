#include "events.h"

#include "app.h"

#include "utils/rest_client.h"
#include "message/message_class.h"

using namespace std;

#define ENSURE_POST_NEEDED if (sdk->config.post_url.empty()) { return CQEVENT_IGNORE; }

static pplx::task<json> post(json &json_body) {
    static const auto TAG = u8"上报";

    http_client client(utility::conversions::to_string_t(sdk->config.post_url));
    return client.request(http::methods::POST, "", json_body.dump(), "application/json").then([](http_response resp) {
        auto succeeded = false;
        auto task = pplx::task_from_result<string>("");
        if (resp.status_code() < 300 && resp.status_code() >= 200) {
            succeeded = true;
            task = resp.extract_utf8string(true);
        }
        Log::d(TAG, u8"上报数据到 " + sdk->config.post_url + (succeeded ? u8" 成功" : u8" 失败"));
        return task;
    }).then([](string &body) {
        return pplx::task_from_result(json::parse(body)); // may throw exceptions due to invalid json
    });
}

static int32_t handle_response(pplx::task<json> task, const function<void(const json &)> func = nullptr) {
    try {
        if (const auto resp_payload = task.get(); resp_payload.is_object()) {
            // custom handler
            if (func) {
                func(resp_payload);
            }

            if (const auto block_it = resp_payload.find("block");
                block_it != resp_payload.end() && block_it->is_boolean() && *block_it) {
                return CQEVENT_BLOCK;
            }
        }
    } catch (invalid_argument &) {
        // failed to parse json
    }
    return CQEVENT_IGNORE;
}

static void do_quick_reply(const json &resp_payload, const function<void(const string &)> send_func) {
    if (const auto reply_it = resp_payload.find("reply");
        reply_it != resp_payload.end() && !reply_it->is_null()) {
        auto reply = *reply_it;
        if (reply.is_string()) {
            if (const auto auto_escape_it = resp_payload.find("auto_escape");
                auto_escape_it != resp_payload.end() && auto_escape_it->is_boolean() && *auto_escape_it) {
                reply = Message::escape(reply.get<string>());
            }
        }
        send_func(Message(reply).process_outward());
    }
}

template <typename Type>
static void handle_key(const json &resp_payload, const string &key, function<void(const Type &)> func) {
    if (const auto it = resp_payload.find(key); it != resp_payload.end()) {
        try {
            func(it->get<Type>());
        } catch (domain_error &) {
            // type invalid
        }
    }
}

int32_t event_private_msg(int32_t sub_type, int32_t send_time, int64_t from_qq, const string &msg, int32_t font) {
    ENSURE_POST_NEEDED;

    auto sub_type_str = [&sub_type]() {
        switch (sub_type) {
        case 11:
            return "friend";
        case 1:
            return "other";
        case 2:
            return "group";
        case 3:
            return "discuss";
        default:
            return "unknown";
        }
    }();
    json payload = {
        {"post_type", "message"},
        {"message_type", "private"},
        {"sub_type", sub_type_str},
        {"time", send_time},
        {"user_id", from_qq},
        {"message", Message(msg).process_inward()},
        {"font", font}
    };

    return handle_response(post(payload), [&](auto resp_payload) {
        do_quick_reply(resp_payload, [&](auto reply) {
            sdk->send_private_msg(from_qq, reply);
        });
    });
}

//int32_t event_group_msg(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq,
//                        const string &from_anonymous, const string &msg, int32_t font) {
//    
//}

int32_t event_discuss_msg(int32_t sub_type, int32_t send_time, int64_t from_discuss, int64_t from_qq, const string &msg,
                          int32_t font) {
    ENSURE_POST_NEEDED;

    json payload = {
        {"post_type", "message"},
        {"message_type", "discuss"},
        {"time", send_time},
        {"discuss_id", from_discuss},
        {"user_id", from_qq},
        {"message", Message(msg).process_inward()},
        {"font", font}
    };

    return handle_response(post(payload), [&](auto resp_payload) {
        do_quick_reply(resp_payload, [&](auto reply) {
            auto prefix = "[CQ:at,qq=" + to_string(from_qq) + "] "; // at sender by default
            handle_key<bool>(resp_payload, "at_sender", [&](auto at_sender) {
                if (!at_sender) {
                    prefix = "";
                }
            });
            sdk->send_discuss_msg(from_discuss, prefix + reply);
        });
    });
}
