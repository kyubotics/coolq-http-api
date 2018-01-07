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
#include <unordered_set>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <websocketpp/common/md5.hpp>

#include "./message_class.h"
#include "utils/http_utils.h"

using namespace std;
namespace fs = boost::filesystem;

using boost::algorithm::starts_with;
using websocketpp::md5::md5_hash_hex;

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
    auto &file = (*file_it).second;

    string filename;
    function<bool()> make_file = nullptr;

    if (starts_with(file, "http://") || starts_with(file, "https://")) {
        const auto &url = file;

        // check if to use cache
        auto use_cache = true; // use cache by default
        if (segment.data.find("cache") != segment.data.end() && segment.data["cache"] == "0") {
            use_cache = false;
        }

        if (use_cache) {
            filename = md5_hash_hex(url) + ".tmp";
        } else {
            filename = md5_hash_hex(url + to_string(random_int(1, 10000))) + ".tmp";
        }

        make_file = [=] {
            const auto filepath = data_file_full_path(data_dir, filename);

            if (use_cache && fs::is_regular_file(s2ws(filepath)) /* use cache */
                || download_remote_file(url, filepath, true) /* or perform download */) {
                return true;
            }
            return false;
        };
    } else if (starts_with(file, "file://")) {
        const auto src_filepath = file.substr(strlen("file://"));
        filename = md5_hash_hex(src_filepath) + ".tmp";
        make_file = [=] {
            const auto filepath = data_file_full_path(data_dir, filename);

            try {
                copy_file(s2ws(src_filepath), s2ws(filepath), fs::copy_option::overwrite_if_exists);
                return true;
            } catch (fs::filesystem_error &) {
                // copy failed
                return false;
            }
        };
    } else if (starts_with(file, "base64://")) {
        filename = md5_hash_hex("from_base64_"
            + to_string(time(nullptr)) + "_"
            + to_string(random_int(1, 10000))) + ".tmp";
        make_file = [=, &file] {
            const auto filepath = data_file_full_path(data_dir, filename);
            const auto base64_encoded = file.substr(strlen("base64://"));

            if (ofstream f(ansi(filepath), ios::binary | ios::out); f.is_open()) {
                f << base64_decode(base64_encoded);
                return true;
            }
            return false;
        };
    }

    static unordered_set<string> files_in_process;
    static mutex files_in_process_mutex;
    static condition_variable cv;

    if (!filename.empty() && make_file != nullptr) {
        unique_lock<mutex> lk(files_in_process_mutex);
        // wait until there is no other thread processing the same file
        cv.wait(lk, [=] { return files_in_process.find(filename) == files_in_process.cend(); });
        files_in_process.insert(filename);
        lk.unlock();

        // we are now sure that only our current thread is processing the file
        if (make_file()) {
            // succeeded
            segment.data["file"] = filename;
        }

        // ok, we can let other threads play
        lk.lock();
        files_in_process.erase(filename);
        lk.unlock();
        cv.notify_all();
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
        const auto cqimg_filepath = data_file_full_path("image", cqimg_filename);

        if (ifstream istrm(ansi(cqimg_filepath), ios::binary); istrm.is_open()) {
            boost::property_tree::ptree pt;
            read_ini(istrm, pt);
            auto url = pt.get_optional<string>("image.url");
            if (url && !url->empty()) {
                segment.data["url"] = url.value();
            }
        }
    }
    return segment;
}
