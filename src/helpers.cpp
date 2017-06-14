#include "helpers.h"

#include "app.h"

#include <cctype>

using namespace std;

bool isnumber(const string &s) {
    for (auto ch : s) {
        if (!isdigit(ch)) {
            return false;
        }
    }
    return !s.empty();
}

void string_replace(string &str, const string &search, const string &replace) {
    if (search.empty())
        return;
    string ws_ret;
    ws_ret.reserve(str.length());
    size_t start_pos = 0, pos;
    while ((pos = str.find(search, start_pos)) != string::npos) {
        ws_ret += str.substr(start_pos, pos - start_pos);
        ws_ret += replace;
        pos += search.length();
        start_pos = pos;
    }
    ws_ret += str.substr(start_pos);
    str.swap(ws_ret); // faster than str = wsRet;
}

str get_coolq_root() {
    static str root;
    if (!root) {
        auto app_dir = CQ->get_app_directory();
        auto suffix = str("app\\" CQ_APP_ID "\\");
        root = app_dir[slice(0, app_dir.length() - suffix.length())];
    }
    return root;
}
