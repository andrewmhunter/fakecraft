#ifndef FAKECRAFT_LOGGER_HPP
#define FAKECRAFT_LOGGER_HPP

#include <exception>
#include <format>
#include <source_location>
#include <string_view>
#include <print>

#define RAISE_ON_FATAL 1

#include <stdarg.h>

#define ENABLE_DEBUG_ASSERT 1

// To work with the preprocessor we have to use integer literals instead
// of the TraceLogLevel which they are equal to
#define MIN_LOG_LEVEL 0

enum TraceLogLevel {
    LOG_ALL,
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL,
    LOG_NONE,
};


class FatalError : public std::exception {
private:
    std::string message;
public:
    FatalError(std::string message);
    virtual const char* what() const noexcept override;
};

class Logger {
private:
    static inline void log(std::string_view levelText, std::string_view message, TraceLogLevel level, std::source_location location = std::source_location::current()) {
        if (level < MIN_LOG_LEVEL || level < Logger::logLevel) {
            return;
        }

        std::println("{} {}:{}: {}\x1b[0m: {}", location.function_name(), location.file_name(), location.line(), levelText, message);
    }

public:
    static TraceLogLevel logLevel;

    static inline void trace(std::string_view message, std::source_location location = std::source_location::current()) {
        Logger::log("\x1b[90mTRACE", message, LOG_TRACE, location);
    }

    static inline void debug(std::string_view message, std::source_location location = std::source_location::current()) {
        Logger::log("\x1b[32mDEBUG", message, LOG_DEBUG, location);
    }

    static inline void info(std::string_view message, std::source_location location = std::source_location::current()) {
        Logger::log("\x1b[36mINFO", message, LOG_INFO, location);
    }

    static inline void warning(std::string_view message, std::source_location location = std::source_location::current()) {
        Logger::log("\x1b[35mWARN", message, LOG_WARNING, location);
    }

    static inline void error(std::string_view message, std::source_location location = std::source_location::current()) {
        Logger::log("\x1b[31mERROR", message, LOG_ERROR, location);
    }

    [[noreturn]]
    static inline void fatal(std::string_view message, std::source_location location = std::source_location::current()) {
        Logger::log("\x1b[31mFATAL", message, LOG_FATAL, location);
        throw FatalError{std::format("{}: {}", location.function_name(), message)};
    }

    static inline void log(TraceLogLevel level, std::string_view message, std::source_location location = std::source_location::current()) {
        switch (level) {
            case LOG_TRACE:
                trace(message, location);
                break;
            case LOG_DEBUG:
                debug(message, location);
                break;
            case LOG_INFO:
                info(message, location);
                break;
            case LOG_WARNING:
                warning(message, location);
                break;
            case LOG_ERROR:
                error(message, location);
                break;
            case LOG_FATAL:
                fatal(message, location);
                break;
            case LOG_NONE:
                break;
            default:
                error(std::format("Invalid log level for message \"{}\"", message), location);
                break;
        }
    }

    static inline void assertion(bool assertion, std::string_view message, std::source_location location = std::source_location::current()) {
        if (!assertion) {
            Logger::fatal(message, location);
        }
    }

    static inline void assertion(bool assertion, std::source_location location = std::source_location::current()) {
        Logger::assertion(assertion, "Assertion failed", location);
    }

    static inline void debug_assertion(bool assertion, std::string_view message, std::source_location location = std::source_location::current()) {
        if (ENABLE_DEBUG_ASSERT) {
            Logger::assertion(assertion, message, location);
        }
    }

    static inline void debug_assertion(bool assertion, std::source_location location = std::source_location::current()) {
        Logger::debug_assertion(assertion, "Assertion failed", location);
    }
};

#endif
