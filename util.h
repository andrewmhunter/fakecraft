#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stdlib.h>
#include <raylib.h>
#include <raymath.h>

extern Font font;

__attribute__((format(printf, 3, 4)))
void renderText(int x, int y, const char* format, ...);

void saveScreenshot(void);

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

// Random numbers

void randomizeSeed();
int randomInt(int max);
int randomRange(int min, int max);
bool randomChance(int numerator, int denominator);

typedef enum {
    DIRECTION_SOUTH,
    DIRECTION_EAST,
    DIRECTION_NORTH,
    DIRECTION_WEST,
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_COUNT,
} Direction;

#define DIRECTION_CARDINAL_COUNT 4

// https://stackoverflow.com/a/14997413
inline int positiveModulo(int x, int y) {
    return (x % y + y) % y;
}

//https://www.geeksforgeeks.org/dsa/floor-and-ceil-of-integer-division/
inline int floorDiv(int x, int y) {
    int value = x / y;
    if ((x ^ y) < 0 && x % y != 0) {
        value--;
    }
    return value;
}

inline int floorDivFast(int x, int y) {
    int value = x / y;
    value -= (value < 0 && x % y != 0);
    return value;
}

typedef struct {
    int x;
    int y;
    int z;
} Point;

inline int sign(int number) {
    if (number == 0) {
        return 0;
    }
    if (number < 0) {
        return -1;
    }
    return 1;
}

inline int wrapInt(int value, int min, int max) {
    int range = max - min;
    value = positiveModulo(value - min, range) + min;
    return value;
}

inline int clampInt(int value, int min, int max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

inline Point point(int x, int y, int z) {
    return (Point){x, y, z};
}

inline Point pointZero() {
    return (Point){0, 0, 0};
}

inline Point pointOne() {
    return (Point){1, 1, 1};
}

inline Point pointNegate(Point point) {
    return (Point){-point.x, -point.y, -point.z};
}

inline Point pointAdd(Point lhs, Point rhs) {
    return (Point){lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

inline Point pointAddValue(Point lhs, int x, int y, int z) {
    return (Point){lhs.x + x, lhs.y + y, lhs.z + z};
}

inline Point pointSubtract(Point lhs, Point rhs) {
    return (Point){lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

inline Point pointSubtractValue(Point lhs, int rhs) {
    return (Point){lhs.x - rhs, lhs.y - rhs, lhs.z - rhs};
}

inline Point pointMultiply(Point lhs, Point rhs) {
    return (Point){lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
}

inline Point pointDivide(Point lhs, Point rhs) {
    return (Point){lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z};
}

inline Point pointScale(Point lhs, int rhs) {
    return (Point){lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
}

inline Point pointDivideScalar(Point lhs, int rhs) {
    return (Point){lhs.x / rhs, lhs.y / rhs, lhs.z / rhs};
}

inline Point pointMin(Point lhs, Point rhs) {
    return (Point) {
        MIN(lhs.x, rhs.x),
        MIN(lhs.y, rhs.y),
        MIN(lhs.z, rhs.z)
    };
}

inline Point pointMax(Point lhs, Point rhs) {
    return (Point) {
        MAX(lhs.x, rhs.x),
        MAX(lhs.y, rhs.y),
        MAX(lhs.z, rhs.z)
    };
}

inline bool pointEquals(Point lhs, Point rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

inline size_t pointHash(Point point) {
    // TODO: Replace with real hash function
    return point.x * 11 + point.y * 7 + point.z * 3;
}

inline Vector3 pointToVector3(Point point) {
    return (Vector3){point.x, point.y, point.z};
}

inline Point vector3ToPoint(Vector3 vector) {
    return (Point){floor(vector.x), floor(vector.y), floor(vector.z)};
    //return (Point){vector.x, vector.y, vector.z};
}

inline Point pointAddX(Point point, int x) {
    return (Point){point.x + x, point.y, point.z};
}

inline Point pointAddY(Point point, int y) {
    return (Point){point.x, point.y + y, point.z};
}

inline Point pointAddZ(Point point, int z) {
    return (Point){point.x, point.y, point.z + z};
}

inline Point directionToPoint(Direction direction) {
    static const Point points[DIRECTION_COUNT] = {
        //[DIRECTION_NONE]  = { 0,  0,  0},
        [DIRECTION_EAST]  = { 1,  0,  0},
        [DIRECTION_WEST]  = {-1,  0,  0},
        [DIRECTION_UP]    = { 0,  1,  0},
        [DIRECTION_DOWN]  = { 0, -1,  0},
        [DIRECTION_NORTH] = { 0,  0,  -1},
        [DIRECTION_SOUTH] = { 0,  0, 1},
    };
    return points[direction];
}

inline Direction invertDirection(Direction direction) {
    static const Direction inverted[DIRECTION_COUNT] = {
        [DIRECTION_UP]    = DIRECTION_DOWN,
        [DIRECTION_SOUTH] = DIRECTION_NORTH,
        [DIRECTION_EAST]  = DIRECTION_WEST,
        [DIRECTION_NORTH] = DIRECTION_SOUTH,
        [DIRECTION_WEST]  = DIRECTION_EAST,
        [DIRECTION_DOWN]  = DIRECTION_UP,
    };
    return inverted[direction];
}

inline Matrix MatrixTranslateV(Vector3 vector) {
    return MatrixTranslate(vector.x, vector.y, vector.z);
}

inline float squaref(float x) {
    return x * x;
}

const char* formatPoint(Point point);
const char* formatVector3(Vector3 vector);


typedef enum {
    AXIS_X,
    AXIS_Y,
    AXIS_Z,
    AXIS_COUNT
} Axis;

inline Axis axisOther0(Axis axis) {
    static const Axis others[AXIS_COUNT] = {
        [AXIS_X] = AXIS_Y,
        [AXIS_Y] = AXIS_X,
        [AXIS_Z] = AXIS_X
    };
    return others[axis];
}

inline Axis axisOther1(Axis axis) {
    static const Axis others[AXIS_COUNT] = {
        [AXIS_X] = AXIS_Z,
        [AXIS_Y] = AXIS_Z,
        [AXIS_Z] = AXIS_Y
    };
    return others[axis];
}

inline float vector3GetAxis(Vector3 vector, Axis axis) {
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

inline Vector3 vector3SetAxis(Vector3 vector, Axis axis, float value) {
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

inline int pointGetAxis(Point point, Axis axis) {
    switch (axis) {
        case AXIS_X:
            return point.x;
        case AXIS_Y:
            return point.y;
        case AXIS_Z:
            return point.z;
        default:
            break;
    }
    return 0;
}

inline Point pointSetAxis(Point point, Axis axis, int value) {
    switch (axis) {
        case AXIS_X:
            return (Point){value, point.y, point.z};
        case AXIS_Y:
            return (Point){point.x, value, point.z};
        case AXIS_Z:
            return (Point){point.x, point.y, value};
        default:
            break;
    }
    return pointZero();
}

#endif
