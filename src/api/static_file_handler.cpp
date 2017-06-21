// 
// static_file_handler.cpp : Implement static file handler.
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

#include "app.h"

#include <event2/buffer.h>
#include <event2/http.h>
#include <io.h>
#include <fcntl.h>

#include "helpers.h"

using namespace std;

const static struct table_entry {
    const char *extension;
    const char *content_type;
} content_type_table[] = {
    {"txt", "text/plain"},
    {"c", "text/plain"},
    {"cpp", "text/plain"},
    {"h", "text/plain"},
    {"hpp", "text/plain"},
    {"html", "text/html"},
    {"htm", "text/htm"},
    {"css", "text/css"},
    {"gif", "image/gif"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"png", "image/png"},
    {"pdf", "application/pdf"},
    {"ps", "application/postscript"},
    {"json", "application/json"},
    {nullptr, nullptr},
};

static const char *guess_content_type(const str &path) {
    auto last_period = strrchr(path.c_str(), '.');
    if (!last_period || strchr(last_period, '/')) {
        goto not_found; /* no exension */
    }
    auto extension = last_period + 1;
    for (auto ent = &content_type_table[0]; ent->extension; ++ent) {
        if (!evutil_ascii_strcasecmp(ent->extension, extension))
            return ent->content_type;
    }

not_found:
    return "application/misc";
}

void static_file_handler(evhttp_request *req, str path) {
    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        // only allow GET method
        evhttp_send_error(req, HTTP_BADMETHOD, nullptr);
        return;
    }

    if (!path.startswith("/data/")) {
        path = str(evhttp_uri_get_path(evhttp_request_get_evhttp_uri(req)));
        if (!path.startswith("/data/")) {
            evhttp_send_error(req, HTTP_BADREQUEST, nullptr);
            return;
        }
    }

    auto decoded_path = evhttp_uridecode(path.c_str(), 0, nullptr);
    if (strstr(decoded_path, "..")) {
        // ".." is not allowed to be present in the path
        L.d("文件请求", "URI 中有非法字符，已拒绝处理");
        evhttp_send_error(req, HTTP_BADREQUEST, nullptr);
        return;
    }

    auto full_path = (get_coolq_root() + decoded_path).replace("/", "\\");
    if (!isfile(full_path)) {
        // is not a file
        L.d("文件请求", "URI 所制定的内容不存在，或为非文件类型，无法传送");
        evhttp_send_error(req, HTTP_NOTFOUND, nullptr);
        return;
    }

    int fd;
    _sopen_s(&fd, ansi(full_path).c_str(), _O_RDONLY | _O_BINARY, _SH_DENYWR, 0);

    if (fd < 0) {
        // cannot open the file
        L.d("文件请求", "打开文件失败");
        evhttp_send_error(req, 403, nullptr);
        return;
    }

    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", guess_content_type(full_path));

    auto buf = evbuffer_new();
    evbuffer_add_file(buf, fd, 0, filesize(full_path));
    evhttp_send_reply(req, 200, nullptr, buf);
    L.d("文件请求", "文件内容已传送完毕");

    evbuffer_free(buf);
    free(decoded_path);

    L.i("文件请求", "已成功传送文件：" + full_path);
}
