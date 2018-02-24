#include "./cqsdk.h"

using namespace cq;
using cq::message::Message;

CQ_INITIALIZE("io.github.richardchien.coolqhttpapi");

CQ_MAIN {
    config.convert_unicode_emoji = true;

    app::on_enable = []() { };

    event::on_private_msg = [](const event::PrivateMessageEvent &e) {
        if (e.user_id != 3281334718) { return event::IGNORE; }

        log::debug(u8"消息", u8"收到消息：" + e.message + u8"～");
        api::send_private_msg(e.user_id, e.message);

        Message msg = u8"测试";
        msg.send(e.target);

        return event::IGNORE;
    };

    event::on_group_msg = [](const auto &e) {
        if (e.group_id != 615346135) { return event::IGNORE; }

        (message::Segment::at(e.user_id) + " " + e.message).send(e.target);
        message::send(e.target, message::Segment::music("qq", 422594));
        message::send(e.target, message::Segment::dice());
        return event::IGNORE;
    };
}
