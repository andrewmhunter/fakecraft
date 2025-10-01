#ifndef DIRECTION_H
#define DIRECTION_H

#include "point.h"

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
#define DIRECTION_FIRST 0

static inline Point directionToPoint(Direction direction) {
    static const Point points[DIRECTION_COUNT] = {
        [DIRECTION_EAST]  = { 1,  0,  0},
        [DIRECTION_WEST]  = {-1,  0,  0},
        [DIRECTION_UP]    = { 0,  1,  0},
        [DIRECTION_DOWN]  = { 0, -1,  0},
        [DIRECTION_NORTH] = { 0,  0,  -1},
        [DIRECTION_SOUTH] = { 0,  0, 1},
    };
    return points[direction];
}

static inline Direction invertDirection(Direction direction) {
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

static inline const char* directionName(Direction direction) {
    static const char* const directionNames[DIRECTION_COUNT] = {
        [DIRECTION_UP]    = "up",
        [DIRECTION_SOUTH] = "south",
        [DIRECTION_EAST]  = "east",
        [DIRECTION_NORTH] = "north",
        [DIRECTION_WEST]  = "west",
        [DIRECTION_DOWN]  = "down",
    };
    return directionNames[direction];
}

#endif
