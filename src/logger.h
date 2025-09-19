#ifndef FAKECRAFT_LOGGER_H
#define FAKECRAFT_LOGGER_H

#include <raylib.h>
#include <stdarg.h>

#define ENABLE_DEBUG_ASSERT 1

// To work with the preprocessor we have to use integer literals instead
// of the TraceLogLevel which they are equal to
#define MIN_LOG_LEVEL 3

#if 1 >= MIN_LOG_LEVEL
#define TRACE(FORMAT, ...) printLog(LOG_TRACE, __FUNCTION__, FORMAT __VA_OPT__(,) __VA_ARGS__)
#else
#define TRACE(FORMAT, ...)
#endif

#if 2 >= MIN_LOG_LEVEL
#define DEBUG(FORMAT, ...) printLog(LOG_DEBUG, __FUNCTION__, FORMAT __VA_OPT__(,) __VA_ARGS__)
#else
#define DEBUG(FORMAT, ...)
#endif

#if 3 >= MIN_LOG_LEVEL
#define INFO(FORMAT, ...) printLog(LOG_INFO, __FUNCTION__, FORMAT __VA_OPT__(,) __VA_ARGS__)
#else
#define INFO(FORMAT, ...)
#endif

#if 4 >= MIN_LOG_LEVEL
#define WARN(FORMAT, ...) printLog(LOG_WARNING, __FUNCTION__, FORMAT __VA_OPT__(,) __VA_ARGS__)
#else
#define WARN(FORMAT, ...)
#endif

#if 5 >= MIN_LOG_LEVEL
#define ERROR(FORMAT, ...) printLog(LOG_ERROR, __FUNCTION__, FORMAT __VA_OPT__(,) __VA_ARGS__)
#else
#define ERROR(FORMAT, ...)
#endif

#define FATAL(FORMAT, ...) printLog(LOG_FATAL, __FUNCTION__, FORMAT __VA_OPT__(,) __VA_ARGS__)

#define STRINGIFY(X) STRINGIFY_2(X)
#define STRINGIFY_2(X) #X

#define ASSERT(COND) assertion((COND), __FILE__ ":" STRINGIFY(__LINE__), #COND)
#define ASSERTF(COND, FORMAT, ...) assertionf((COND), __FILE__ ":" STRINGIFY(__LINE__), #COND, FORMAT __VA_OPT__(,) __VA_ARGS__)

#if ENABLE_DEBUG_ASSERT
#define DEBUG_ASSERT(COND) ASSERT(COND)
#define DEBUG_ASSERTF(COND, FORMAT, ...) ASSERTF(COND, FORMAT __VA_OPT__(,) __VA_ARGS__)
#else
#define DEBUG_ASSERT(COND)
#define DEBUG_ASSERTF(COND, FORMAT, ...)
#endif

TraceLogLevel setLogLevel(TraceLogLevel level);

__attribute__((format(printf, 3, 4)))
void printLog(TraceLogLevel level, const char* where, const char* format, ...);
void vprintLog(TraceLogLevel level, const char* where, const char* format, va_list args); 

void raylibLogCallback(int level, const char* format, va_list args); 


static inline void assertion(bool condition, const char* where, const char* conditionText) {
    if (!condition) {
        printLog(LOG_FATAL, where, "Assertion failed: %s", conditionText);
    }
}

__attribute__((format(printf, 4, 5)))
static inline void assertionf(bool condition, const char* where, const char* conditionText, const char* format, ...) {
    if (!condition) {
        va_list args;
        va_start(args, format);
        vprintLog(LOG_ERROR, where, format, args);
        va_end(args);
        printLog(LOG_FATAL, where, "Assertion failed: %s", conditionText);
    }
}

#endif

