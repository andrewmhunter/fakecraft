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

enum class LogLevel {
    all,
    trace,
    debug,
    info,
    warning,
    error,
    fatal,
    none,
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
    static inline void log(std::string_view levelText, std::string_view message, LogLevel level, std::source_location location = std::source_location::current()) {
        if (static_cast<int>(level) < MIN_LOG_LEVEL || level < Logger::logLevel) {
            return;
        }

        std::println("{} {}:{}: {}\x1b[0m: {}", location.function_name(), location.file_name(), location.line(), levelText, message);
    }

public:
    static LogLevel logLevel;

    static inline void trace(std::string_view message, std::source_location location = std::source_location::current()) {
        Logger::log("\x1b[90mTRACE", message, LogLevel::trace, location);
    }

    static inline void debug(std::string_view message, std::source_location location = std::source_location::current()) {
        Logger::log("\x1b[32mDEBUG", message, LogLevel::debug, location);
    }

    static inline void info(std::string_view message, std::source_location location = std::source_location::current()) {
        Logger::log("\x1b[36mINFO", message, LogLevel::info, location);
    }

    static inline void warning(std::string_view message, std::source_location location = std::source_location::current()) {
        Logger::log("\x1b[35mWARN", message, LogLevel::warning, location);
    }

    static inline void error(std::string_view message, std::source_location location = std::source_location::current()) {
        Logger::log("\x1b[31mERROR", message, LogLevel::error, location);
    }

    [[noreturn]]
    static inline void fatal(std::string_view message, std::source_location location = std::source_location::current()) {
        Logger::log("\x1b[31mFATAL", message, LogLevel::fatal, location);
        throw FatalError{std::format("{}: {}", location.function_name(), message)};
    }

    static inline void log(LogLevel level, std::string_view message, std::source_location location = std::source_location::current()) {
        switch (level) {
            case LogLevel::trace:
                trace(message, location);
                break;
            case LogLevel::debug:
                debug(message, location);
                break;
            case LogLevel::info:
                info(message, location);
                break;
            case LogLevel::warning:
                warning(message, location);
                break;
            case LogLevel::error:
                error(message, location);
                break;
            case LogLevel::fatal:
                fatal(message, location);
                break;
            case LogLevel::none:
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
