#pragma once

#include "./enums.h"
#include "./types.h"
#include "./target.h"
#include "./app.h"
#include "./event.h"
#include "./api.h"
#include "./message.h"
#include "./logging.h"
#include "./dir.h"

namespace cq {
    using event::Event;
    using event::MessageEvent;
    using event::NoticeEvent;
    using event::RequestEvent;
    using event::PrivateMessageEvent;
    using event::GroupMessageEvent;
    using event::DiscussMessageEvent;
    using event::GroupUploadEvent;
    using event::GroupAdminEvent;
    using event::GroupMemberDecreaseEvent;
    using event::GroupMemberIncreaseEvent;
    using event::FriendAddEvent;
    using event::FriendRequestEvent;
    using event::GroupRequestEvent;

    using message::Message;
    using message::MessageSegment;
}
