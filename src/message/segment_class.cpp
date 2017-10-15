// 
// segment_class.cpp : Implement Message::Segment class.
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

#include <cpprest/filestream.h>
#include <websocketpp/common/md5.hpp>
#include <boost/filesystem.hpp>

#include "./message_class.h"
#include "utils/rest_client.h"

using namespace std;
using boost::algorithm::starts_with;
using websocketpp::md5::md5_hash_hex;

static Message::Segment enhance_remote_file(const Message::Segment &raw, string data_dir);
static Message::Segment enhance_parse_cqimg(const Message::Segment &raw);

Message::Segment Message::Segment::enhanced(const Direction direction) const {
    Segment result;
    if (direction == Directions::OUTWARD) {
        // messages to send
        if (this->type == "image") {
            result = enhance_remote_file(*this, "image");
        } else if (this->type == "record") {
            result = enhance_remote_file(*this, "record");
        } else {
            result = *this;
        }

    } else if (direction == Directions::INWARD) {
        // messages received
        if (this->type == "image") {
            result = enhance_parse_cqimg(*this);
        } else {
            result = *this;
        }
    }
    return result;
}

static Message::Segment enhance_remote_file(const Message::Segment &raw, string data_dir) {
    const auto file_it = raw.data.find("file");
    if (file_it == raw.data.end()) {
        // there is no "file" parameter, skip it
        return raw;
    }

    auto segment = raw;
    auto file = (*file_it).second;

    if (starts_with(file, "http://") || starts_with(file, "https://")) {
        const auto &url = file;
        const auto ws_url = s2ws(url);
        const auto filename = md5_hash_hex(url) + ".tmp"; // despite of the format, we store all images as ".tmp"
        const auto filepath = sdk->get_coolq_directory() + "data\\" + data_dir + "\\" + filename;
        const auto ws_filepath = s2ws(filepath);

        // check if to use cache
        auto use_cache = true; // use cache by default
        if (segment.data.find("cache") != segment.data.end() && segment.data["cache"] == "0") {
            use_cache = false;
        }
        const auto cached = boost::filesystem::is_regular_file(ws_filepath);

        if (!cached || !use_cache) {
            // perform download
            using concurrency::streams::ostream;
            using concurrency::streams::fstream;

            optional<ostream> file_stream;

            fstream::open_ostream(ws_filepath).then([&](ostream out_file) {
                file_stream = out_file;

                http_client client(ws_url);
                http_request request(http::methods::GET);
                request.headers().add(L"User-Agent",
                                      L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
                                      "AppleWebKit/537.36 (KHTML, like Gecko) "
                                      "Chrome/56.0.2924.87 Safari/537.36");
                request.headers().add(L"Referer", ws_url);

                return client.request(request);
            }).then([&](http_response resp) {
                if (resp.status_code() < 300 && resp.status_code() >= 200) {
                    // we can assume here that the request is succeeded
                    return resp.body().read_to_end(file_stream->streambuf());
                }
                return pplx::task_from_result<size_t>(0);
            }).then([&](size_t size) {
                if (size > 0) {
                    // download succeeded
                    segment.data["file"] = filename;
                }
                return file_stream->close();
            }).wait();
        } else {
            segment.data["file"] = filename;
        }
    } else if (starts_with(file, "file://")) {
        const auto path = file.substr(strlen("file://"));
        const auto new_filename = md5_hash_hex(path) + ".tmp";

        const auto new_filepath = sdk->get_coolq_directory() + "data\\" + data_dir + "\\" + new_filename;
        if (CopyFileW(s2ws(path).c_str(), s2ws(new_filepath).c_str(), false)) {
            // copy remote file succeeded
            segment.data["file"] = new_filename;
        }
    } else if (starts_with(file, "base64://")) {
        const auto base64_encoded = file.substr(strlen("base64://"));
        const auto filename = "from_base64.tmp"; // despite of the format, we store all images as ".tmp"
        const auto filepath = sdk->get_coolq_directory() + "data\\" + data_dir + "\\" + filename;

        if (fstream f(ansi(filepath), ios::binary | ios::out); f.is_open()) {
            f << base64_decode(base64_encoded);
            f.close();
            segment.data["file"] = filename;
        }
    }

    return segment;
}

static Message::Segment enhance_parse_cqimg(const Message::Segment &raw) {
    const auto file_it = raw.data.find("file");
    if (file_it == raw.data.end()) {
        // there is no "file" parameter, skip it
        return raw;
    }

    auto segment = raw;
    const auto filename = (*file_it).second;

    if (!filename.empty()) {
        const auto cqimg_filename = filename + ".cqimg";
        const auto cqimg_filepath = sdk->get_coolq_directory() + "data\\image\\" + cqimg_filename;
        if (ifstream istrm(ansi(cqimg_filepath), ios::binary); istrm.is_open()) {
            string url = "";
            string line;
            while (!istrm.eof()) {
                istrm >> line;
                if (starts_with(line, "url=")) {
                    url = line.substr(strlen("url="));
                    break;
                }
            }
            if (url.length() != 0) {
                segment.data["url"] = url;
            }
            istrm.close();
        }
    }
    return segment;
}
