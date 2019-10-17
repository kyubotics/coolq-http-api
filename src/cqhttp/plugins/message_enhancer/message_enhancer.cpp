#include "./message_enhancer.h"

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>
#include <mutex>
#include <unordered_set>

#include "cqhttp/utils/crypt.h"
#include "cqhttp/utils/filesystem.h"
#include "cqhttp/utils/http.h"
#include "cqhttp/utils/random.h"
#include "cqhttp/utils/string.h"
#include "cqsdk/utils/base64.h"

#include "rcnb/decode.h"

using namespace std;

namespace cqhttp::plugins {
    using cq::Message;
    using cq::MessageSegment;
    using boost::algorithm::starts_with;
    using utils::fs::data_file_full_path;
    using utils::crypt::md5_hash_hex;
    using utils::random::random_int;
    using utils::to_bool;
    namespace fs = std::filesystem;
    namespace base64 = cq::utils::base64;

    static MessageSegment enhance_send_file(const MessageSegment &raw, const string &data_dir);
    static MessageSegment enhance_receive_image(const MessageSegment &raw);

    static struct FileType {
        string ext;
        string mime;
    };

    static FileType detect_file_type(const string &buffer) {
        // see https://github.com/sindresorhus/file-type/blob/master/index.js
        const auto check = [&buffer](const vector<int> &header) {
            auto i = 0;
            for (; i < header.size() && i < buffer.size(); i++) {
                if (header[i] != static_cast<unsigned char>(buffer[i])) return false;
            }
            return i == header.size();
        };
        if (check({0xFF, 0xD8, 0xFF})) {
            return {"jpg", "image/jpeg"};
        } else if (check({0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A})) {
            return {"png", "image/png"};
        } else if (check({0x47, 0x49, 0x46})) {
            return {"gif", "image/gif"};
        } else if (check({0x42, 0x4D})) {
            return {"bmp", "image/bmp"};
        }
        return {};
    }

    void MessageEnhancer::hook_message_event(EventContext<cq::MessageEvent> &ctx) {
        Message msg;
        for (const auto &segment : ctx.event.message) {
            if (segment.type == "image") {
                msg.push_back(enhance_receive_image(segment));
            } else {
                msg.push_back(segment);
            }
        }
        ctx.data["message"] = msg;

        ctx.next();
    }

    void MessageEnhancer::hook_before_action(ActionContext &ctx) {
        if (regex_match(ctx.action, regex("send[_a-z]*_msg"))
            && ctx.params.raw.find("message") != ctx.params.raw.end()) {
            auto msg = ctx.params.get_message("message");
            for (auto &segment : msg) {
                if (segment.type == "image") {
                    segment = enhance_send_file(segment, "image");
                } else if (segment.type == "record") {
                    segment = enhance_send_file(segment, "record");
                }
            }
            ctx.params.raw["message"] = msg;
        }

        ctx.next();
    }

    static MessageSegment enhance_send_file(const MessageSegment &raw, const string &data_dir) {
        if (!raw.data.count("file")) {
            // there is no "file" parameter, skip it
            return raw;
        }

        auto segment = raw;
        auto &file = raw.data.at("file");

        string filename, ext;
        function<bool()> make_file = nullptr;

        const static auto check_ext = [](const auto &name) -> string {
            if (smatch m; regex_search(name, m, regex(R"(\.(png|jpg|jpeg|gif|bmp)$)", regex::flag_type::icase))) {
                return m.str(1);
            }
            return "tmp";
        };

        if (starts_with(file, "http://") || starts_with(file, "https://")) {
            const auto &url = file;

            // check if to use cache
            auto use_cache = true; // use cache by default
            if (segment.data.find("cache") != segment.data.end()) {
                use_cache = to_bool(segment.data["cache"], true);
            }

            if (use_cache) {
                filename = md5_hash_hex(url) + "." + check_ext(url);
            } else {
                filename = md5_hash_hex(url + to_string(random_int(1, 10000))) + "." + check_ext(url);
            }

            make_file = [=] {
                const auto filepath = data_file_full_path(data_dir, filename);

                if (use_cache && fs::is_regular_file(s2ws(filepath)) /* use cache */
                    || utils::http::download_file(url, filepath, true) /* or perform download */) {
                    return true;
                }
                return false;
            };
        } else if (smatch m; regex_search(file, m, regex(R"(^file:\/{0,3})"))) {
            const auto src_filepath = file.substr(m.str().length());
            filename = md5_hash_hex(src_filepath) + "." + check_ext(file);
            make_file = [=] {
                const auto filepath = data_file_full_path(data_dir, filename);

                try {
                    copy_file(s2ws(src_filepath), s2ws(filepath), fs::copy_options::overwrite_existing);
                    return true;
                } catch (fs::filesystem_error &) {
                    // copy failed
                    return false;
                }
            };
        } else if (starts_with(file, "base64://")) {
            filename = md5_hash_hex("from_base64_" + to_string(time(nullptr)) + "_"
                                    + to_string(random_int(1, 10000))); // note that there isn't an extension yet
            make_file = [=, &file, &filename] {
                const auto base64_encoded = file.substr(strlen("base64://"));
                const auto raw = base64::decode(base64_encoded);
                const auto file_type = detect_file_type(raw);
                filename = filename + "." + (file_type.ext.empty() ? "tmp" : file_type.ext);
                const auto filepath = data_file_full_path(data_dir, filename);

                if (ofstream f(ansi(filepath), ios::binary | ios::out); f.is_open()) {
                    f << raw;
                    return true;
                }
                return false;
            };
        } else if (starts_with(file, "rcnb://")) {
            filename = md5_hash_hex("from_rcnb_" + to_string(time(nullptr)) + "_"
                                    + to_string(random_int(1, 10000))); // note that there isn't an extension yet
            make_file = [=, &file, &filename] {
                const auto rcnb_encoded = file.substr(strlen("rcnb://"));
                rcnb::decoder dec;
                stringstream ss;
                wstringstream wss;
                wss << s2ws(rcnb_encoded);
                dec.decode(wss, ss);
                const auto raw = ss.str();
                const auto file_type = detect_file_type(raw);
                filename = filename + "." + (file_type.ext.empty() ? "tmp" : file_type.ext);
                const auto filepath = data_file_full_path(data_dir, filename);

                if (ofstream f(ansi(filepath), ios::binary | ios::out); f.is_open()) {
                    f << raw;
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

    static MessageSegment enhance_receive_image(const MessageSegment &raw) {
        if (raw.data.find("url") != raw.data.end()) {
            // already has "url" parameter, skip it
        }

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
} // namespace cqhttp::plugins
