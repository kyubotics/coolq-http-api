#pragma once

#include "./common.h"

#include "./exception.h"
#include "./utils/base64.h"
#include "./utils/binpack.h"

namespace cq {
    struct ObjectHelper {
        /**
         * Parse an object from a given base64 string.
         * This is prefered to "T::from_bytes" because it may have extra behaviors.
         */
        template <typename T>
        static T from_base64(const std::string &b64) {
            return T::from_bytes(utils::base64::decode(b64));
        }

        /**
         * Parse multiple objects from a given base64 string.
         * This is prefered to "T::from_bytes" because it may have extra behaviors.
         */
        template <typename Container>
        static Container multi_from_base64(const std::string &b64) {
            Container result;
            auto inserter = std::back_inserter(result);
            auto pack = utils::BinPack(utils::base64::decode(b64));
            try {
                const auto count = pack.pop_int<int32_t>();
                for (auto i = 0; i < count; i++) {
                    *inserter = Container::value_type::from_bytes(pack.pop_token());
                }
            } catch (exception::BytesNotEnough &) {
                throw exception::ParseError("failed to parse from bytes to multiple objects");
            }
            return result;
        }
    };

    enum class Sex { MALE = 0, FEMALE = 1, UNKNOWN = 255 };

    enum class GroupRole { MEMBER = 1, ADMIN = 2, OWNER = 3 };

    struct User {
        const static size_t MIN_SIZE = 18;

        int64_t user_id = 0;
        std::string nickname;
        Sex sex = Sex::UNKNOWN;
        int32_t age = 0;

        static User from_bytes(const std::string &bytes) {
            auto pack = utils::BinPack(bytes);
            User stranger;
            try {
                stranger.user_id = pack.pop_int<int64_t>();
                stranger.nickname = pack.pop_string();
                stranger.sex = static_cast<Sex>(pack.pop_int<int32_t>());
                stranger.age = pack.pop_int<int32_t>();
            } catch (exception::BytesNotEnough &) {
                throw exception::ParseError("failed to parse from bytes to a User object");
            }
            return stranger;
        }
    };

    struct Group {
        const static size_t MIN_SIZE = 10;

        int64_t group_id = 0;
        std::string group_name;

        static Group from_bytes(const std::string &bytes) {
            auto pack = utils::BinPack(bytes);
            Group group;
            try {
                group.group_id = pack.pop_int<int64_t>();
                group.group_name = pack.pop_string();
            } catch (exception::BytesNotEnough &) {
                throw exception::ParseError("failed to parse from bytes to a Group object");
            }
            return group;
        }
    };

    struct GroupMember : User {
        const static size_t MIN_SIZE = 58;

        int64_t group_id = 0;
        // int64_t user_id; // from User
        // std::string nickname; // from User
        std::string card;
        // Sex sex = Sex::UNKNOWN; // from User
        // int32_t age; // from User
        std::string area;
        int32_t join_time = 0;
        int32_t last_sent_time = 0;
        std::string level;
        GroupRole role = GroupRole::MEMBER;
        bool unfriendly = false;
        std::string title;
        int32_t title_expire_time = 0;
        bool card_changeable = false;

        static GroupMember from_bytes(const std::string &bytes) {
            auto pack = utils::BinPack(bytes);
            GroupMember member;
            try {
                member.group_id = pack.pop_int<int64_t>();
                member.user_id = pack.pop_int<int64_t>();
                member.nickname = pack.pop_string();
                member.card = pack.pop_string();
                member.sex = static_cast<Sex>(pack.pop_int<int32_t>());
                member.age = pack.pop_int<int32_t>();
                member.area = pack.pop_string();
                member.join_time = pack.pop_int<int32_t>();
                member.last_sent_time = pack.pop_int<int32_t>();
                member.level = pack.pop_string();
                member.role = static_cast<GroupRole>(pack.pop_int<int32_t>());
                member.unfriendly = pack.pop_bool();
                member.title = pack.pop_string();
                member.title_expire_time = pack.pop_int<int32_t>();
                member.card_changeable = pack.pop_bool();
            } catch (exception::BytesNotEnough &) {
                throw exception::ParseError("failed to parse from bytes to a GroupMember object");
            }
            return member;
        }
    };

    struct Anonymous {
        const static size_t MIN_SIZE = 12;

        int64_t id = 0;
        std::string name;
        std::string token; // binary
        std::string flag; // base64 of the whole Anonymous object

        static Anonymous from_bytes(const std::string &bytes) {
            auto pack = utils::BinPack(bytes);
            Anonymous anonymous;
            try {
                anonymous.id = pack.pop_int<int64_t>();
                anonymous.name = pack.pop_string();
                anonymous.token = pack.pop_token();
                // NOTE: we don't initialize "flag" here because it represents the
                // whole object it will be initialized in the specialized
                // ObjectHelper::from_base64 function
            } catch (exception::BytesNotEnough &) {
                throw exception::ParseError("failed to parse from bytes to an Anonymous object");
            }
            return anonymous;
        }
    };

    template <>
    inline Anonymous ObjectHelper::from_base64<Anonymous>(const std::string &b64) {
        auto anonymous = Anonymous::from_bytes(utils::base64::decode(b64));
        anonymous.flag = b64;
        return anonymous;
    }

    struct File {
        const static size_t MIN_SIZE = 20;

        std::string id;
        std::string name;
        int64_t size = 0;
        int64_t busid = 0;

        static File from_bytes(const std::string &bytes) {
            auto pack = utils::BinPack(bytes);
            File file;
            try {
                file.id = pack.pop_string();
                file.name = pack.pop_string();
                file.size = pack.pop_int<int64_t>();
                file.busid = pack.pop_int<int64_t>();
            } catch (exception::BytesNotEnough &) {
                throw exception::ParseError("failed to parse from bytes to a File object");
            }
            return file;
        }
    };
} // namespace cq
