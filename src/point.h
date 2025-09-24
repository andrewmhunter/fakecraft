#ifndef POINT_H
#define POINT_H

#include "util.h"
#include "hash.h"

typedef struct Point {
    int x;
    int y;
    int z;
} Point;

const char* formatPoint(Point point);

static inline Point point(int x, int y, int z) {
    return (Point){x, y, z};
}

static inline Point pointZero() {
    return (Point){0, 0, 0};
}

static inline Point pointOne() {
    return (Point){1, 1, 1};
}

static inline Point pointNegate(Point point) {
    return (Point){-point.x, -point.y, -point.z};
}

static inline Point pointAdd(Point lhs, Point rhs) {
    return (Point){lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

static inline Point pointAddValue(Point lhs, int x, int y, int z) {
    return (Point){lhs.x + x, lhs.y + y, lhs.z + z};
}

static inline Point pointSubtract(Point lhs, Point rhs) {
    return (Point){lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

static inline Point pointSubtractValue(Point lhs, int rhs) {
    return (Point){lhs.x - rhs, lhs.y - rhs, lhs.z - rhs};
}

static inline Point pointMultiply(Point lhs, Point rhs) {
    return (Point){lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
}

static inline Point pointDivide(Point lhs, Point rhs) {
    return (Point){lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z};
}

static inline Point pointScale(Point lhs, int rhs) {
    return (Point){lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
}

static inline Point pointDivideScalar(Point lhs, int rhs) {
    return (Point){lhs.x / rhs, lhs.y / rhs, lhs.z / rhs};
}

static inline Point pointMin(Point lhs, Point rhs) {
    return (Point) {
        MIN(lhs.x, rhs.x),
        MIN(lhs.y, rhs.y),
        MIN(lhs.z, rhs.z)
    };
}

static inline Point pointMax(Point lhs, Point rhs) {
    return (Point) {
        MAX(lhs.x, rhs.x),
        MAX(lhs.y, rhs.y),
        MAX(lhs.z, rhs.z)
    };
}

static inline bool pointEquals(Point lhs, Point rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

static inline Hash pointHash(Hash hash, Point point) {
    hash = hashInt(hash, point.x);
    hash = hashInt(hash, point.y);
    return hashInt(hash, point.z);
}

static inline Hash point2Hash(Hash hash, Point point) {
    hash = hashInt(hash, point.x);
    return hashInt(hash, point.z);
}

static inline Vector3 pointToVector3(Point point) {
    return (Vector3){point.x, point.y, point.z};
}

static inline Point vector3ToPoint(Vector3 vector) {
    return (Point){floor(vector.x), floor(vector.y), floor(vector.z)};
    //return (Point){vector.x, vector.y, vector.z};
}

static inline Point pointAddX(Point point, int x) {
    return (Point){point.x + x, point.y, point.z};
}

static inline Point pointAddY(Point point, int y) {
    return (Point){point.x, point.y + y, point.z};
}

static inline Point pointAddZ(Point point, int z) {
    return (Point){point.x, point.y, point.z + z};
}

static inline int pointGetAxis(Point point, Axis axis) {
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

static inline Point pointSetAxis(Point point, Axis axis, int value) {
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
