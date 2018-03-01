#pragma once

#include <ctime>

#include "cqhttp/core/common.h"

namespace cq {
    inline void to_json(json &j, const Sex &sex) {
        switch (sex) {
        case Sex::MALE:
            j = "male";
            break;
        case Sex::FEMALE:
            j = "female";
            break;
        default:
            j = "unknown";
            break;
        }
    }

    inline void to_json(json &j, const GroupRole &role) {
        switch (role) {
        case GroupRole::MEMBER:
            j = "member";
            break;
        case GroupRole::ADMIN:
            j = "admin";
            break;
        case GroupRole::OWNER:
            j = "owner";
            break;
        default:
            j = "unknown";
            break;
        }
    }

    inline void to_json(json &j, const User &u) {
        j = {
            {"user_id", u.user_id},
            {"nickname", u.nickname},
            {"sex", u.sex},
            {"age", u.age}
        };
    }

    inline void to_json(json &j, const Group &g) {
        j = {
            {"group_id", g.group_id},
            {"group_name", g.group_name}
        };
    }

    inline void to_json(json &j, const GroupMember &m) {
        j = {
            {"group_id", m.group_id},
            {"user_id", m.user_id},
            {"nickname", m.nickname},
            {"card", m.card},
            {"sex", m.sex},
            {"age", m.age},
            {"area", m.area},
            {"join_time", m.join_time},
            {"last_sent_time", m.last_sent_time},
            {"level", m.level},
            {"role", m.role},
            {"unfriendly", m.unfriendly},
            {"title", m.title},
            {"title_expire_time", m.title_expire_time},
            {"card_changeable", m.card_changeable}
        };
    }

    inline void to_json(json &j, const Anonymous &a) {
        j = {
            {"id", a.id},
            {"name", a.name}
        };
    }

    inline void to_json(json &j, const File &f) {
        j = {
            {"id", f.id},
            {"name", f.name},
            {"size", f.size},
            {"busid", f.busid}
        };
    }
}

namespace cq::message {
    inline void to_json(json &j, const MessageSegment &segment) {
        j = json{{"type", segment.type}, {"data", segment.data}};
    }

    inline void from_json(const json &j, MessageSegment &segment) {
        segment.type = j.at("type").get<std::string>();
        segment.data = j.at("data").get<std::map<std::string, std::string>>();
    }

    inline void to_json(json &j, const Message &message) {
        j = message.segments();
    }

    inline void from_json(const json &j, Message &message) {
        if (j.is_string()) {
            message = j.get<std::string>();
        }
        message.segments() = j.get<std::remove_reference<decltype(message.segments())>::type>();
    }

    inline void to_json(json &j, const Type &type) {
        switch (type) {
        case PRIVATE:
            j = "private";
            break;
        case GROUP:
            j = "group";
            break;
        case DISCUSS:
            j = "discuss";
            break;
        default:
            j = "unknown";
            break;
        }
    }
}

namespace cq::notice {
    inline void to_json(json &j, const Type &type) {
        switch (type) {
        case GROUP_UPLOAD:
            j = "group_upload";
            break;
        case GROUP_ADMIN:
            j = "group_admin";
            break;
        case GROUP_MEMBER_DECREASE:
            j = "group_decrease";
            break;
        case GROUP_MEMBER_INCREASE:
            j = "group_increase";
            break;
        case FRIEND_ADD:
            j = "friend_add";
            break;
        default:
            j = "unknown";
            break;
        }
    }
}

namespace cq::request {
    inline void to_json(json &j, const Type &type) {
        switch (type) {
        case FRIEND:
            j = "friend";
            break;
        case GROUP:
            j = "group";
            break;
        default:
            j = "unknown";
            break;
        }
    }
}

namespace cq::event {
    inline void to_json(json &j, const Type &type) {
        switch (type) {
        case MESSAGE:
            j = "message";
            break;
        case NOTICE:
            j = "event"; // for backward compatibility
            break;
        case REQUEST:
            j = "request";
            break;
        default:
            j = "unknown";
            break;
        }
    }

    inline void to_json(json &j, const PrivateMessageEvent &e) {
        const auto sub_type_str = [&]() {
            switch (e.sub_type) {
            case message::PRIVATE_FRIEND: return "friend";
            case message::PRIVATE_GROUP: return "group";
            case message::PRIVATE_DISCUSS: return "discuss";
            case message::PRIVATE_OTHER: return "other";
            default: return "unknown";
            }
        }();

        j = {
            {"time", time(nullptr)}, // for backward compatibility
            {"post_type", e.type},
            {"message_type", e.message_type},
            {"sub_type", sub_type_str},
            {"message_id", e.message_id},
            {"user_id", e.user_id},
            {"message", e.message},
            {"raw_message", e.raw_message},
            {"font", e.font}
        };
    }

    inline void to_json(json &j, const GroupMessageEvent &e) {
        const auto sub_type_str = [&]() {
            if (e.is_anonymous() || e.user_id == 80000000) {
                return "anonymous";
            }
            if (e.user_id == 1000000) {
                return "notice";
            }
            return "normal";
        }();

        j = {
            {"time", time(nullptr)}, // for backward compatibility
            {"post_type", e.type},
            {"message_type", e.message_type},
            {"sub_type", sub_type_str},
            {"message_id", e.message_id},
            {"group_id", e.group_id},
            {"user_id", e.user_id},
            {"anonymous", e.anonymous},
            {"anonymous_flag", e.anonymous.flag}, // for backward compatibility
            {"message", e.message},
            {"raw_message", e.raw_message},
            {"font", e.font}
        };
    }

    inline void to_json(json &j, const DiscussMessageEvent &e) {
        j = {
            {"time", time(nullptr)}, // for backward compatibility
            {"post_type", e.type},
            {"message_type", e.message_type},
            {"message_id", e.message_id},
            {"discuss_id", e.discuss_id},
            {"user_id", e.user_id},
            {"message", e.message},
            {"raw_message", e.raw_message},
            {"font", e.font}
        };
    }

    inline void to_json(json &j, const GroupUploadEvent &e) {
        j = {
            {"time", e.time},
            {"post_type", e.type},
            {"event", e.notice_type},
            {"group_id", e.group_id},
            {"user_id", e.user_id},
            {"file", e.file}
        };
    }

    inline void to_json(json &j, const GroupAdminEvent &e) {
        const auto sub_type_str = [&]() {
            switch (e.sub_type) {
            case notice::GROUP_ADMIN_UNSET: return "unset";
            case notice::GROUP_ADMIN_SET: return "set";
            default: return "unknown";
            }
        }();

        j = {
            {"time", e.time},
            {"post_type", e.type},
            {"event", e.notice_type},
            {"sub_type", sub_type_str},
            {"group_id", e.group_id},
            {"user_id", e.user_id}
        };
    }

    inline void to_json(json &j, const GroupMemberDecreaseEvent &e) {
        const auto sub_type_str = [&]() {
            switch (e.sub_type) {
            case notice::GROUP_MEMBER_DECREASE_LEAVE: return "leave";
            case notice::GROUP_MEMBER_DECREASE_KICK:
                if (e.user_id != api::get_login_user_id()) {
                    // the one been kicked out is not me
                    return "kick";
                }
            case -1: return "kick_me";
            default: return "unknown";
            }
        }();

        j = {
            {"post_type", e.type},
            {"event", e.notice_type},
            {"sub_type", sub_type_str},
            {"time", e.time},
            {"group_id", e.group_id},
            {"operator_id", e.operator_id},
            {"user_id", e.user_id}
        };
    }

    inline void to_json(json &j, const GroupMemberIncreaseEvent &e) {
        const auto sub_type_str = [&]() {
            switch (e.sub_type) {
            case notice::GROUP_MEMBER_INCREASE_APPROVE: return "approve";
            case notice::GROUP_MEMBER_INCREASE_INVITE: return "invite";
            default: return "unknown";
            }
        }();

        j = {
            {"post_type", e.type},
            {"event", e.notice_type},
            {"sub_type", sub_type_str},
            {"time", e.time},
            {"group_id", e.group_id},
            {"operator_id", e.operator_id},
            {"user_id", e.user_id}
        };
    }

    inline void to_json(json &j, const FriendAddEvent &e) {
        j = {
            {"post_type", e.type},
            {"event", e.notice_type},
            {"time", e.time},
            {"user_id", e.user_id}
        };
    }

    inline void to_json(json &j, const FriendRequestEvent &e) {
        j = {
            {"post_type", e.type},
            {"request_type", "friend"},
            {"time", e.time},
            {"user_id", e.user_id},
            {"message", e.comment}, // for backward compatibility
            {"flag", e.flag}
        };
    }

    inline void to_json(json &j, const GroupRequestEvent &e) {
        const auto sub_type_str = [&]() {
            switch (e.sub_type) {
            case request::GROUP_ADD: return "add";
            case request::GROUP_INVITE: return "invite";
            default: return "unknown";
            }
        }();

        j = {
            {"post_type", e.type},
            {"request_type", e.request_type},
            {"sub_type", sub_type_str},
            {"time", e.time},
            {"group_id", e.group_id},
            {"user_id", e.user_id},
            {"message", e.comment}, // for backward compatibility
            {"flag", e.flag}
        };
    }
}
