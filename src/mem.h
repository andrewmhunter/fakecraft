#ifndef MEM_H
#define MEM_H

#include <stdlib.h>

#define ALLOC(T) (malloc(sizeof(T)))
#define ALLOC_N(T, SIZE) (malloc(sizeof(T) * (SIZE)))

#define SPAN_T(T) struct { \
        T* data; \
        size_t length; \
    }

#define LIST_TS(T, SPAN) struct { \
        union { \
            SPAN span; \
            struct { \
                T* data; \
                size_t length; \
            }; \
        }; \
        size_t capacity; \
    }

#define LIST_T(T) struct { \
        union { \
            SPAN_T(T) span; \
            struct { \
                T* data; \
                size_t length; \
            }; \
        }; \
        size_t capacity; \
    }

#define LIST_INIT(LIST) do { \
        (LIST)->data = NULL; \
        (LIST)->length = 0; \
        (LIST)->capacity = 0; \
    } while (0)

#define LIST_FREE(LIST) do { \
        free((LIST)->data); \
        LIST_INIT(LIST); \
    } while (0)

#define LIST_RESIZE(LIST, SIZE) do { \
        (LIST)->data = arrayResize((LIST)->data, sizeof((LIST)->data[0]), \
                &(LIST)->capacity, (SIZE)); \
    } while (0)

#define LIST_EXTEND(LIST, EXTENSION) \
    LIST_RESIZE(LIST, (LIST)->length + (EXTENSION))

#define LIST_PUSH(LIST, VALUE) do { \
        LIST_EXTEND(LIST, 1); \
        (LIST)->data[(LIST)->length++] = (VALUE); \
    } while (0)


static inline void* arrayResize(void* data, size_t elementSize,
        size_t* capacity, size_t newCapacity
) {
    if (newCapacity <= *capacity) {
        return data;
    }

    if (*capacity == 0) {
        *capacity = 8;
    }

    while (*capacity < newCapacity) {
        *capacity *= 2;
    }


    return realloc(data, *capacity * elementSize);
}


typedef SPAN_T(char) StringSpan;
typedef LIST_TS(char, StringSpan) String;

//String stringAllocPrintf(const char* format, ...);
//void stringClearPrintf(String* string, const char* format, ...);

#endif
