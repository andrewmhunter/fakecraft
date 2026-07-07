#ifndef FILEIO_HPP
#define FILEIO_HPP

#include <stdio.h>
#include <stdbool.h>
#include "engine/logger.hpp"

bool openFile(FILE** file, const char* fileName, const char* mode, LogLevel logLevel);
FILE* openFileRequired(const char* fileName, const char* mode);

#endif
