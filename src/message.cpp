// 
// Message.cpp : Implement methods of Message class.
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

#include "cqcode.h"

using namespace std;

Message::Message(const str &msg_str) : msg_str_(msg_str), msg_json_(nullptr) {}

Message::Message(json_t *msg_json) : msg_json_(nullptr) {
    json_incref(msg_json);
}

Message::Message(const Message &other) {
    this->msg_str_ = other.msg_str_;
    this->msg_json_ = other.msg_json_;
    json_incref(this->msg_json_);
    this->segments_ = other.segments_;
}

Message::~Message() {
    json_decref(this->msg_json_);
}

str Message::process_outcoming() const {
    if (this->msg_str_) {
        // string already exists
        return enhance_cqcode(this->msg_str_, CQCODE_ENHANCE_OUTCOMING);
    }

    if (json_is_string(this->msg_json_)) {
        return json_string_value(this->msg_json_);
    }

    str msg;
    if (json_is_array(this->msg_json_)) {
        size_t i;
        json_t *part;
        json_array_foreach(this->msg_json_, i, part) {
            if (!json_is_object(part)) {
                continue;
            }
            auto type_json = json_object_get(part, "type");
            auto data_json = json_object_get(part, "data");
            if (!(json_is_string(type_json) && json_is_object(data_json))) {
                continue;
            }

            str type = json_string_value(type_json);
            if (type == "text") {
                // plain text
                auto text_json = json_object_get(data_json, "text");
                if (!json_is_string(text_json)) { continue; }
                msg += message_escape(json_string_value(text_json));
            } else {
                msg += enhance_cqcode(make_cqcode(type, data_json), CQCODE_ENHANCE_OUTCOMING);
            }
        }
    }

    return msg;
}

json_t *Message::process_incoming(str msg_fmt) const {
    if (!msg_fmt) {
        msg_fmt = CQ->config.post_message_format;
    }

    str msg_str;
    if (this->msg_str_) {
        msg_str = this->msg_str_;
    } else if (json_is_string(this->msg_json_)) {
        msg_str = json_string_value(this->msg_json_);
    }

    json_t *result_json = nullptr;
    if (msg_fmt == MSG_FMT_ARRAY) {
        result_json = json_array();
    }
    str result_str;

    // process every message part
    auto process_part = [&](str part /* escaped */, bool is_cqcode) {
                if (!part) { return; }
                if (is_cqcode) {
                    part = enhance_cqcode(part, CQCODE_ENHANCE_INCOMING);
                }
                if (msg_fmt == MSG_FMT_STRING) {
                    result_str += part;
                } else if (msg_fmt == MSG_FMT_ARRAY) {
                    if (!is_cqcode) {
                        // plain text
                        json_array_append_new(result_json,
                                              json_pack("{s:s, s:o}",
                                                        "type", "text",
                                                        "data", json_pack("{s:s}", "text", message_unescape(part).c_str())));
                    }

                    // we are sure that "part" is a CQ code now
                    smatch match;
                    if (regex_search(part.c_begin(), part.c_end(), match, CQCODE_REGEX)) {
                        // it should reach here every time

                        auto function = match.str(1);
                        auto params = match.str(2);

                        json_t *params_json = nullptr;
                        if (params.length() > 0) {
                            // has parameters
                            params_json = json_object();
                            stringstream params_ss(params);
                            while (params_ss.good()) {
                                // split key and value
                                string key, value;
                                getline(params_ss, key, '=');
                                getline(params_ss, value, ',');

                                if (key.length() > 0 && value.length() > 0) {
                                    json_object_set_new(params_json, key.c_str(), json_string(message_unescape(value).c_str()));
                                }
                            }
                        }

                        json_array_append_new(result_json,
                                              json_pack("{s:s, s:o?}",
                                                        "type", function.c_str(),
                                                        "data", params_json));
                    }
                }
            };

    if (msg_str) {
        // convert from string format
        smatch match;
        auto search_iter(msg_str.c_begin());
        while (regex_search(search_iter, msg_str.c_end(), match, regex(R"(\[CQ:((?:.|\r|\n)*?)\])"))) {
            // normal message before this current CQ code
            // NOTE: because "search_iter" is a string::iterator, we are fine to add "search_iter" and "match.position()"
            // if it's a str::iterator, the following line will break
            // because "match.position()" is not the real UTF-8 character's position, but the byte's position
            process_part(string(search_iter, search_iter + match.position()), false);
            search_iter += match.position() + match.length();

            str cqcode_full = match.str(0);

            // special case
            if (match.str(1) == "bface") {
                // format on CoolQ Air: "[CQ:bface]&#91;bface text&#93;the rest part of message"
                smatch m;
                if (regex_search(search_iter, msg_str.c_end(), m, regex("^&#91;(.+?)&#93;"))) {
                    auto bface_text = m.str(1);
                    cqcode_full = cqcode_full[slice(0, -1)] + ",text=" + bface_text + "]";
                    search_iter += m.length();
                }
            }

            process_part(cqcode_full, true);
        }
        process_part(string(search_iter, msg_str.c_end()), false);
    } else if (json_is_array(this->msg_json_)) {
        // convert from array format
        size_t i;
        json_t *part;
        json_array_foreach(this->msg_json_, i, part) {
            if (!json_is_object(part)) {
                continue;
            }
            auto type_json = json_object_get(part, "type");
            auto data_json = json_object_get(part, "data");
            if (!(json_is_string(type_json) && json_is_object(data_json))) {
                continue;
            }

            str type = json_string_value(type_json);
            if (type == "text") {
                // plain text
                auto text_json = json_object_get(data_json, "text");
                if (!json_is_string(text_json)) { continue; }
                process_part(message_escape(json_string_value(text_json)), false);
            } else {
                process_part(make_cqcode(type, data_json), true);
            }
        }
    }

    if (msg_fmt == MSG_FMT_STRING) {
        if (result_json) {
            json_decref(result_json);
        }
        result_json = json_string(result_str.c_str());
    }

    return result_json;
}
