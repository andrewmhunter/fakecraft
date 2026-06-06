#include "fileio.hpp"
#include "logger.hpp"
#include <errno.h>
#include <string.h>

bool openFile(FILE** file, const char* fileName, const char* mode, TraceLogLevel level) {
    *file = fopen(fileName, mode);
    if (*file == NULL) {
        LOG(level, "Could not open file '%s' mode '%s'. OS Error %d: %s", fileName, mode, errno, strerror(errno));
        return false;
    }
    return true;
}

FILE* openFileRequired(const char* fileName, const char* mode) {
    FILE* file = NULL;
    openFile(&file, fileName, mode, LOG_FATAL);
    return file;
}

