#include "./http.h"

#include "cqhttp/utils/http.h"

namespace cqhttp::plugins {
    void Http::hook_after_event(EventContext<cq::Event> &ctx) {
        logging::debug("http", ctx.data.dump(2));
        ctx.next();
    }
} // namespace cqhttp::plugins
