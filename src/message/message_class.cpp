// 
// message_class.cpp : Implement Message class.
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

#include "./message_class.h"

#include "app.h"

using namespace std;

const string Message::Formats::STRING = "string";
const string Message::Formats::ARRAY = "array";

string Message::escape(string msg) {
    string_replace(msg, "&", "&amp;");
    string_replace(msg, "[", "&#91;");
    string_replace(msg, "]", "&#93;");
    string_replace(msg, ",", "&#44;");
    return msg;
}

string Message::unescape(string msg) {
    string_replace(msg, "&#91;", "[");
    string_replace(msg, "&#93;", "]");
    string_replace(msg, "&#44;", ",");
    string_replace(msg, "&amp;", "&");
    return msg;
}

/**
 * Implement a FSM manually,
 * because the regex lib of VC++ will throw stack overflow in some cases.
 */
static list<Message::Segment> split(const string &raw_msg) {
    list<Message::Segment> segments;
    const static auto TEXT = 0;
    const static auto FUNCTION_NAME = 1;
    const static auto PARAMS = 2;
    auto state = TEXT;
    const auto end = raw_msg.cend();
    stringstream text_s, function_name_s, params_s;
    auto curr_cq_start = end;
    for (auto it = raw_msg.cbegin(); it != end; ++it) {
        const auto curr = *it;
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
                    seg.data[key] = Message::unescape(value);
                }

                if (text_s.rdbuf()->in_avail()) {
                    // there is a text segment before this CQ code
                    segments.push_back(Message::Segment{"text", {{"text", Message::unescape(text_s.str())}}});
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
            segments.push_back(Message::Segment{"text", {{"text", Message::unescape(text_s.str())}}});
        }
    default: break;
    }

    return segments;
}

static string merge(const list<Message::Segment> &segments) {
    stringstream ss;
    for (auto seg : segments) {
        if (seg.type.empty()) {
            continue;
        }
        if (seg.type == "text") {
            if (const auto it = seg.data.find("text"); it != seg.data.end()) {
                ss << Message::escape((*it).second);
            }
        } else {
            ss << "[CQ:" << seg.type;
            for (const auto item : seg.data) {
                ss << "," << item.first << "=" << Message::escape(item.second);
            }
            ss << "]";
        }
    }
    return ss.str();
}

/**
 * Merge adjacent "text" segments.
 */
static void reduce(list<Message::Segment> &segments) {
    if (segments.empty()) {
        return;
    }

    auto last_seg_it = segments.begin();
    for (auto it = segments.begin(); ++it != segments.end();) {
        if (it->type == "text" && last_seg_it->type == "text"
            && it->data.find("text") != it->data.end()
            && last_seg_it->data.find("text") != last_seg_it->data.end()) {
            // found adjacent "text" segments
            last_seg_it->data["text"] += it->data["text"];
            // remove the current element and continue
            segments.erase(it);
            it = last_seg_it;
        } else {
            last_seg_it = it;
        }
    }
}

Message::Message(const string &msg_str) {
    this->segments_ = split(msg_str);
}

Message::Message(const json &msg_json) {
    if (msg_json.is_string()) {
        this->segments_ = split(msg_json.get<string>());
    } else if (msg_json.is_array()) {
        for (auto seg : msg_json) {
            if (seg.is_object()) {
                try {
                    this->segments_.push_back(seg.get<Segment>());
                } catch (...) {
                    // it's not a valid message segment, skip it
                }
            }
        }
    }
}

string Message::process_outward() const {
    list<Segment> segments;
    for (const auto &seg : this->segments_) {
        segments.push_back(seg.enhanced(Directions::OUTWARD));
    }
    return merge(segments);
}

json Message::process_inward(optional<Format> fmt) const {
    if (!fmt) {
        fmt = config.post_message_format;
    }

    list<Segment> segments;
    for (const auto &seg : this->segments_) {
        segments.push_back(seg.enhanced(Directions::INWARD));
    }

    if (fmt == Formats::STRING) {
        return merge(segments);
    }

    if (fmt == Formats::ARRAY) {
        reduce(segments);
        return segments;
    }

    return nullptr;
}

void to_json(json &j, const Message::Segment &seg) {
    j = json{
        {"type", seg.type},
        {"data", seg.data}
    };
}

void from_json(const json &j, Message::Segment &seg) {
    seg.type = j.at("type").get<string>();
    auto data = j.at("data");
    if (data.is_object()) {
        for (auto it = data.begin(); it != data.end(); ++it) {
            if (it.value().is_string()) {
                seg.data[it.key()] = it.value().get<string>();
            }
        }
    }
}
