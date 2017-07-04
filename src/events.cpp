// 
// events.cpp : Implement all CoolQ event handlers.
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

#include "events.h"

#include "app.h"

#include "post_json.h"
#include "structs.h"
#include "Message.h"

using namespace std;

#define ENSURE_POST_NEEDED if (CQ->config.post_url.length() == 0) { return EVENT_IGNORE; }

static bool match_pattern(const str &msg) {
    return regex_search(msg.to_bytes(), CQ->config.pattern);
}

static int32_t handle_block_response(const json &resp_json) {
    auto block = false;
    if (resp_json.find("block") != resp_json.end()
        && resp_json["block"].is_boolean()
        && resp_json["block"]) {
        block = true;
    }
    return block ? EVENT_BLOCK : EVENT_IGNORE;
}

int32_t event_private_msg(int32_t sub_type, int32_t send_time, int64_t from_qq, const str &msg, int32_t font) {
    ENSURE_POST_NEEDED;

    if (match_pattern(msg)) {
        json json = {
            {"post_type", "message"},
            {"message_type", "private"},
            {"sub_type", [&]() {
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
                }()},
            {"time", send_time},
            {"user_id", from_qq},
            {"message", Message(msg).process_incoming()}
        };
        auto resp = post_json(json);

        auto resp_json = resp.json();
        if (resp.json().is_object()) {
            if (resp_json.find("reply") != resp_json.end()) {
                CQ->send_private_msg(from_qq, Message(resp_json["reply"]).process_outcoming());
            }
            return handle_block_response(resp_json);
        }
    }
    return EVENT_IGNORE;
}

int32_t event_group_msg(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const str &from_anonymous, const str &msg, int32_t font) {
    ENSURE_POST_NEEDED;

    if (match_pattern(msg)) {
        str anonymous = "";
        if (from_qq == 8000000 || from_anonymous) {
            auto anonymous_bin = base64_decode(from_anonymous);
            anonymous = Anonymous::from_bytes(anonymous_bin).name;
        }
        auto is_anonymous = from_anonymous.length() > 0;
        json json = {
            {"post_type", "message"},
            {"message_type", "group"},
            {"time", send_time},
            {"group_id", from_group},
            {"user_id", from_qq},
            {"anonymous", anonymous},
            {"anonymous_flag", from_anonymous},
            {"message", Message(msg).process_incoming()}
        };
        auto resp = post_json(json);

        auto resp_json = resp.json();
        if (resp_json.is_object()) {
            if (resp_json.find("reply") != resp_json.end()) {
                auto reply = Message(resp_json["reply"]).process_outcoming();

                if (reply) {
                    // check if should at sender
                    auto at_sender = true;
                    if (resp_json.find("at_sender") != resp_json.end()
                        && resp_json["at_sender"].is_boolean()
                        && resp_json["at_sender"] == false) {
                        at_sender = false;
                    }
                    if (at_sender) {
                        if (is_anonymous) {
                            reply = "@" + anonymous + " " + reply;
                        } else {
                            reply = "[CQ:at,qq=" + str(from_qq) + "] " + reply;
                        }
                    }

                    // send reply
                    CQ->send_group_msg(from_group, reply);
                }
            }

            // kick sender if needed
            if (resp_json.find("kick") != resp_json.end()
                && resp_json["kick"].is_boolean()
                && resp_json["kick"]
                && !is_anonymous) {
                CQ->set_group_kick(from_group, from_qq, false);
            }

            // ban sender if needed
            if (resp_json.find("ban") != resp_json.end()
                && resp_json["ban"].is_boolean()
                && resp_json["ban"]) {
                if (is_anonymous) {
                    CQ->set_group_anonymous_ban(from_group, from_anonymous, 30 * 60);
                } else {
                    CQ->set_group_ban(from_group, from_qq, 30 * 60);
                }
            }

            return handle_block_response(resp_json);
        }
    }
    return EVENT_IGNORE;
}

int32_t event_discuss_msg(int32_t sub_type, int32_t send_time, int64_t from_discuss, int64_t from_qq, const str &msg, int32_t font) {
    ENSURE_POST_NEEDED;

    if (match_pattern(msg)) {
        json json = {
            {"post_type", "message"},
            {"message_type", "discuss"},
            {"time", send_time},
            {"discuss_id", from_discuss},
            {"user_id", from_qq},
            {"message", Message(msg).process_incoming()}
        };
        auto resp = post_json(json);

        auto resp_json = resp.json();
        if (resp_json.is_object()) {
            if (resp_json.find("reply") != resp_json.end()) {
                auto reply = Message(resp_json["reply"]).process_outcoming();
                if (reply) {
                    // check if should at sender
                    auto at_sender = true;
                    if (resp_json.find("at_sender") != resp_json.end()
                        && resp_json["at_sender"].is_boolean()
                        && resp_json["at_sender"] == false) {
                        at_sender = false;
                    }
                    if (at_sender) {
                        reply = "[CQ:at,qq=" + str(from_qq) + "] " + reply;
                    }

                    // send reply
                    CQ->send_discuss_msg(from_discuss, reply);
                }
            }

            return handle_block_response(resp_json);
        }
    }
    return EVENT_IGNORE;
}

int32_t event_group_upload(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const str &file) {
    ENSURE_POST_NEEDED;

    auto file_bin = base64_decode(file);
    json json = {
        {"post_type", "event"},
        {"event", "group_upload"},
        {"time", send_time},
        {"group_id", from_group},
        {"user_id", from_qq},
        {"file", file_bin.size() >= GroupFile::MIN_SIZE ? GroupFile::from_bytes(file_bin).json() : nullptr}
    };
    auto resp = post_json(json);

    if (resp.json().is_object()) {
        return handle_block_response(resp.json());
    }

    return EVENT_IGNORE;
}

int32_t event_group_admin(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t being_operate_qq) {
    ENSURE_POST_NEEDED;

    json json = {
        {"post_type", "event"},
        {"event", "group_admin"},
        {"sub_type", [&]() {
                switch (sub_type) {
                case 1:
                    return "unset";
                case 2:
                    return "set";
                default:
                    return "unknown";
                }
            }()},
        {"time", send_time},
        {"group_id", from_group},
        {"user_id", being_operate_qq}
    };
    auto resp = post_json(json);

    if (resp.json().is_object()) {
        return handle_block_response(resp.json());
    }

    return EVENT_IGNORE;
}

int32_t event_group_member_decrease(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, int64_t being_operate_qq) {
    ENSURE_POST_NEEDED;

    json json = {
        {"post_type", "event"},
        {"event", "group_decrease"},
        {"sub_type", [&]() {
                switch (sub_type) {
                case 1:
                    return "leave";
                case 2:
                    if (being_operate_qq != CQ->get_login_qq()) {
                        // the one been kicked out is not me
                        return "kick";
                    }
                case 3:
                    return "kick_me";
                default:
                    return "unknown";
                }
            }()},
        {"time", send_time},
        {"group_id", from_group},
        {"operator_id", sub_type == 1 ? being_operate_qq /* leave by him/herself */ : from_qq},
        {"user_id", being_operate_qq}
    };
    auto resp = post_json(json);

    if (resp.json().is_object()) {
        return handle_block_response(resp.json());
    }

    return EVENT_IGNORE;
}

int32_t event_group_member_increase(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, int64_t being_operate_qq) {
    ENSURE_POST_NEEDED;

    json json = {
        {"post_type", "event"},
        {"event", "group_increase"},
        {"sub_type", [&]() {
                switch (sub_type) {
                case 1:
                    return "approve";
                case 2:
                    return "invite";
                default:
                    return "unknown";
                }
            }()},
        {"time", send_time},
        {"group_id", from_group},
        {"operator_id", from_qq},
        {"user_id", being_operate_qq}
    };
    auto resp = post_json(json);

    if (resp.json().is_object()) {
        return handle_block_response(resp.json());
    }

    return EVENT_IGNORE;
}

int32_t event_friend_add(int32_t sub_type, int32_t send_time, int64_t from_qq) {
    ENSURE_POST_NEEDED;

    json json = {
        {"post_type", "event"},
        {"event", "friend_add"},
        {"time", send_time},
        {"user_id", from_qq}
    };
    auto resp = post_json(json);

    if (resp.json().is_object()) {
        return handle_block_response(resp.json());
    }

    return EVENT_IGNORE;
}

int32_t event_add_friend_request(int32_t sub_type, int32_t send_time, int64_t from_qq, const str &msg, const str &response_flag) {
    ENSURE_POST_NEEDED;

    json json = {
        {"post_type", "request"},
        {"request_type", "friend"},
        {"time", send_time},
        {"user_id", from_qq},
        {"message", msg},
        {"flag", response_flag}
    };
    auto resp = post_json(json);

    auto resp_json = resp.json();
    if (resp_json.is_object()) {
        // approve or reject request if needed
        if (resp_json.find("approve") != resp_json.end()
            && resp_json["approve"].is_boolean()) {
            // the action is specified
            auto approve = resp_json["approve"].get<bool>();
            str remark;
            if (resp_json.find("remark") != resp_json.end()
                && resp_json["remark"].is_string()) {
                remark = resp_json["remark"].get<str>();
            }
            CQ->set_friend_add_request(response_flag, approve ? REQUEST_ALLOW : REQUEST_DENY, remark);
        }

        return handle_block_response(resp_json);
    }

    return EVENT_IGNORE;
}

int32_t event_add_group_request(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const str &msg, const str &response_flag) {
    ENSURE_POST_NEEDED;

    json json = {
        {"post_type", "request"},
        {"request_type", "group"},
        {"sub_type", [&]() {
                switch (sub_type) {
                case 1:
                    return "add";
                case 2:
                    return "invite";
                default:
                    return "unknown";
                }
            }()},
        {"time", send_time},
        {"group_id", from_group},
        {"user_id", from_qq},
        {"message", msg},
        {"flag", response_flag}
    };
    auto resp = post_json(json);

    auto resp_json = resp.json();
    if (resp_json.is_object()) {
        // approve or reject request if needed
        if (resp_json.find("approve") != resp_json.end()
            && resp_json["approve"].is_boolean()) {
            // the action is specified
            auto approve = resp_json["approve"].get<bool>();
            str reason;
            if (resp_json.find("reason") != resp_json.end()
                && resp_json["reason"].is_string()) {
                reason = resp_json["reason"].get<str>();
            }
            CQ->set_group_add_request(response_flag, sub_type, approve ? REQUEST_ALLOW : REQUEST_DENY, reason);
        }

        return handle_block_response(resp_json);
    }

    return EVENT_IGNORE;
}
