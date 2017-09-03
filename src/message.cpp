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

#include "helpers.h"

using namespace std;

str Message::escape(const str &msg) {
    string tmp = msg;
    string_replace(tmp, "&", "&amp;");
    string_replace(tmp, "[", "&#91;");
    string_replace(tmp, "]", "&#93;");
    string_replace(tmp, ",", "&#44;");
    return tmp;
}

str Message::unescape(const str &msg) {
    string tmp = msg;
    string_replace(tmp, "&#91;", "[");
    string_replace(tmp, "&#93;", "]");
    string_replace(tmp, "&#44;", ",");
    string_replace(tmp, "&amp;", "&");
    return tmp;
}

/**
 * Implement a FSM manually,
 * because the regex lib of VC++ will throw stack overflow in some cases.
 */
//static vector<Message::Segment> split(const str &raw_msg) {
//    vector<Message::Segment> segments;
//    const static auto TEXT = 0;
//    const static auto FUNCTION_NAME = 1;
//    const static auto PARAMS = 2;
//    auto state = TEXT;
//    auto end = raw_msg.c_end();
//    stringstream text_s, function_name_s, params_s;
//    auto curr_cq_start = end;
//    for (auto it = raw_msg.c_begin(); it != end; ++it) {
//        auto curr = *it;
//        switch (state) {
//        case TEXT: {
//        text:
//            if (curr == '[' && end - it >= 5 /* [CQ:a] at least 5 chars behind */
//                && *(it + 1) == 'C' && *(it + 2) == 'Q' && *(it + 3) == ':') {
//                state = FUNCTION_NAME;
//                curr_cq_start = it;
//                it += 3;
//            } else {
//                text_s << curr;
//            }
//            break;
//        }
//        case FUNCTION_NAME: {
//            if (curr >= 'A' && curr <= 'Z'
//                || curr >= 'a' && curr <= 'z'
//                || curr >= '0' && curr <= '9') {
//                function_name_s << curr;
//            } else if (curr == ',') {
//                // function name out, params in
//                state = PARAMS;
//            } else if (curr == ']') {
//                // CQ code end, with no params
//                goto params;
//            } else {
//                // unrecognized character
//                text_s << string(curr_cq_start, it); // mark as text
//                curr_cq_start = end;
//                function_name_s = stringstream();
//                params_s = stringstream();
//                state = TEXT;
//                // because the current char may be '[', so we goto text part
//                goto text;
//            }
//            break;
//        }
//        case PARAMS: {
//        params:
//            if (curr == ']') {
//                // CQ code end
//                Message::Segment seg;
//
//                seg.type = function_name_s.str();
//                while (params_s.rdbuf()->in_avail()) {
//                    // split key and value
//                    string key, value;
//                    getline(params_s, key, '=');
//                    getline(params_s, value, ',');
//                    seg.data[key] = Message::unescape(value);
//                }
//
//                if (text_s.rdbuf()->in_avail()) {
//                    // there is a text segment before this CQ code
//                    segments.push_back(Message::Segment{"text", {{"text", Message::unescape(text_s.str())}}});
//                    text_s = stringstream();
//                }
//
//                segments.push_back(seg);
//                curr_cq_start = end;
//                text_s = stringstream();
//                function_name_s = stringstream();
//                params_s = stringstream();
//                state = TEXT;
//            } else {
//                params_s << curr;
//            }
//        }
//        default: break;
//        }
//    }
//
//    // iterator end, there may be some rest of message we haven't put into segments
//    switch (state) {
//    case FUNCTION_NAME:
//    case PARAMS:
//        // we are in CQ code, but it ended with no ']', so it's a text segment
//        text_s << string(curr_cq_start, end);
//        // should fall through
//    case TEXT:
//        if (text_s.rdbuf()->in_avail()) {
//            segments.push_back(Message::Segment{"text", {{"text", Message::unescape(text_s.str())}}});
//        }
//    default: break;
//    }
//
//    return segments;
//}

const static string CQ_START_FLAG = "[CQ:";

static vector<Message::Segment> split(const str &raw_msg_str) {
    // to avoid potential problems, we use "string" instead of "str" here
    auto raw_msg = raw_msg_str.to_bytes();

    vector<Message::Segment> segments;

    stringstream text_buffer_s;
    auto flush_text_buffer = [&]() {
                auto text = text_buffer_s.str();
                if (text.length() > 0) {
                    Message::Segment segment{"text", {{str("text"), Message::unescape(text)}}};
                    segments.push_back(segment);
                    text_buffer_s = stringstream();
                }
            };

    auto it = raw_msg.begin();
    while (it != raw_msg.end()) {
        auto curr_cq_start_it = search(it, raw_msg.end(), CQ_START_FLAG.begin(), CQ_START_FLAG.end());
        if (curr_cq_start_it == raw_msg.end()) {
            // there is no CQ code left
            text_buffer_s << string(it, curr_cq_start_it);
            it = curr_cq_start_it;
        } else {
            auto temp_it = curr_cq_start_it + CQ_START_FLAG.length();

            stringstream function_name_s, param_s;
            vector<string> params;
            auto in_params = false; // initially false, which means in function name
            auto succeeded = false;

            auto flush_param = [&]() {
                        auto p = param_s.str();
                        if (p.length() > 0) {
                            params.push_back(param_s.str());
                            param_s = stringstream();
                        }
                    };

            while (temp_it != raw_msg.end()) {
                // scanning the current possible CQ code
                auto should_stop = false;
                auto ch = *(temp_it++);
                switch (ch) {
                case ',': {
                    if (!in_params) {
                        // we were reading function name just now, and now we are switching to params
                        in_params = true;
                    } else {
                        flush_param();
                    }
                    break;
                }
                case ']': {
                    // CQ code finished
                    flush_param();
                    succeeded = true;
                    break;
                }
                case '[': {
                    // '[' should never show up inside a CQ code, so we will stop parsing the current one
                    should_stop = true;
                    // temp_it now is the next position of '[', we move it back
                    temp_it -= 1;
                    break;
                }
                default: {
                    // other characters, we read them in
                    if (in_params) {
                        // in params
                        param_s << ch;
                    } else {
                        // in function name
                        function_name_s << ch;
                    }
                    break;
                }
                }

                if (succeeded || should_stop) {
                    break;
                }
            }

            if (succeeded) {
                // handle the text segment (if any) before the current CQ code
                text_buffer_s << string(it, curr_cq_start_it);
                flush_text_buffer();

                Message::Segment segment;

                segment.type = str(function_name_s.str());
                function_name_s = stringstream();

                for (auto param_text : params) {
                    auto pos = param_text.find('=');
                    auto key = param_text.substr(0, pos);
                    string value;
                    if (pos != string::npos && pos + 1 < param_text.length()) {
                        value = param_text.substr(pos + 1);
                    }
                    if (key.length() > 0) {
                        segment.data[str(key)] = Message::unescape(value);
                    }
                }

                segments.push_back(segment);
            } else {
                text_buffer_s << string(it, temp_it);
            }
            it = temp_it;
        }
    }

    // flush the possibly remained text
    flush_text_buffer();

    return segments;
}

static str merge(vector<Message::Segment> segments) {
    stringstream ss;
    for (auto seg : segments) {
        if (seg.type == "") {
            continue;
        }
        if (seg.type == "text") {
            auto it = seg.data.find("text");
            if (it != seg.data.end()) {
                ss << Message::escape((*it).second);
            }
        } else {
            ss << "[CQ:" << seg.type;
            for (auto item : seg.data) {
                ss << "," << item.first << "=" << Message::escape(item.second);
            }
            ss << "]";
        }
    }
    return ss.str();
}

Message::Message(const str &msg_str) {
    this->segments_ = split(msg_str);
}

Message::Message(json_t *msg_json) {
    if (json_is_string(msg_json)) {
        *this = Message(str(json_string_value(msg_json)));
    } else if (json_is_array(msg_json)) {
        size_t i;
        json_t *seg;
        json_array_foreach(msg_json, i, seg) {
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

str Message::process_outcoming() const {
    vector<Segment> segments;
    for (auto seg : this->segments_) {
        segments.push_back(seg.enhanced(Segment::ENHANCE_OUTCOMING));
    }
    return merge(segments);
}

json_t *Message::process_incoming(str msg_fmt) const {
    if (!msg_fmt) {
        msg_fmt = CQ->config.post_message_format;
    }

    vector<Segment> segments;
    for (auto seg : this->segments_) {
        segments.push_back(seg.enhanced(Segment::ENHANCE_INCOMING));
    }

    if (msg_fmt == MSG_FMT_STRING) {
        return json_string(merge(segments).c_str());
    }

    if (msg_fmt == MSG_FMT_ARRAY) {
        auto result = json_array();
        for (auto seg : segments) {
            auto seg_json = json_object();
            json_object_set_new(seg_json, "type", json_string(seg.type.c_str()));
            auto data_json = json_object();
            for (auto param : seg.data) {
                json_object_set_new(data_json, param.first.c_str(), json_string(param.second.c_str()));
            }
            json_object_set(seg_json, "data", data_json);
            json_decref(data_json);

            json_array_append(result, seg_json);
            json_decref(seg_json);
        }
        return result;
    }

    return nullptr;
}
