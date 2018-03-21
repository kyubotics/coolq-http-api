#include "./backward_compatibility.h"

using namespace std;

namespace cqhttp::plugins {
    void BackwardCompatibility::hook_event(EventContext<cq::Event> &ctx) {
        logging::debug("comp", ctx.data.dump(2));

        ctx.next();
    }
} // namespace cqhttp::plugins
