#include "cqcode.h"

#include <regex>
#include <fstream>
#include <curl/curl.h>

#include "encoding/md5.h"
#include "encoding/encoding.h"
#include "misc_functions.h"

using namespace std;

string message_escape(const string &msg) {
    string tmp = msg;
    string_replace(tmp, "&", "&amp;");
    string_replace(tmp, "[", "&#91;");
    string_replace(tmp, "]", "&#93;");
    string_replace(tmp, ",", "&#44;");
    return tmp;
}

string message_unescape(const string &msg) {
    string tmp = msg;
    string_replace(tmp, "&#91;", "[");
    string_replace(tmp, "&#93;", "]");
    string_replace(tmp, "&#44;", ",");
    string_replace(tmp, "&amp;", "&");
    return tmp;
}

static string enhance_cq_code_remote_file(string data_dir, smatch &match);
static string enhance_cq_code_parse_cqimg(smatch &match);

string enhance_cq_code(const string &msg, cqcode_enhance_mode mode) {
    string result;

    // 0: full CQ code function message, 1: function name, 2: params string
    regex exp("\\[CQ:([\\._\\-0-9A-Za-z]+?)(?:\\s*\\]|\\s*,\\s*(.*?)\\])");

    smatch match;
    string::const_iterator search_iter(msg.cbegin());
    while (regex_search(search_iter, msg.cend(), match, exp)) {
        result += string(search_iter, search_iter + match.position()); // normal message before this current CQ code

        string function = match.str(1);
        if (mode == CQCODE_ENHANCE_OUTCOMING) {
            // messages sent out to others
            if (function == "image")
                result += enhance_cq_code_remote_file("image", match);
            else if (function == "record")
                result += enhance_cq_code_remote_file("record", match);
            else
                result += match.str();
        } else if (mode == CQCODE_ENHANCE_INCOMING) {
            // messages received from others
            if (function == "image") {
                result += enhance_cq_code_parse_cqimg(match);
            } else {
                result += match.str();
            }
        }

        search_iter += match.position() + match.length();
    }
    result += string(search_iter, msg.cend()); // add the rest plain text
    return result;
}

static string enhance_cq_code_remote_file(string data_dir, smatch &match) {
    // enhance CQ functions to support file from the internet or a different directory in filesystem
    string cqcode_call = match.str(0); // full CQ code function message
    string params = match.str(2);
    smatch m;
    if (regex_search(params, m, regex("file=(https?:\\/\\/[^,\\?]+\\??[^,\\?]*)"))) {
        string raw_url = m.str(1);
        string url = message_unescape(raw_url);
        //        string ext = m.str(2);
        MD5 md5(url);
        string hash = md5.toStr();
        string filename = hash + ".jpg";

        string filepath = get_cq_root_path() + "data\\" + data_dir + "\\" + filename;
        FILE *fp = NULL;
        fopen_s(&fp, filepath.c_str(), "wb");
        if (fp) {
            CURL *curl = curl_easy_init();
            curl_easy_setopt(curl, CURLOPT_URL, url);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_file_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

            struct curl_slist *chunk = NULL;
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
        string_replace(cqcode_call, raw_url, filename);
    } else if (regex_search(params, m, regex("file=file:\\/\\/([^,\\?]+(\\.[^\\s,\\?]+))"))) {
        string raw_path = m.str(1);
        string path = message_unescape(raw_path);
        string ext = m.str(2);
        MD5 md5(path);
        string hash = md5.toStr();
        string new_filename = hash + ext;

        string new_filepath = get_cq_root_path() + "data\\" + data_dir + "\\" + new_filename;
        wstring path_wstr = utf8_to_wstr(path.c_str());
        wstring new_filepath_wstr = utf8_to_wstr(new_filepath.c_str());
        CopyFileW(path_wstr.c_str(), new_filepath_wstr.c_str(), false); // copy remote file

        string_replace(cqcode_call, "file://" + raw_path, new_filename);
    }
    return cqcode_call;
}

static string enhance_cq_code_parse_cqimg(smatch &match) {
    string cqcode_call = match.str(0); // full CQ code function message
    string params = match.str(2);
    smatch m;
    if (regex_search(params, m, regex("file=([a-zA-Z0-9]+\\.[a-zA-Z0-9]+)"))) {
        string filename = m.str(1);
        string cqimg_filename = filename + ".cqimg";
        string cqimg_filepath = get_cq_root_path() + "data\\image\\" + cqimg_filename;
        ifstream istrm(cqimg_filepath);
        if (istrm.is_open()) {
            string url = "";
            string line;
            while (!istrm.eof()) {
                istrm >> line;
                smatch url_match;
                if (regex_search(line, url_match, regex("url=(.*?)\\?"))) {
                    url = url_match.str(1);
                    break;
                }
            }
            if (url.length() != 0) {
                string_replace(cqcode_call, filename, filename + ",url=" + message_escape(url));
            }
        }
    }
    return cqcode_call;
}
