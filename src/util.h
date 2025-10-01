#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <raylib.h>
#include <raymath.h>

extern Font font;

__attribute__((format(printf, 3, 4)))
void renderText(int x, int y, const char* format, ...);

void saveScreenshot(void);

// Must be freed
__attribute__((format(printf, 1, 2)))
char* memprintf(const char* format, ...);
// Must be freed
char* vmemprintf(const char* format, va_list args);

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

// Random numbers

void randomizeSeed();
int randomInt(int max);
int randomRange(int min, int max);
bool randomChance(int numerator, int denominator);

// https://stackoverflow.com/a/14997413
static inline int positiveModulo(int x, int y) {
    return (x % y + y) % y;
}

//https://www.geeksforgeeks.org/dsa/floor-and-ceil-of-integer-division/
static inline int floorDiv(int x, int y) {
    int value = x / y;
    if ((x ^ y) < 0 && x % y != 0) {
        value--;
    }
    return value;
}

static inline int floorDivFast(int x, int y) {
    int value = x / y;
    value -= (value < 0 && x % y != 0);
    return value;
}

static inline int sign(int number) {
    if (number == 0) {
        return 0;
    }
    if (number < 0) {
        return -1;
    }
    return 1;
}

static inline int wrapInt(int value, int min, int max) {
    int range = max - min;
    value = positiveModulo(value - min, range) + min;
    return value;
}

static inline int clampInt(int value, int min, int max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

static inline Matrix MatrixTranslateV(Vector3 vector) {
    return MatrixTranslate(vector.x, vector.y, vector.z);
}

static inline float squaref(float x) {
    return x * x;
}

const char* formatVector3(Vector3 vector);


typedef enum {
    AXIS_X,
    AXIS_Y,
    AXIS_Z,
    AXIS_COUNT
} Axis;

static inline Axis axisOther0(Axis axis) {
    static const Axis others[AXIS_COUNT] = {
        [AXIS_X] = AXIS_Y,
        [AXIS_Y] = AXIS_X,
        [AXIS_Z] = AXIS_X
    };
    return others[axis];
}

static inline Axis axisOther1(Axis axis) {
    static const Axis others[AXIS_COUNT] = {
        [AXIS_X] = AXIS_Z,
        [AXIS_Y] = AXIS_Z,
        [AXIS_Z] = AXIS_Y
    };
    return others[axis];
}

#if 0
static float vector3GetAxis(Vector3 vector, Axis axis) {
    switch (axis) {
        case AXIS_X:
            return vector.x;
        case AXIS_Y:
            return vector.y;
        case AXIS_Z:
            return vector.z;
        default:
            break;
    }
    return 0.f;
}

static inline Vector3 vector3SetAxis(Vector3 vector, Axis axis, float value) {
    switch (axis) {
        case AXIS_X:
            return (Vector3){value, vector.y, vector.z};
        case AXIS_Y:
            return (Vector3){vector.x, value, vector.z};
        case AXIS_Z:
            return (Vector3){vector.x, vector.y, value};
        default:
            break;
    }
    return Vector3Zero();
}
#endif

#endif
