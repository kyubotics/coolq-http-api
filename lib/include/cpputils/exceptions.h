#pragma once

#include <type_traits>

#include "str.h"

namespace rc {

    class Exception {
    public:
        str msg;

        Exception() { this->msg = "unknown reason"; }
        explicit Exception(const str &msg) { this->msg = msg; }

        template <typename E, typename = typename std::enable_if<std::is_base_of<Exception, E>::value, E>::type>
        friend std::ostream &operator<<(std::ostream &os, const E &obj) {
            os << str(typeid(obj).name()).split(" ", 1)[1] << ": " << obj.msg;
            return os;
        }
    };

    class IndexError : public Exception {
        using Exception::Exception;
    };

    class KeyError : public Exception {
        using Exception::Exception;
    };

    class ValueError : public Exception {
        using Exception::Exception;
    };

    class OutOfRangeError : public Exception {
        using Exception::Exception;
    };

    class TypeError : public Exception {
        using Exception::Exception;
    };

    class IOError : public Exception {
        using Exception::Exception;
    };
}
