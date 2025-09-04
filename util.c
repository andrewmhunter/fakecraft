#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <time.h>
#include "util.h"

void randomizeSeed() {
    srand(time(NULL));
}

int randomInt(int max) {
    return rand() % max;
}

int randomRange(int min, int max) {
    return randomInt(max - min) + min;
}

bool randomChance(int numerator, int denominator) {
    return randomInt(denominator) < numerator;
}


Point pointZero() {
    return (Point){0, 0, 0};
}

Point pointOne() {
    return (Point){1, 1, 1};
}

Point pointNegate(Point point) {
    return (Point){-point.x, -point.y, -point.z};
}

Point pointAdd(Point lhs, Point rhs) {
    return (Point){lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Point pointAddValue(Point lhs, int rhs) {
    return (Point){lhs.x + rhs, lhs.y + rhs, lhs.z + rhs};
}

Point pointSubtract(Point lhs, Point rhs) {
    return (Point){lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

Point pointSubtractValue(Point lhs, int rhs) {
    return (Point){lhs.x - rhs, lhs.y - rhs, lhs.z - rhs};
}

Point pointMultiply(Point lhs, Point rhs) {
    return (Point){lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
}

Point pointDivide(Point lhs, Point rhs) {
    return (Point){lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z};
}

Point pointScale(Point lhs, int rhs) {
    return (Point){lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
}

Point pointDivideScalar(Point lhs, int rhs) {
    return (Point){lhs.x / rhs, lhs.y / rhs, lhs.z / rhs};
}

Point pointMin(Point lhs, Point rhs) {
    return (Point) {
        MIN(lhs.x, rhs.x),
        MIN(lhs.y, rhs.y),
        MIN(lhs.z, rhs.z)
    };
}

Point pointMax(Point lhs, Point rhs) {
    return (Point) {
        MAX(lhs.x, rhs.x),
        MAX(lhs.y, rhs.y),
        MAX(lhs.z, rhs.z)
    };
}

bool pointEquals(Point lhs, Point rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

size_t pointHash(Point point) {
    return point.x * 11 + point.y * 7 + point.z * 3;
}

Vector3 pointToVector3(Point point) {
    return (Vector3){point.x, point.y, point.z};
}

Point vector3ToPoint(Vector3 vector) {
    return (Point){floor(vector.x), floor(vector.y), floor(vector.z)};
    //return (Point){vector.x, vector.y, vector.z};
}

Point directionToPoint(Direction direction) {
    const static Point points[DIRECTION_COUNT] = {
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

Direction invertDirection(Direction direction) {
    const static Direction inverted[DIRECTION_COUNT] = {
        [DIRECTION_UP]    = DIRECTION_DOWN,
        [DIRECTION_SOUTH] = DIRECTION_NORTH,
        [DIRECTION_EAST]  = DIRECTION_WEST,
        [DIRECTION_NORTH] = DIRECTION_SOUTH,
        [DIRECTION_WEST]  = DIRECTION_EAST,
        [DIRECTION_DOWN]  = DIRECTION_UP,
    };
    return inverted[direction];
}

Matrix MatrixTranslateV(Vector3 vector) {
    return MatrixTranslate(vector.x, vector.y, vector.z);
}

const char* formatPoint(Point point) {
    return TextFormat("x: %d, y: %d, z: %d", point.x, point.y, point.z);
}

const char* formatVector3(Vector3 vector) {
    return TextFormat("x: %.02f, y: %.02f, z: %.02f", vector.x, vector.y, vector.z);
}

