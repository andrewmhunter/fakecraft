#include "fileio.hpp"
#include "logger.hpp"
#include <errno.h>
#include <string.h>

bool openFile(FILE** file, const char* fileName, const char* mode, TraceLogLevel level) {
    *file = fopen(fileName, mode);
    if (*file == NULL) {
        Logger::log(level, std::format("Could not open file '{}' mode '{}'. OS Error {}: {}", fileName, mode, errno, strerror(errno)));
        return false;
    }
    return true;
}

FILE* openFileRequired(const char* fileName, const char* mode) {
    FILE* file = NULL;
    openFile(&file, fileName, mode, LOG_FATAL);
    return file;
}

