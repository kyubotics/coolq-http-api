#include "cqsdk/cqsdk.h"

#include "cqhttp/cqhttp.h"

#include "cqhttp/plugins/config_loader/ini_config_loader.h"
#include "cqhttp/plugins/config_loader/json_config_loader.h"

#include "cqhttp/plugins/async_actions/async_actions.h"
#include "cqhttp/plugins/event_data_patcher/event_data_patcher.h"
#include "cqhttp/plugins/experimental_actions/experimental_actions.h"
#include "cqhttp/plugins/message_enhancer/message_enhancer.h"
#include "cqhttp/plugins/restarter/restarter.h"

#include "cqhttp/plugins/backward_compatibility/backward_compatibility.h"
#include "cqhttp/plugins/event_filter/event_filter.h"
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

    // extend the Context object
    use(plugins::event_data_patcher);
    use(plugins::message_enhancer);
    use(plugins::restarter);
    use(plugins::async_actions);
    use(plugins::experimental_actions);

    // handle api and event, must in order and at the end
    use(plugins::event_filter);
    use(plugins::post_message_formatter);
    use(plugins::backward_compatibility); // this will affect the data passed to plugins::event_filter
    use(plugins::http);
    use(plugins::websocket);
    use(plugins::websocket_reverse);
}
