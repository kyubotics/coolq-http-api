#include "./cqsdk.h"

CQ_INITIALIZE("io.github.richardchien.coolqhttpapi")

CQ_MAIN {
    cq::config.convert_unicode_emoji = true;

    cq::app::on_enable = []() { };

    cq::event::on_private_msg = [](const cq::event::PrivateMessageEvent &e) {
        cq::log::debug(u8"ÏûÏ¢", e.message);
        cq::api::send_private_msg(e.user_id, e.message);
        return cq::event::IGNORE;
    };
}
