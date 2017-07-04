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

/**
 * Implement a FSM manually,
 * because the regex lib of VC++ will throw stack overflow in some cases.
 */
static vector<Message::Segment> parse(const str &raw_msg) {
    vector<Message::Segment> segments;
    const static auto TEXT = 0;
    const static auto FUNCTION_NAME = 1;
    const static auto PARAMS = 2;
    auto state = TEXT;
    auto end = raw_msg.c_end();
    stringstream text_s, function_name_s, params_s;
    auto curr_cq_start = end;
    for (auto it = raw_msg.c_begin(); it != end; ++it) {
        auto curr = *it;
        switch (state) {
        case TEXT: {
        text:
            if (curr == '[' && end - it >= 5 /* [CQ:a] at least 5 chars behind */
                && *(it + 1) == 'C' && *(it + 2) == 'Q' && *(it + 3) == ':') {
                state = FUNCTION_NAME;
                curr_cq_start = it;
                it += 3;
            } else {
                text_s << curr;
            }
            break;
        }
        case FUNCTION_NAME: {
            if (curr >= 'A' && curr <= 'Z'
                || curr >= 'a' && curr <= 'z'
                || curr >= '0' && curr <= '9') {
                function_name_s << curr;
            } else if (curr == ',') {
                // function name out, params in
                state = PARAMS;
            } else if (curr == ']') {
                // CQ code end, with no params
                goto params;
            } else {
                // unrecognized character
                text_s << string(curr_cq_start, it); // mark as text
                curr_cq_start = end;
                function_name_s = stringstream();
                params_s = stringstream();
                state = TEXT;
                // because the current char may be '[', so we goto text part
                goto text;
            }
            break;
        }
        case PARAMS: {
        params:
            if (curr == ']') {
                // CQ code end
                Message::Segment seg;

                seg.type = function_name_s.str();
                while (params_s.rdbuf()->in_avail()) {
                    // split key and value
                    string key, value;
                    getline(params_s, key, '=');
                    getline(params_s, value, ',');
                    seg.data[key] = message_unescape(value);
                }

                if (text_s.rdbuf()->in_avail()) {
                    // there is a text segment before this CQ code
                    segments.push_back(Message::Segment{"text",{{"text", message_unescape(text_s.str())}}});
                    text_s = stringstream();
                }

                segments.push_back(seg);
                curr_cq_start = end;
                text_s = stringstream();
                function_name_s = stringstream();
                params_s = stringstream();
                state = TEXT;
            } else {
                params_s << curr;
            }
        }
        default: break;
        }
    }

    // iterator end, there may be some rest of message we haven't put into segments
    switch (state) {
    case FUNCTION_NAME:
    case PARAMS:
        // we are in CQ code, but it ended with no ']', so it's a text segment
        text_s << string(curr_cq_start, end);
        // should fall through
    case TEXT:
        if (text_s.rdbuf()->in_avail()) {
            segments.push_back(Message::Segment{"text",{{"text", text_s.str()}}});
        }
    default: break;
    }

    return segments;
}

Message::Message(const str &msg_str) : msg_str_(msg_str), msg_json_(nullptr) {
    this->segments_ = parse(msg_str);
}

Message::Message(json_t *msg_json) : msg_json_(msg_json) {
    json_incref(msg_json);
    if (json_is_string(msg_json)) {
        this->segments_ = parse(str(json_string_value(msg_json)));
    } else if (json_is_array(msg_json)) {
        size_t i;
        json_t *seg;
        json_array_foreach(this->msg_json_, i, seg) {
            if (!json_is_object(seg)) {
                continue;
            }
            auto type_json = json_object_get(seg, "type");
            auto data_json = json_object_get(seg, "data");
            if (!json_is_string(type_json) 
                || data_json && !json_is_null(data_json) && !json_is_object(data_json)) {
                // type is not a string,
                // or data is neither null nor an object,
                // we think this segment is invalid
                continue;
            }

            str type = json_string_value(type_json);
            map<str, str> data;
            if (json_is_object(data_json)) {
                const char *key;
                json_t *value;
                json_object_foreach(data_json, key, value) {
                    if (!key || strlen(key) == 0) {
                        continue;
                    }
                    str v;
                    if (json_is_string(value)) {
                        v = json_string_value(value);
                    }
                    data[key] = v;
                }
            }
            this->segments_.push_back(Segment{type, data});
        }
    }
}

Message::Message(const Message &other) {
    this->msg_str_ = other.msg_str_;
    this->msg_json_ = other.msg_json_;
    json_incref(this->msg_json_);
    this->segments_ = other.segments_;
}

Message::~Message() {
    if (this->msg_json_) {
        json_decref(this->msg_json_);
    }
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
