#include "misc_functions.h"

#include <cstdlib>
#include <cctype>

size_t curl_write_stringstream_callback(char *buf, size_t size, size_t nmemb, std::stringstream &ss)
{
    char *tmp = (char *)malloc(nmemb + 1);
    memcpy(tmp, buf, nmemb);
    tmp[nmemb] = '\0';
    ss << tmp;
    free(tmp);
    return size * nmemb;
}

std::string string_format(const std::string &fmt, ...)
{
    int size = ((int)fmt.size()) * 2 + 50; // Use a rubric appropriate for your code
    std::string str;
    va_list ap;
    while (1)
    { // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size)
        { // Everything worked
            str.resize(n);
            return str;
        }
        if (n > -1)       // Needed size returned
            size = n + 1; // For null char
        else
            size *= 2; // Guess at a larger size (OS specific)
    }
    return str;
}

bool isnumber(const std::string &s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it))
        ++it;
    return !s.empty() && it == s.end();
}
