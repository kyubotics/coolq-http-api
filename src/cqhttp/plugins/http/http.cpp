#include "./http.h"

#include "cqhttp/utils/http.h"

namespace cqhttp::plugins {
    void Http::hook_message_event(EventContext<cq::MessageEvent> &ctx) {
        if (ctx.event.target.user_id.value_or(0) != 3281334718) {
            ctx.next();
            return;
        }

        const auto link = ctx.event.message.extract_plain_text();
        logging::debug("http", link);

        const auto path = cq::dir::app_tmp() + "temp";
        utils::http::download_file(link, path, true);

        ctx.next();
    }
} // namespace cqhttp::plugins
