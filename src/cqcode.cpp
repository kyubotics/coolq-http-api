// 
// cqcode.cpp : Implement functions about CQ code.
// 
// Copyright (C) 2017  Richard Chien <richardchienthebest@gmail.com>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

#include "cqcode.h"

#include <regex>
#include <fstream>
#include <curl/curl.h>

#include "encoding/md5.h"
#include "helpers.h"

using namespace std;

str message_escape(const str &msg) {
    string tmp = msg;
    string_replace(tmp, "&", "&amp;");
    string_replace(tmp, "[", "&#91;");
    string_replace(tmp, "]", "&#93;");
    string_replace(tmp, ",", "&#44;");
    return tmp;
}

str message_unescape(const str &msg) {
    string tmp = msg;
    string_replace(tmp, "&#91;", "[");
    string_replace(tmp, "&#93;", "]");
    string_replace(tmp, "&#44;", ",");
    string_replace(tmp, "&amp;", "&");
    return tmp;
}

str make_cqcode(const str &function, const str &params_str) {
    return str("[CQ:{},{}]").format(function, params_str);
}

str make_cqcode(const str &function, json_t *params_obj) {
    vector<str> params;
    const char *key;
    json_t *value;
    json_object_foreach(params_obj, key, value) {
        if (json_is_string(value)) {
            params.push_back(str(key) + "=" + message_escape(json_string_value(value)));
        }
    }
    return make_cqcode(function, str(",").join(params));
}

static str enhance_cqcode_remote_file(str data_dir, const smatch &match);
static str enhance_cqcode_parse_cqimg(const smatch &match);

str enhance_cqcode(const str &msg, int mode) {
    vector<string> parts;
    smatch match;
    auto search_iter(msg.c_begin());
    while (regex_search(search_iter, msg.c_end(), match, CQCODE_REGEX)) {
        // normal message before this current CQ code
        // NOTE: because "search_iter" is a string::iterator, we are fine to add "search_iter" and "match.position()"
        // if it's a str::iterator, the following line will break
        // because "match.position()" is not the real UTF-8 character's position, but the byte's position
        parts.push_back(string(search_iter, search_iter + match.position()));

        // handle CQ code
        auto function = match.str(1);
        string cqcode;
        if (mode == CQCODE_ENHANCE_OUTCOMING) {
            // messages to send
            if (function == "image") {
                cqcode = enhance_cqcode_remote_file("image", match);
            } else if (function == "record") {
                cqcode = enhance_cqcode_remote_file("record", match);
            } else {
                cqcode = match.str();
            }

        } else if (mode == CQCODE_ENHANCE_INCOMING) {
            // messages received
            if (function == "image") {
                cqcode = enhance_cqcode_parse_cqimg(match);
            } else {
                cqcode = match.str();
            }
        }
        parts.push_back(cqcode);

        search_iter += match.position() + match.length();
    }
    parts.push_back(string(search_iter, msg.c_end())); // add the remained plain text
    return str().join(parts);
}

static str enhance_cqcode_remote_file(str data_dir, const smatch &match) {
    // enhance CQ functions to support file from internet or a different directory in filesystem
    auto function = match.str(1);
    auto params = match.str(2);
    smatch m;
    if (regex_search(params, m, regex("file=(https?:\\/\\/[^,]+)"))) {
        auto url = message_unescape(m.str(1));
        auto filename = MD5(url).toStr() + ".tmp"; // despite of the format, we store all images as ".tmp"
        auto filepath = get_coolq_root() + "data\\" + data_dir + "\\" + filename;

        // check if to use cache
        auto use_cache = true; // use cache by default
        if (regex_search(params, regex("cache=0"))) {
            use_cache = false;
        }
        auto cached = isfile(filepath);

        if (!cached || !use_cache) {
            // perform download

            FILE *fp = nullptr;
            fopen_s(&fp, filepath.c_str(), "wb");
            if (fp) {
                auto curl = curl_easy_init();
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

                curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
                auto cb = [](char *buf, size_t size, size_t nmemb, void *fp) {
                            size_t written_size = 0;
                            if (fp) {
                                written_size = fwrite(buf, size, nmemb, static_cast<FILE *>(fp));
                            }
                            return written_size;
                        };
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, static_cast<CURLWriteFunctionPtr>(cb));

                struct curl_slist *chunk = nullptr;
                chunk = curl_slist_append(chunk,
                                          "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
                                          "AppleWebKit/537.36 (KHTML, like Gecko) "
                                          "Chrome/56.0.2924.87 Safari/537.36");
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

                curl_easy_perform(curl); // download remote file

                fclose(fp);
                curl_easy_cleanup(curl);
                curl_slist_free_all(chunk);
            }
        }

        params = params.substr(0, m.position()) + "file=" + filename + params.substr(m.position() + m.length());
    } else if (regex_search(params, m, regex("file=file:\\/\\/([^\\r\\n,]+(\\.[^\\s\\r\\n,]+))"))) {
        auto path = message_unescape(m.str(1));
        auto new_filename = MD5(path).toStr() + ".tmp";

        auto new_filepath = get_coolq_root() + "data\\" + data_dir + "\\" + new_filename;
        CopyFileW(path.to_wstring().c_str(), new_filepath.to_wstring().c_str(), false); // copy remote file

        params = params.substr(0, m.position()) + "file=" + new_filename + params.substr(m.position() + m.length());
    }
    return make_cqcode(function, params);
}

static str enhance_cqcode_parse_cqimg(const smatch &match) {
    auto function = match.str(1);
    auto params = match.str(2);
    smatch m;
    if (regex_search(params, m, regex("file=([a-zA-Z0-9]+\\.[a-zA-Z0-9]+)"))) {
        auto filename = m.str(1);
        auto cqimg_filename = filename + ".cqimg";
        string cqimg_filepath = get_coolq_root() + "data\\image\\" + cqimg_filename;
        ifstream istrm(cqimg_filepath);
        if (istrm.is_open()) {
            string url = "";
            string line;
            while (!istrm.eof()) {
                istrm >> line;
                if (str(line).startswith("url=")) {
                    url = line.substr(4);
                    break;
                }
            }
            if (url.length() != 0) {
                params = params + ",url=" + message_escape(url);
            }
        }
    }
    return make_cqcode(function, params);
}
