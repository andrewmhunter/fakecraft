#include "logger.hpp"

FatalError::FatalError(std::string message) : message{message} {}

const char* FatalError::what() const noexcept {
    return message.c_str();
}


LogLevel Logger::logLevel = LogLevel::info;
