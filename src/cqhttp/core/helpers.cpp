#include "./helpers.h"

#include "cqhttp/core/core.h"

using namespace std;

namespace cqhttp::helpers {
    string get_update_source_url(string rel_path) {
        auto url = app.config().get_string("update_source", "global");

        if (url == "global" || url == "github") {
            url = "https://raw.githubusercontent.com/richardchien/coolq-http-api-release/master/";
        } else if (url == "china" || url == "coding") {
            url = "https://coding.net/u/richardchien/p/coolq-http-api-release/git/raw/master/";
        } else if (url == "gitee") {
            url = "https://gitee.com/richardchien/coolq-http-api-release/raw/master/";
        }

        if (!boost::ends_with(url, "/")) {
            url = url + "/";
        }

        return url + rel_path;
    }
} // namespace cqhttp::helpers
