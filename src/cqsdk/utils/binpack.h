#pragma once

#include "../common.h"

#include "../exception.h"
#include "./string.h"

namespace cq::exception {
    struct BytesNotEnough : LogicError {
        BytesNotEnough(const size_t have, const size_t needed)
            : LogicError("there aren't enough bytes remained (have " + std::to_string(have) + ", but "
                         + std::to_string(needed) + " are/is needed)") {}
    };
} // namespace cq::exception

namespace cq::utils {
    class BinPack {
    public:
        BinPack() : bytes_(""), curr_(0) {}
        explicit BinPack(const std::string &b) : bytes_(b), curr_(0) {}

        size_t size() const noexcept { return bytes_.size() - curr_; }
        bool empty() const noexcept { return size() == 0; }

        template <typename IntType>
        IntType pop_int() noexcept(false) {
            constexpr auto size = sizeof(IntType);
            check_enough(size);

            auto s = bytes_.substr(curr_, size);
            curr_ += size;
            std::reverse(s.begin(), s.end());

            IntType result;
            memcpy(static_cast<void *>(&result), s.data(), size);
            return result;
        }

        std::string pop_string() noexcept(false) {
            const auto len = pop_int<int16_t>();
            if (len == 0) {
                return std::string();
            }
            check_enough(len);
            auto result = string_from_coolq(bytes_.substr(curr_, len));
            curr_ += len;
            return result;
        }

        std::string pop_bytes(const size_t len) noexcept(false) {
            auto result = bytes_.substr(curr_, len);
            curr_ += len;
            return result;
        }

        std::string pop_token() noexcept(false) { return pop_bytes(pop_int<int16_t>()); }

        bool pop_bool() noexcept(false) { return static_cast<bool>(pop_int<int32_t>()); }

    private:
        std::string bytes_;
        size_t curr_;

        void check_enough(const size_t needed) const noexcept(false) {
            if (size() < needed) {
                throw exception::BytesNotEnough(size(), needed);
            }
        }
    };
} // namespace cq::utils
