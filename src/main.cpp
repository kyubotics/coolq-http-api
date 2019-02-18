#include "cqhttp/core/core.h"

#include "cqhttp/plugins/config_loader/default_config_generator.h"
#include "cqhttp/plugins/config_loader/ini_config_loader.h"
#include "cqhttp/plugins/config_loader/json_config_loader.h"

#include "cqhttp/plugins/heartbeat_generator/heartbeat_generator.h"
#include "cqhttp/plugins/loggers/loggers.h"
#include "cqhttp/plugins/worker_pool_resizer/worker_pool_resizer.h"

#include "cqhttp/plugins/event_data_patcher/event_data_patcher.h"
#include "cqhttp/plugins/message_enhancer/message_enhancer.h"

#include "cqhttp/plugins/async_actions/async_actions.h"
#include "cqhttp/plugins/experimental_actions/experimental_actions.h"
#include "cqhttp/plugins/rate_limited_actions/rate_limited_actions.h"
#include "cqhttp/plugins/restarter/restarter.h"
#include "cqhttp/plugins/updater/updater.h"

#include "cqhttp/plugins/backward_compatibility/backward_compatibility.h"
#include "cqhttp/plugins/event_filter/event_filter.h"
#include "cqhttp/plugins/extension_loader/extension_loader.h"
#include "cqhttp/plugins/post_message_formatter/post_message_formatter.h"
#include "cqhttp/plugins/web/http.h"
#include "cqhttp/plugins/web/websocket.h"
#include "cqhttp/plugins/web/websocket_reverse.h"

using namespace cqhttp;

CQ_INITIALIZE(CQHTTP_ID);

CQ_MAIN {
    init();

    // load configurations
    use(plugins::ini_config_loader);
    use(plugins::json_config_loader);
    use(plugins::default_config_generator);

    // config global things
    use(plugins::loggers);
    use(plugins::worker_pool_resizer);
    use(plugins::heartbeat_generator);

    // extend the Context object
    use(plugins::event_data_patcher);
    use(plugins::message_enhancer);

    // extend actions
    use(plugins::restarter);
    use(plugins::updater);
    use(plugins::rate_limited_actions);
    use(plugins::async_actions);
    use(plugins::experimental_actions);

    // handle api and event, must in order and at the end
    use(plugins::event_filter);
    use(plugins::extension_loader);
    use(plugins::backward_compatibility);
    use(plugins::post_message_formatter);
    use(plugins::http);
    use(plugins::websocket);
    use(plugins::websocket_reverse);
}
