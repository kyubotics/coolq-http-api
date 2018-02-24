#pragma once

#include "./common.h"

#include <map>

#include "./target.h"
#include "./utils/string.h"

namespace cq::message {
    /**
     * Escape special characters in the given string.
     */
    std::string escape(std::string str);

    /**
     * Unescape special characters in the given string.
     */
    std::string unescape(std::string str);

    struct Segment {
        std::string type;
        std::map<std::string, std::string> data;

        static Segment text(const std::string &text) { return {"text", {{"text", text}}}; }
        static Segment face(const int id) { return {"face", {{"id", std::to_string(id)}}}; }
        static Segment image(const std::string &file) { return {"image", {{"file", file}}}; }

        static Segment record(const std::string &file, const bool magic = false) {
            return {"record", {{"file", file}, {"magic", std::to_string(magic)}}};
        }

        static Segment at(const int64_t user_id) { return {"at", {{"qq", std::to_string(user_id)}}}; }
        static Segment rps() { return {"rps", {}}; }
        static Segment dice() { return {"dice", {}}; }
        static Segment shake() { return {"shake", {}}; }

        static Segment anonymous(const bool ignore_failure = false) {
            return {"anonymous", {{"ignore", std::to_string(ignore_failure)}}};
        }

        static Segment share(const std::string &url, const std::string &title, const std::string &content = "",
                             const std::string &image_url = "") {
            return {"share", {{"url", url}, {"title", title}, {"content", content}, {"image", image_url}}};
        }

        enum class ContactType { USER, GROUP };

        static Segment contact(const ContactType &type, const int64_t id) {
            return {
                "contact", {
                    {"type", type == ContactType::USER ? "qq" : "group"},
                    {"id", std::to_string(id)},
                }
            };
        }

        static Segment location(const double latitude, const double longitude, const std::string &title = "",
                                const std::string &content = "") {
            return {
                "location", {
                    {"lat", std::to_string(latitude)},
                    {"lon", std::to_string(longitude)},
                    {"title", title}, {"content", content}
                }
            };
        }

        static Segment music(const std::string &type, const int64_t &id) {
            return {"music", {{"type", type}, {"id", std::to_string(id)}}};
        }

        static Segment music(const std::string &url, const std::string &audio_url, const std::string &title,
                             const std::string &content = "", const std::string &image_url = "") {
            return {
                "music", {
                    {"type", "custom"}, {"url", url}, {"audio", audio_url},
                    {"title", title}, {"content", content}, {"image", image_url}
                }
            };
        }
    };

    struct Message : std::list<Segment> {
        Message() = default;

        /**
         * Split a string to a Message object.
         */
        Message(const std::string &msg_str);
        Message(const char *msg_str) : Message(std::string(msg_str)) {}

        Message(const Segment &seg) {
            this->push_back(seg);
        }

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
        int32_t send(const Target &target) const;

        /**
         * Extract and merge plain text segments in the message.
         */
        std::string extract_plain_text() const;

    private:
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
    inline int32_t send(const Target &target, const Message &msg) {
        return msg.send(target);
    }
}

namespace std {
    inline string to_string(const cq::message::Message &msg) {
        return string(msg);
    }
}
