#include "helpers.h"

#include "app.h"

#include <cctype>
#include <sstream>

using namespace std;

size_t curl_write_file_callback(char *buf, size_t size, size_t nmemb, FILE *fp) {
    size_t written_size = 0;
    if (fp)
        written_size = fwrite(buf, size, nmemb, fp);
    return written_size;
}

bool isnumber(const string &s) {
    string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it))
        ++it;
    return !s.empty() && it == s.end();
}

string itos(int64_t i) {
    stringstream ss;
    ss << i;
    return ss.str();
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

string get_cq_root_path() {
    auto app_dir = CQ->getAppDirectory();
    auto suffix = "app\\" CQ_APP_ID "\\";
    return app_dir[slice(0, app_dir.length() - strlen(suffix))];
}
