#include "misc_functions.h"

#include <cstdlib>
#include <cctype>
#include <sstream>

#include "app.h"

using namespace std;

extern int ac;

size_t curl_write_stringstream_callback(char* buf, size_t size, size_t nmemb, stringstream& ss)
{
    char* tmp = (char *)malloc(nmemb + 1);
    memcpy(tmp, buf, nmemb);
    tmp[nmemb] = '\0';
    ss << tmp;
    free(tmp);
    return size * nmemb;
}

size_t curl_write_file_callback(char* buf, size_t size, size_t nmemb, FILE* fp)
{
    size_t written_size = 0;
    if (fp)
        written_size = fwrite(buf, size, nmemb, fp);
    return written_size;
}

bool isnumber(const string& s)
{
    string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it))
        ++it;
    return !s.empty() && it == s.end();
}

string itos(int64_t i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

void string_replace(string& str, const string& search, const string& replace)
{
    if (search.empty())
        return;
    string ws_ret;
    ws_ret.reserve(str.length());
    size_t start_pos = 0, pos;
    while ((pos = str.find(search, start_pos)) != string::npos)
    {
        ws_ret += str.substr(start_pos, pos - start_pos);
        ws_ret += replace;
        pos += search.length();
        start_pos = pos;
    }
    ws_ret += str.substr(start_pos);
    str.swap(ws_ret); // faster than str = wsRet;
}

string get_cq_root_path()
{
    string app_dir = CQ_getAppDirectory(ac);
    string suffix = "app\\" CQAPPID "\\";
    return app_dir.substr(0, app_dir.size() - suffix.size());;
}
