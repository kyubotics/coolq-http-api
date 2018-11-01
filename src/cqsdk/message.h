#pragma once

#include "./common.h"

#include "./target.h"
#include "./utils/string.h"

namespace cq::message {
    /**
     * Escape special characters in the given string.
     */
    std::string escape(std::string str, bool escape_comma = true);

    /**
     * Unescape special characters in the given string.
     */
    std::string unescape(std::string str);

    struct MessageSegment {
        std::string type;
        std::map<std::string, std::string> data;

        static MessageSegment text(const std::string &text) { return {"text", {{"text", text}}}; }
        static MessageSegment emoji(const uint32_t id) { return {"emoji", {{"id", std::to_string(id)}}}; }
        static MessageSegment face(const int id) { return {"face", {{"id", std::to_string(id)}}}; }
        static MessageSegment image(const std::string &file) { return {"image", {{"file", file}}}; }

        static MessageSegment record(const std::string &file, const bool magic = false) {
            return {"record", {{"file", file}, {"magic", std::to_string(magic)}}};
        }

        static MessageSegment at(const int64_t user_id) { return {"at", {{"qq", std::to_string(user_id)}}}; }
        static MessageSegment rps() { return {"rps", {}}; }
        static MessageSegment dice() { return {"dice", {}}; }
        static MessageSegment shake() { return {"shake", {}}; }

        static MessageSegment anonymous(const bool ignore_failure = false) {
            return {"anonymous", {{"ignore", std::to_string(ignore_failure)}}};
        }

        static MessageSegment share(const std::string &url, const std::string &title, const std::string &content = "",
                                    const std::string &image_url = "") {
            return {"share", {{"url", url}, {"title", title}, {"content", content}, {"image", image_url}}};
        }

        enum class ContactType { USER, GROUP };

        static MessageSegment contact(const ContactType &type, const int64_t id) {
            return {
                "contact",
                {
                    {"type", type == ContactType::USER ? "qq" : "group"},
                    {"id", std::to_string(id)},
                },
            };
        }

        static MessageSegment location(const double latitude, const double longitude, const std::string &title = "",
                                       const std::string &content = "") {
            return {
                "location",
                {
                    {"lat", std::to_string(latitude)},
                    {"lon", std::to_string(longitude)},
                    {"title", title},
                    {"content", content},
                },
            };
        }

        static MessageSegment music(const std::string &type, const int64_t &id) {
            return {"music", {{"type", type}, {"id", std::to_string(id)}}};
        }

        static MessageSegment music(const std::string &url, const std::string &audio_url, const std::string &title,
                                    const std::string &content = "", const std::string &image_url = "") {
            return {
                "music",
                {
                    {"type", "custom"},
                    {"url", url},
                    {"audio", audio_url},
                    {"title", title},
                    {"content", content},
                    {"image", image_url},
                },
            };
        }
    };

    struct Message : std::list<MessageSegment> {
        Message() = default;

        /**
         * Split a string to a Message object.
         */
        Message(const std::string &msg_str);

        Message(const char *msg_str) : Message(std::string(msg_str)) {}

        Message(const MessageSegment &seg) { this->push_back(seg); }

        /**
         * Merge all segments to a string.
         */
        operator std::string() const;

        Message &operator+=(const Message &other) {
            this->insert(this->end(), other.begin(), other.end());
            this->reduce();
            return *this;
        }

        template <typename T>
        Message &operator+=(const T &other) {
            return this->operator+=(Message(other));
        }

        Message operator+(const Message &other) const {
            auto result = *this;
            result += other; // use operator+=
            return result;
        }

        template <typename T>
        Message operator+(const T &other) const {
            return this->operator+(Message(other));
        }

        /**
         * Send the message to a given target.
         */
        int64_t send(const Target &target) const;

        /**
         * Extract and merge plain text segments in the message.
         */
        std::string extract_plain_text() const;

        std::list<MessageSegment> &segments() { return *this; }
        const std::list<MessageSegment> &segments() const { return *this; }

        /**
         * Merge adjacent "text" segments.
         */
        void reduce();
    };

    template <typename T>
    Message operator+(const T &lhs, const Message &rhs) {
        return Message(lhs) + rhs;
    }

    /**
     * Send a message to the given target.
     * Thanks to the auto type conversion, a Segment object can also be passed in.
     */
    inline int64_t send(const Target &target, const Message &msg) { return msg.send(target); }
} // namespace cq::message

namespace std {
    inline string to_string(const cq::message::Message &msg) { return string(msg); }
} // namespace std
