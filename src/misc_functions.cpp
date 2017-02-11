#include "misc_functions.h"

#include <cstdlib>
#include <cctype>
#include <sstream>

size_t curl_write_stringstream_callback(char* buf, size_t size, size_t nmemb, std::stringstream& ss)
{
    char* tmp = (char *)malloc(nmemb + 1);
    memcpy(tmp, buf, nmemb);
    tmp[nmemb] = '\0';
    ss << tmp;
    free(tmp);
    return size * nmemb;
}

bool isnumber(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it))
        ++it;
    return !s.empty() && it == s.end();
}

std::string itos(int64_t i)
{
    std::stringstream ss;
    ss << i;
    return ss.str();
}
