#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stdlib.h>
#include <raymath.h>

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

// Random numbers

void randomizeSeed();
int randomInt(int max);
int randomRange(int min, int max);
bool randomChance(int numerator, int denominator);

typedef enum {
    //DIRECTION_NONE,
    DIRECTION_UP,
    DIRECTION_SOUTH,
    DIRECTION_EAST,
    DIRECTION_NORTH,
    DIRECTION_WEST,
    DIRECTION_DOWN,
    DIRECTION_COUNT,
} Direction;

int floorDivide(int x, int y);
int positiveModulo(int x, int y);

typedef struct {
    int x;
    int y;
    int z;
} Point;

Point pointZero();
Point pointOne();

Point pointNegate(Point point);
Point pointAdd(Point lhs, Point rhs);
Point pointAddValue(Point lhs, int rhs);
Point pointSubtract(Point lhs, Point rhs);
Point pointSubtractValue(Point lhs, int rhs);
Point pointMultiply(Point lhs, Point rhs);
Point pointDivide(Point lhs, Point rhs);
Point pointScale(Point lhs, int rhs);
Point pointDivideScalar(Point lhs, int rhs);

Point pointMin(Point lhs, Point rhs);
Point pointMax(Point lhs, Point rhs);

bool pointEquals(Point lhs, Point rhs);
size_t pointHash(Point point);

Vector3 pointToVector3(Point point);
Point vector3ToPoint(Vector3 vector);

Point directionToPoint(Direction direction);
Direction invertDirection(Direction direction);

Matrix MatrixTranslateV(Vector3 vector);

const char* formatPoint(Point point);
const char* formatVector3(Vector3 vector);


#endif
