#include "./cqsdk.h"

using cq::message::Message;

CQ_INITIALIZE("io.github.richardchien.coolqhttpapi");

CQ_MAIN {
    cq::config.convert_unicode_emoji = true;

    cq::app::on_enable = []() { };

    cq::event::on_private_msg = [](const cq::event::PrivateMessageEvent &e) {
        cq::log::debug(u8"消息", u8"收到消息：" + e.message);
        cq::api::send_private_msg(e.user_id, e.message);

        Message msg = u8"测试";
        msg.send(e.target);

        return cq::event::IGNORE;
    };
}
