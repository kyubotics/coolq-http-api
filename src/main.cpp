#include "cqsdk/cqsdk.h"

#include "cqhttp/cqhttp.h"
#include "cqhttp/plugins/filter/filter.h"
#include "cqhttp/plugins/http/http.h"

using namespace cqhttp;

CQ_INITIALIZE(CQHTTP_ID);

CQ_MAIN {
    init();
    use(plugins::filter);
    use(plugins::http);
}
