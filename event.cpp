#include "./event.h"

#include "./def.h"
#include "./exception.h"
#include "./utils/function.h"
#include "./utils/string.h"

namespace cq::event {
    std::function<void(const PrivateMessageEvent &)> on_private_msg;
    std::function<void(const GroupMessageEvent &)> on_group_msg;
    std::function<void(const DiscussMessageEvent &)> on_discuss_msg;
    std::function<void(const GroupUploadEvent &)> on_group_upload;
    std::function<void(const GroupAdminEvent &)> on_group_admin;
    std::function<void(const GroupMemberDecreaseEvent &)> on_group_member_decrease;
    std::function<void(const GroupMemberIncreaseEvent &)> on_group_member_increase;
    std::function<void(const FriendAddEvent &)> on_friend_add;
    std::function<void(const FriendRequestEvent &)> on_friend_request;
    std::function<void(const GroupRequestEvent &)> on_group_request;
} // namespace cq::event

using namespace std;
using namespace cq;
using cq::utils::call_if_valid;
using cq::utils::string_from_coolq;

/**
 * Type=21 私聊消息
 * sub_type 子类型，11/来自好友 1/来自在线状态 2/来自群 3/来自讨论组
 */
__CQ_EVENT(int32_t, cq_event_private_msg, 24)
(int32_t sub_type, int32_t msg_id, int64_t from_qq, const char *msg, int32_t font) {
    event::PrivateMessageEvent e;
    e.target = Target(from_qq);
    e.sub_type = static_cast<message::SubType>(sub_type);
    e.message_id = msg_id;
    e.raw_message = string_from_coolq(msg);
    e.message = e.raw_message;
    e.font = font;
    e.user_id = from_qq;
    call_if_valid(event::on_private_msg, e);
    return e.operation;
}

/**
 * Type=2 群消息
 */
__CQ_EVENT(int32_t, cq_event_group_msg, 36)
(int32_t sub_type, int32_t msg_id, int64_t from_group, int64_t from_qq, const char *from_anonymous, const char *msg,
 int32_t font) {
    event::GroupMessageEvent e;
    e.target = Target(from_qq, from_group, Target::GROUP);
    e.sub_type = static_cast<message::SubType>(sub_type);
    e.message_id = msg_id;
    e.raw_message = string_from_coolq(msg);
    // e.message = e.raw_message; // moved to the bottom
    e.font = font;
    e.user_id = from_qq;
    e.group_id = from_group;
    try {
        e.anonymous = ObjectHelper::from_base64<Anonymous>(string_from_coolq(from_anonymous));
    } catch (cq::exception::ParseError &) {
    }

    if (e.is_anonymous()) {
        // in CoolQ Air, there is a prefix in the message
        auto prefix = "&#91;" + e.anonymous.name + "&#93;:";
        if (boost::starts_with(e.raw_message, prefix)) {
            e.raw_message = e.raw_message.substr(prefix.length());
        }
    }

    e.message = e.raw_message;

    call_if_valid(event::on_group_msg, e);
    return e.operation;
}

/**
 * Type=4 讨论组消息
 */
__CQ_EVENT(int32_t, cq_event_discuss_msg, 32)
(int32_t sub_type, int32_t msg_id, int64_t from_discuss, int64_t from_qq, const char *msg, int32_t font) {
    event::DiscussMessageEvent e;
    e.target = Target(from_qq, from_discuss, Target::DISCUSS);
    e.sub_type = static_cast<message::SubType>(sub_type);
    e.message_id = msg_id;
    e.raw_message = string_from_coolq(msg);
    e.message = e.raw_message;
    e.font = font;
    e.user_id = from_qq;
    e.discuss_id = from_discuss;
    call_if_valid(event::on_discuss_msg, e);
    return e.operation;
}

/**
 * Type=11 群事件-文件上传
 */
__CQ_EVENT(int32_t, cq_event_group_upload, 28)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const char *file) {
    event::GroupUploadEvent e;
    e.target = Target(from_qq, from_group, Target::GROUP);
    e.time = send_time;
    e.sub_type = static_cast<notice::SubType>(sub_type);
    try {
        e.file = ObjectHelper::from_base64<File>(string_from_coolq(file));
    } catch (cq::exception::ParseError &) {
    }
    e.user_id = from_qq;
    e.group_id = from_group;
    call_if_valid(event::on_group_upload, e);
    return e.operation;
}

/**
 * Type=101 群事件-管理员变动
 * sub_type 子类型，1/被取消管理员 2/被设置管理员
 */
__CQ_EVENT(int32_t, cq_event_group_admin, 24)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t being_operate_qq) {
    event::GroupAdminEvent e;
    e.target = Target(being_operate_qq, from_group, Target::GROUP);
    e.time = send_time;
    e.sub_type = static_cast<notice::SubType>(sub_type);
    e.user_id = being_operate_qq;
    e.group_id = from_group;
    call_if_valid(event::on_group_admin, e);
    return e.operation;
}

/**
 * Type=102 群事件-群成员减少
 * sub_type 子类型，1/群员离开 2/群员被踢 3/自己(即登录号)被踢
 * from_qq 操作者QQ(仅subType为2、3时存在)
 * being_operate_qq 被操作QQ
 */
__CQ_EVENT(int32_t, cq_event_group_member_decrease, 32)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, int64_t being_operate_qq) {
    event::GroupMemberDecreaseEvent e;
    e.target = Target(being_operate_qq, from_group, Target::GROUP);
    e.time = send_time;
    e.sub_type = static_cast<notice::SubType>(sub_type);
    e.user_id = being_operate_qq;
    e.group_id = from_group;
    e.operator_id = e.sub_type == notice::GROUP_MEMBER_DECREASE_LEAVE ? being_operate_qq : from_qq;
    call_if_valid(event::on_group_member_decrease, e);
    return e.operation;
}

/**
 * Type=103 群事件-群成员增加
 * sub_type 子类型，1/管理员已同意 2/管理员邀请
 * from_qq 操作者QQ(即管理员QQ)
 * being_operate_qq 被操作QQ(即加群的QQ)
 */
__CQ_EVENT(int32_t, cq_event_group_member_increase, 32)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, int64_t being_operate_qq) {
    event::GroupMemberIncreaseEvent e;
    e.target = Target(being_operate_qq, from_group, Target::GROUP);
    e.time = send_time;
    e.sub_type = static_cast<notice::SubType>(sub_type);
    e.user_id = being_operate_qq;
    e.group_id = from_group;
    e.operator_id = from_qq;
    call_if_valid(event::on_group_member_increase, e);
    return e.operation;
}

/**
 * Type=201 好友事件-好友已添加
 */
__CQ_EVENT(int32_t, cq_event_friend_add, 16)
(int32_t sub_type, int32_t send_time, int64_t from_qq) {
    event::FriendAddEvent e;
    e.target = Target(from_qq);
    e.time = send_time;
    e.sub_type = static_cast<notice::SubType>(sub_type);
    e.user_id = from_qq;
    call_if_valid(event::on_friend_add, e);
    return e.operation;
}

/**
 * Type=301 请求-好友添加
 * msg 附言
 * response_flag 反馈标识(处理请求用)
 */
__CQ_EVENT(int32_t, cq_event_add_friend_request, 24)
(int32_t sub_type, int32_t send_time, int64_t from_qq, const char *msg, const char *response_flag) {
    event::FriendRequestEvent e;
    e.target = Target(from_qq);
    e.time = send_time;
    e.sub_type = static_cast<request::SubType>(sub_type);
    e.comment = string_from_coolq(msg);
    e.flag = string_from_coolq(response_flag);
    e.user_id = from_qq;
    call_if_valid(event::on_friend_request, e);
    return e.operation;
}

/**
 * Type=302 请求-群添加
 * sub_type 子类型，1/他人申请入群 2/自己(即登录号)受邀入群
 * msg 附言
 * response_flag 反馈标识(处理请求用)
 */
__CQ_EVENT(int32_t, cq_event_add_group_request, 32)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const char *msg, const char *response_flag) {
    event::GroupRequestEvent e;
    e.target = Target(from_qq, from_group, Target::GROUP);
    e.time = send_time;
    e.sub_type = static_cast<request::SubType>(sub_type);
    e.comment = string_from_coolq(msg);
    e.flag = string_from_coolq(response_flag);
    e.user_id = from_qq;
    e.group_id = from_group;
    call_if_valid(event::on_group_request, e);
    return e.operation;
}
