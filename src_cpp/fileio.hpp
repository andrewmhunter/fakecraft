#ifndef FILEIO_HPP
#define FILEIO_HPP

#include <stdio.h>
#include <stdbool.h>
#include "logger.hpp"

bool openFile(FILE** file, const char* fileName, const char* mode, TraceLogLevel logLevel);
FILE* openFileRequired(const char* fileName, const char* mode);

#endif
