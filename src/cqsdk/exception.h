#pragma once

#include <exception>

namespace cq::exception {
    struct Exception : std::exception {
        Exception(const char *what_arg) : exception(what_arg) {}
        Exception(const std::string &what_arg) : exception(what_arg.c_str()) {}
    };

    /**
     * Exception that caused by the user of a module.
     */
    struct LogicError : Exception {
        using Exception::Exception;
    };

    /**
     * Exception that caused by outside user-uncontrollable factors.
     */
    struct RuntimeError : Exception {
        using Exception::Exception;
    };

    /**
     * Exception that occurs when parsing objects from serialized data.
     */
    struct ParseError : RuntimeError {
        using RuntimeError::RuntimeError;
    };
} // namespace cq::exception
