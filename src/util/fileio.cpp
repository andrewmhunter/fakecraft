#include "fileio.hpp"
#include "engine/logger.hpp"
#include <errno.h>
#include <string.h>

bool openFile(FILE** file, const char* fileName, const char* mode, LogLevel level) {
    *file = fopen(fileName, mode);
    if (*file == NULL) {
        Logger::log(level, std::format("Could not open file '{}' mode '{}'. OS Error {}: {}", fileName, mode, errno, strerror(errno)));
        return false;
    }
    return true;
}

FILE* openFileRequired(const char* fileName, const char* mode) {
    FILE* file = NULL;
    openFile(&file, fileName, mode, LogLevel::fatal);
    return file;
}

