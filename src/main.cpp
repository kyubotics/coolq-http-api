#include "cqsdk/cqsdk.h"

#include "cqhttp/cqhttp.h"

#include "cqhttp/plugins/config_loader/ini_config_loader.h"
#include "cqhttp/plugins/config_loader/json_config_loader.h"

#include "cqhttp/plugins/event_data_patcher/event_data_patcher.h"
#include "cqhttp/plugins/restarter/restarter.h"
#include "cqhttp/plugins/experimental_actions/experimental_actions.h"

#include "cqhttp/plugins/backward_compatibility/backward_compatibility.h"
#include "cqhttp/plugins/filter/filter.h"
#include "cqhttp/plugins/post_message_formatter/post_message_formatter.h"
#include "cqhttp/plugins/web/http.h"
#include "cqhttp/plugins/web/websocket.h"
#include "cqhttp/plugins/web/websocket_reverse.h"

using namespace cqhttp;

CQ_INITIALIZE(CQHTTP_ID);

CQ_MAIN {
    init();

    use(plugins::ini_config_loader);
    use(plugins::json_config_loader);

    use(plugins::event_data_patcher);
    use(plugins::restarter);
    use(plugins::experimental_actions);

    use(plugins::filter);
    use(plugins::post_message_formatter);
    use(plugins::backward_compatibility);
    use(plugins::http);
    use(plugins::websocket);
    use(plugins::websocket_reverse);
}
