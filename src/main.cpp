#include "cqsdk/cqsdk.h"
#include "cqhttp/cqhttp.h"
#include "cqhttp/plugins/http/http.h"
#include "cqhttp/plugins/filter/filter.h"

CQ_INITIALIZE("io.github.richardchien.coolqhttpapi");

CQ_MAIN {
    cqhttp::init();
    auto app = std::make_shared<cqhttp::Application>();
    app->use(cqhttp::plugins::filter);
    app->use(cqhttp::plugins::http);
    cqhttp::apply(app);
}
