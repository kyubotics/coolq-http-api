#include "cqsdk/cqsdk.h"

#include "cqhttp/cqhttp.h"

#include "cqhttp/plugins/config_loader/ini_config_loader.h"
#include "cqhttp/plugins/config_loader/json_config_loader.h"

#include "cqhttp/plugins/filter/filter.h"

#include "cqhttp/plugins/backward_compatibility/backward_compatibility.h"
#include "cqhttp/plugins/http/http.h"

using namespace cqhttp;

CQ_INITIALIZE(CQHTTP_ID);

CQ_MAIN {
    init();
    use(plugins::ini_config_loader);
    use(plugins::json_config_loader);
    use(plugins::filter);

    use(plugins::backward_compatibility);
    use(plugins::http);
}
