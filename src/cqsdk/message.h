#pragma once

#include "./common.h"

#include <map>

#include "./target.h"

namespace cq::message {
    std::string escape(std::string msg);
    std::string unescape(std::string msg);

    struct Segment {
        std::string type;
        std::map<std::string, std::string> data;
    };

    struct Message : std::list<Segment> {
        Message() = default;
        Message(const std::string &msg_str);
        Message(const char *msg_str) : Message(std::string(msg_str)) {}

        operator std::string() const;

        Message &operator+=(const Message &other) {
            this->insert(this->end(), other.begin(), other.end());
            this->reduce();
            return *this;
        }

        Message &operator+=(const std::string &other) {
            return this->operator+=(Message(other));
        }

        Message operator+(const Message &other) const {
            auto result = *this;
            result += other; // use operator+=
            return result;
        }

        Message operator+(const std::string &other) const {
            return this->operator+(Message(other));
        }

        int32_t send(const Target &target) const;

    private:
        void reduce();
    };

    inline Message operator+(const std::string &lhs, const Message &rhs) {
        return Message(lhs) + rhs;
    }
}
