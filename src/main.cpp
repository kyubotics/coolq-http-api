#include "cqsdk/cqsdk.h"
#include "cqhttp/cqhttp.h"
#include "cqhttp/plugins/http/http.h"
#include "cqhttp/plugins/filter/filter.h"

using namespace cqhttp;

CQ_INITIALIZE("io.github.richardchien.coolqhttpapi");

CQ_MAIN {
    auto app = std::make_shared<Application>();
    app->use(plugins::filter);
    app->use(plugins::http);
    apply(app);
}
