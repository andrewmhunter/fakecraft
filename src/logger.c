#include <stdio.h>
#include <signal.h>
#include "logger.h"
#include "util.h"

static TraceLogLevel logLevel = LOG_INFO;

TraceLogLevel setLogLevel(TraceLogLevel level) {
    level = MIN(level, LOG_FATAL);

    TraceLogLevel oldLevel = logLevel;
    logLevel = level;
    return oldLevel;
}

void printLog(TraceLogLevel level, const char* where, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintLog(level, where, format, args);
    va_end(args);
}

void vprintLog(TraceLogLevel level, const char* where, const char* format, va_list args) {
    static const char* const levelText[LOG_NONE] = {
        [LOG_ALL] = "ALL",
        [LOG_TRACE] = "\x1b[90mTRACE",
        [LOG_DEBUG] = "\x1b[32mDEBUG",
        [LOG_INFO] = "\x1b[36mINFO",
        [LOG_WARNING] = "\x1b[35mWARN",
        [LOG_ERROR] = "\x1b[31mERROR",
        [LOG_FATAL] = "\x1b[31mFATAL",
    };

    if (level < logLevel) {
        return;
    }

    fprintf(stderr, "%s\x1b[0m: %s: ", levelText[level], where);
    vfprintf(stderr, format, args);
    fputc('\n', stderr);

    if (level == LOG_FATAL) {
        raise(SIGABRT);
    }
}

void raylibLogCallback(int level, const char* format, va_list args) {
    vprintLog(level, "raylib", format, args);
}

