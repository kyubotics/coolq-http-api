// 
// MessageSegment.cpp : Implement methods of Message::Segment class.
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

#include "Message.h"

#include "app.h"

#include <fstream>

#include "encoding/md5.h"
#include "helpers.h"
#include "curl_wrapper.h"

using namespace std;

static Message::Segment enhance_remote_file(const Message::Segment &raw, str data_dir);
static Message::Segment enhance_parse_cqimg(const Message::Segment &raw);

Message::Segment Message::Segment::enhanced(int mode) const {
    Segment result;
    if (mode == ENHANCE_OUTCOMING) {
        // messages to send
        if (this->type == "image") {
            result = enhance_remote_file(*this, "image");
        } else if (this->type == "record") {
            result = enhance_remote_file(*this, "record");
        } else {
            result = *this;
        }

    } else if (mode == ENHANCE_INCOMING) {
        // messages received
        if (this->type == "image") {
            result = enhance_parse_cqimg(*this);
        } else {
            result = *this;
        }
    }
    return result;
}

static Message::Segment enhance_remote_file(const Message::Segment &raw, str data_dir) {
    auto file_it = raw.data.find("file");
    if (file_it == raw.data.end()) {
        // there is no "file" parameter, skip it
        return raw;
    }

    auto segment = raw;
    auto file = (*file_it).second;

    if (file.startswith("http://") || file.startswith("https://")) {
        auto &url = file;
        auto filename = MD5(url).toStr() + ".tmp"; // despite of the format, we store all images as ".tmp"
        auto filepath = get_coolq_root() + "data\\" + data_dir + "\\" + filename;

        // check if to use cache
        auto use_cache = true; // use cache by default
        if (segment.data.find("cache") != segment.data.end() && segment.data["cache"] == "0") {
            use_cache = false;
        }
        auto cached = isfile(filepath);

        if (!cached || !use_cache) {
            // perform download
            ofstream f(ansi(filepath), ios::binary);
            if (f.is_open()) {
                auto req = curl::Request(url, curl::Headers{{"User-Agent",
                                             "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
                                             "AppleWebKit/537.36 (KHTML, like Gecko) "
                                             "Chrome/56.0.2924.87 Safari/537.36"}});
                req.write_data = &f;
                req.write_func = [](char *buf, size_t size, size_t count, void *file) {
                            *static_cast<ofstream *>(file) << string(buf, count);
                            return size * count;
                        };
                auto resp = req.get();
                f.close();

                if (resp.curl_code == CURLE_OK && resp.status_code == 200) {
                    segment.data["file"] = filename;
                }
            }
        } else {
            segment.data["file"] = filename;
        }
    } else if (file.startswith("file://")) {
        auto path = file[slice(str("file://").length())];
        auto new_filename = MD5(path).toStr() + ".tmp";

        auto new_filepath = get_coolq_root() + "data\\" + data_dir + "\\" + new_filename;
        auto ok = CopyFileW(path.to_wstring().c_str(), new_filepath.to_wstring().c_str(), false); // copy remote file
        if (ok) {
            segment.data["file"] = new_filename;
        }
    }

    return segment;
}

static Message::Segment enhance_parse_cqimg(const Message::Segment &raw) {
    auto file_it = raw.data.find("file");
    if (file_it == raw.data.end()) {
        // there is no "file" parameter, skip it
        return raw;
    }

    auto segment = raw;
    auto filename = (*file_it).second;

    if (filename) {
        auto cqimg_filename = filename + ".cqimg";
        auto cqimg_filepath = get_coolq_root() + "data\\image\\" + cqimg_filename;
        ifstream istrm(ansi(cqimg_filepath), ios::binary);
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
                segment.data["url"] = url;
            }
            istrm.close();
        }
    }
    return segment;
}
