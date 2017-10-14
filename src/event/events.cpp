#include "events.h"

#include "app.h"

#include "utils/rest_client.h"
#include "message/message_class.h"
#include "structs.h"

using namespace std;

#define ENSURE_POST_NEEDED if (config.post_url.empty()) { return CQEVENT_IGNORE; }

static pplx::task<json> post(json &json_body) {
    static const auto TAG = u8"上报";

    http_request request(http::methods::POST);
    request.headers().add(L"User-Agent", CQAPP_USER_AGENT);
    request.headers().add(L"Content-Type", L"application/json; charset=UTF-8");
    request.set_body(json_body.dump());
    return http_client(s2ws(config.post_url))
            .request(request)
            .then([](pplx::task<http_response> task) {
                auto next_task = pplx::task_from_result<string>("");
                try {
                    auto resp = task.get();
                    auto succeeded = false;
                    if (resp.status_code() < 300 && resp.status_code() >= 200) {
                        succeeded = true;
                        next_task = resp.extract_utf8string(true);
                    }
                    Log::d(TAG,
                           u8"上报数据到 " + config.post_url + (succeeded ? u8" 成功" : u8" 失败")
                           + u8"，状态码：" + to_string(resp.status_code()));
                } catch (http_exception &) {
                    // failed to request
                    Log::d(TAG, u8"上报地址 " + config.post_url + u8" 无法访问");
                }
                return next_task;
            })
            .then([](string &body) {
                if (!body.empty()) {
                    Log::d(TAG, u8"收到响应 " + body);
                    return pplx::task_from_result(json::parse(body)); // may throw invalid_argument due to invalid json
                }
                return pplx::task_from_result(json());
            });
}

static int32_t handle_response(pplx::task<json> task, const function<void(const json &)> func = nullptr) {
    static const auto TAG = u8"上报";

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
        Log::d(TAG, u8"上报响应不是有效的 JSON，已忽略");
    }
    return CQEVENT_IGNORE;
}

static void do_quick_reply(const json &resp_payload, const function<void(const string &)> send_func) {
    static const auto TAG = u8"快速回复";

    if (const auto reply_it = resp_payload.find("reply"); reply_it != resp_payload.end() && !reply_it->is_null()) {
        auto reply = *reply_it;
        if (reply.is_string()) {
            if (const auto auto_escape_it = resp_payload.find("auto_escape");
                auto_escape_it != resp_payload.end() && auto_escape_it->is_boolean() && *auto_escape_it) {
                reply = Message::escape(reply.get<string>());
            }
        }
        if (send_func) {
            const auto reply_str = Message(reply).process_outward();
            send_func(reply_str);
            Log::d(TAG, u8"已快速回复：" + reply_str);
        }
    }
}

/**
 * Call "func" if the "key" exists in "resp_payload" and the type matches the template "Type".
 */
template <typename Type>
static void handle_key(const json &resp_payload, const string &key, function<void(const Type &)> func) {
    if (const auto it = resp_payload.find(key); it != resp_payload.end()) {
        try {
            if (func) {
                func(it->get<Type>());
            }
        } catch (domain_error &) {
            // type doesn't match
        }
    }
}

int32_t event_private_msg(int32_t sub_type, int32_t send_time, int64_t from_qq, const string &msg, int32_t font) {
    ENSURE_POST_NEEDED;

    const auto sub_type_str = [&sub_type]() {
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

    return handle_response(post(payload), [&](auto &resp_payload) {
        do_quick_reply(resp_payload, [&](auto &reply) {
            sdk->send_private_msg(from_qq, reply);
        });
    });
}

int32_t event_group_msg(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq,
                        const string &from_anonymous, const string &msg, int32_t font) {
    ENSURE_POST_NEEDED;

    string anonymous;
    if (from_qq == 8000000 || !from_anonymous.empty()) {
        const auto anonymous_bin = base64_decode(from_anonymous);
        anonymous = Anonymous::from_bytes(anonymous_bin).name;
    }
    auto is_anonymous = !anonymous.empty();

    json payload = {
        {"post_type", "message"},
        {"message_type", "group"},
        {"time", send_time},
        {"group_id", from_group},
        {"user_id", from_qq},
        {"anonymous", anonymous},
        {"anonymous_flag", from_anonymous},
        {"message", Message(msg).process_inward()},
        {"font", font}
    };

    return handle_response(post(payload), [&](auto &resp_payload) {
        do_quick_reply(resp_payload, [&](auto &reply) {
            auto prefix = "[CQ:at,qq=" + to_string(from_qq) + "] "; // at sender by default
            handle_key<bool>(resp_payload, "at_sender", [&](auto at_sender) {
                if (!at_sender) {
                    prefix = "";
                }
            });
            sdk->send_group_msg(from_group, prefix + reply);
        });

        handle_key<bool>(resp_payload, "kick", [&](auto kick) {
            if (!is_anonymous) {
                sdk->set_group_kick(from_group, from_qq, false);
            }
        });

        handle_key<bool>(resp_payload, "ban", [&](auto ban) {
            const auto ban_duration = 30 * 60; // 30 minutes by default
            if (is_anonymous) {
                sdk->set_group_anonymous_ban(from_group, from_anonymous, ban_duration);
            } else {
                sdk->set_group_ban(from_group, from_qq, ban_duration);
            }
        });
    });
}

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

    return handle_response(post(payload), [&](auto &resp_payload) {
        do_quick_reply(resp_payload, [&](auto &reply) {
            auto prefix = "[CQ:at,qq=" + to_string(from_qq) + "] "; // at sender by default
            handle_key<bool>(resp_payload, "at_sender", [&prefix](auto at_sender) {
                if (!at_sender) {
                    prefix = "";
                }
            });
            sdk->send_discuss_msg(from_discuss, prefix + reply);
        });
    });
}

int32_t event_group_upload(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq,
                           const string &file) {
    ENSURE_POST_NEEDED;

    const auto file_bin = base64_decode(file);
    json payload = {
        {"post_type", "event"},
        {"event", "group_upload"},
        {"time", send_time},
        {"group_id", from_group},
        {"user_id", from_qq},
        {"file", file_bin.size() >= GroupFile::MIN_SIZE ? GroupFile::from_bytes(file_bin).json() : nullptr}
    };

    return handle_response(post(payload));
}

int32_t event_group_admin(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t being_operate_qq) {
    ENSURE_POST_NEEDED;

    const auto sub_type_str = [&sub_type]() {
        switch (sub_type) {
        case 1:
            return "unset";
        case 2:
            return "set";
        default:
            return "unknown";
        }
    }();
    json payload = {
        {"post_type", "event"},
        {"event", "group_admin"},
        {"sub_type", sub_type_str},
        {"time", send_time},
        {"group_id", from_group},
        {"user_id", being_operate_qq}
    };

    return handle_response(post(payload));
}

int32_t event_group_member_decrease(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq,
                                    int64_t being_operate_qq) {
    ENSURE_POST_NEEDED;

    const auto sub_type_str = [&sub_type, &being_operate_qq]() {
        switch (sub_type) {
        case 1:
            return "leave";
        case 2:
            if (being_operate_qq != sdk->get_login_qq()) {
                // the one been kicked out is not me
                return "kick";
            }
        case 3:
            return "kick_me";
        default:
            return "unknown";
        }
    }();
    json payload = {
        {"post_type", "event"},
        {"event", "group_decrease"},
        {"sub_type", sub_type_str},
        {"time", send_time},
        {"group_id", from_group},
        {"operator_id", sub_type == 1 ? being_operate_qq /* leave by him/herself */ : from_qq},
        {"user_id", being_operate_qq}
    };

    return handle_response(post(payload));
}

int32_t event_group_member_increase(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq,
                                    int64_t being_operate_qq) {
    ENSURE_POST_NEEDED;

    const auto sub_type_str = [&sub_type]() {
        switch (sub_type) {
        case 1:
            return "approve";
        case 2:
            return "invite";
        default:
            return "unknown";
        }
    }();
    json payload = {
        {"post_type", "event"},
        {"event", "group_increase"},
        {"sub_type", sub_type_str},
        {"time", send_time},
        {"group_id", from_group},
        {"operator_id", from_qq},
        {"user_id", being_operate_qq}
    };

    return handle_response(post(payload));
}

int32_t event_friend_add(int32_t sub_type, int32_t send_time, int64_t from_qq) {
    ENSURE_POST_NEEDED;

    json payload = {
        {"post_type", "event"},
        {"event", "friend_add"},
        {"time", send_time},
        {"user_id", from_qq}
    };

    return handle_response(post(payload));
}

int32_t event_add_friend_request(int32_t sub_type, int32_t send_time, int64_t from_qq, const string &msg,
                                 const string &response_flag) {
    ENSURE_POST_NEEDED;

    json payload = {
        {"post_type", "request"},
        {"request_type", "friend"},
        {"time", send_time},
        {"user_id", from_qq},
        {"message", msg},
        {"flag", response_flag}
    };

    return handle_response(post(payload), [&](auto &resp_payload) {
        handle_key<bool>(resp_payload, "approve", [&](auto approve) {
            string remark;
            handle_key<string>(resp_payload, "remark", [&remark](auto &r) {
                remark = r;
            });
            sdk->set_friend_add_request(response_flag, approve ? CQREQUEST_ALLOW : CQREQUEST_DENY, remark);
        });
    });
}

int32_t event_add_group_request(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq,
                                const string &msg, const string &response_flag) {
    ENSURE_POST_NEEDED;

    const auto sub_type_str = [&sub_type]() {
        switch (sub_type) {
        case 1:
            return "add";
        case 2:
            return "invite";
        default:
            return "unknown";
        }
    }();
    json payload = {
        {"post_type", "request"},
        {"request_type", "group"},
        {"sub_type", sub_type_str},
        {"time", send_time},
        {"group_id", from_group},
        {"user_id", from_qq},
        {"message", msg},
        {"flag", response_flag}
    };

    return handle_response(post(payload), [&](auto &resp_payload) {
        handle_key<bool>(resp_payload, "approve", [&](auto approve) {
            string reason;
            handle_key<string>(resp_payload, "reason", [&reason](auto &r) {
                reason = r;
            });
            sdk->set_group_add_request(response_flag, sub_type, approve ? CQREQUEST_ALLOW : CQREQUEST_DENY, reason);
        });
    });
}
