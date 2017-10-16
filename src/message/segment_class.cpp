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

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <websocketpp/common/md5.hpp>

#include "./message_class.h"
#include "utils/rest_client.h"

using namespace std;

namespace fs {
    using namespace boost::filesystem;
}

using boost::algorithm::starts_with;
using websocketpp::md5::md5_hash_hex;

static Message::Segment enhance_send_file(const Message::Segment &raw, const string &data_dir);
static Message::Segment enhance_receive_image(const Message::Segment &raw);

Message::Segment Message::Segment::enhanced(const Direction direction) const {
    Segment result;
    if (direction == Directions::OUTWARD) {
        // messages to send
        if (this->type == "image") {
            result = enhance_send_file(*this, "image");
        } else if (this->type == "record") {
            result = enhance_send_file(*this, "record");
        } else {
            result = *this;
        }

    } else if (direction == Directions::INWARD) {
        // messages received
        if (this->type == "image") {
            result = enhance_receive_image(*this);
        } else {
            result = *this;
        }
    }
    return result;
}

static Message::Segment enhance_send_file(const Message::Segment &raw, const string &data_dir) {
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
        const auto filepath = sdk->directories().coolq() + "data\\" + data_dir + "\\" + filename;
        const auto ws_filepath = s2ws(filepath);

        // check if to use cache
        auto use_cache = true; // use cache by default
        if (segment.data.find("cache") != segment.data.end() && segment.data["cache"] == "0") {
            use_cache = false;
        }
        const auto cached = boost::filesystem::is_regular_file(ws_filepath);

        if (use_cache && cached /* use cache */
            || download_remote_file(url, filepath, true) /* or perform download */) {
            segment.data["file"] = filename;
        }
    } else if (starts_with(file, "file://")) {
        const auto path = file.substr(strlen("file://"));
        const auto new_filename = md5_hash_hex(path) + ".tmp";

        const auto new_filepath = sdk->directories().coolq() + "data\\" + data_dir + "\\" + new_filename;
        try {
            copy_file(ansi(path), ansi(new_filepath), fs::copy_option::overwrite_if_exists);
            segment.data["file"] = new_filename;
        } catch (fs::filesystem_error &) {
            // copy failed
        }
    } else if (starts_with(file, "base64://")) {
        const auto base64_encoded = file.substr(strlen("base64://"));
        const auto filename = "from_base64.tmp"; // despite of the format, we store all images as ".tmp"
        const auto filepath = sdk->directories().coolq() + "data\\" + data_dir + "\\" + filename;

        if (ofstream f(ansi(filepath), ios::binary | ios::out); f.is_open()) {
            f << base64_decode(base64_encoded);
            f.close();
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
            boost::property_tree::ptree pt;
            read_ini(istrm, pt);
            auto url = pt.get_optional<string>("image.url");
            if (url && !url->empty()) {
                segment.data["url"] = url.value();
            }
            istrm.close();
        }
    }
    return segment;
}
