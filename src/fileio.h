#ifndef FILEIO_H
#define FILEIO_H

#include <stdio.h>
#include <raylib.h>
#include <stdbool.h>

bool openFile(FILE** file, const char* fileName, const char* mode, TraceLogLevel logLevel);
FILE* openFileRequired(const char* fileName, const char* mode);

#endif
