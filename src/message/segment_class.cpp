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

#include <ctime>
#include <regex>
#include <fstream>

#include "./message_class.h"
#include "utils/crypt.h"
#include "utils/filesystem.h"

using namespace std;
namespace fs = filesystem;

static Message::Segment enhance_send_file(const Message::Segment &raw, const string &data_dir);
static Message::Segment enhance_receive_image(const Message::Segment &raw);

Message::Segment Message::Segment::enhanced(const Direction direction) const {
    if (direction == Directions::OUTWARD) {
        // messages to send
        if (this->type == "image") {
            return enhance_send_file(*this, "image");
        }
        if (this->type == "record") {
            return enhance_send_file(*this, "record");
        }
    }
    if (direction == Directions::INWARD) {
        // messages received
        if (this->type == "image") {
            return enhance_receive_image(*this);
        }
    }
    return *this;
}

static Message::Segment enhance_send_file(const Message::Segment &raw, const string &data_dir) {
    const auto file_it = raw.data.find("file");
    if (file_it == raw.data.end()) {
        // there is no "file" parameter, skip it
        return raw;
    }

    auto segment = raw;
    auto file = (*file_it).second;

    if (string_starts_with(file, "http://") || string_starts_with(file, "https://")) {
        const auto &url = file;
        const auto filename = md5_hash_hex(url) + ".tmp"; // despite of the format, we store all images as ".tmp"
        const auto filepath = sdk->directories().coolq() + "data\\" + data_dir + "\\" + filename;

        // check if to use cache
        auto use_cache = true; // use cache by default
        if (segment.data.find("cache") != segment.data.end() && segment.data["cache"] == "0") {
            use_cache = false;
        }
        const auto cached = fs::is_regular_file(ansi(filepath));

        if (use_cache && cached /* use cache */
            || download_remote_file(url, filepath, true) /* or perform download */) {
            segment.data["file"] = filename;
        }
    } else if (string_starts_with(file, "file://")) {
        const auto path = file.substr(strlen("file://"));
        const auto new_filename = md5_hash_hex(path) + ".tmp";

        const auto new_filepath = sdk->directories().coolq() + "data\\" + data_dir + "\\" + new_filename;
        try {
            fs::copy_file(ansi(path), ansi(new_filepath), true);
            segment.data["file"] = new_filename;
        } catch (fs::filesystem_error &) {
            // copy failed
        }
    } else if (string_starts_with(file, "base64://")) {
        const auto base64_encoded = file.substr(strlen("base64://"));
        const auto filename = "from_base64_" + to_string(time(nullptr)) + "_" + to_string(random_int(1, 1000)) + ".tmp";
        const auto filepath = sdk->directories().coolq() + "data\\" + data_dir + "\\" + filename;

        if (ofstream f(ansi(filepath), ios::binary | ios::out); f.is_open()) {
            f << base64_decode(base64_encoded);
            segment.data["file"] = filename;
        }
    }

    return segment;
}

static Message::Segment enhance_receive_image(const Message::Segment &raw) {
    const auto file_it = raw.data.find("file");
    if (file_it == raw.data.end()) {
        // there is no "file" parameter, skip it
        return raw;
    }

    auto segment = raw;
    const auto filename = (*file_it).second;

    if (!filename.empty()) {
        const auto cqimg_filename = filename + ".cqimg";
        const auto cqimg_filepath = sdk->directories().coolq() + "data\\image\\" + cqimg_filename;

        if (ifstream istrm(ansi(cqimg_filepath), ios::binary); istrm.is_open()) {
            string url, line;
            while (!istrm.eof()) {
                if (istrm >> line; string_starts_with(line, "url=")) {
                    url = line.substr(4);
                    break;
                }
            }
            if (!url.empty()) {
                segment.data["url"] = url;
            }
        }
    }
    return segment;
}
