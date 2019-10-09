#pragma once

#include "./common.h"

#include "./api.h"
#include "./app.h"
#include "./dir.h"
#include "./enums.h"
#include "./event.h"
#include "./logging.h"
#include "./menu.h"
#include "./message.h"
#include "./target.h"
#include "./types.h"

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
    using event::GroupBanEvent;
    using event::FriendAddEvent;
    using event::FriendRequestEvent;
    using event::GroupRequestEvent;

    using message::Message;
    using message::MessageSegment;
} // namespace cq
