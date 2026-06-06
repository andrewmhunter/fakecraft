#ifndef DIRECTION_HPP
#define DIRECTION_HPP

#include <glm/glm.hpp>

enum Direction {
    DIRECTION_SOUTH,
    DIRECTION_EAST,
    DIRECTION_NORTH,
    DIRECTION_WEST,
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_COUNT,
};

#define DIRECTION_CARDINAL_COUNT 4
#define DIRECTION_FIRST 0

static inline glm::ivec3 directionToPoint(Direction direction) {
    glm::ivec3 points[DIRECTION_COUNT] = {};
    points[DIRECTION_EAST]  = { 1,  0,  0};
    points[DIRECTION_WEST]  = {-1,  0,  0};
    points[DIRECTION_UP]    = { 0,  1,  0};
    points[DIRECTION_DOWN]  = { 0, -1,  0};
    points[DIRECTION_NORTH] = { 0,  0,  -1};
    points[DIRECTION_SOUTH] = { 0,  0, 1};
    return points[direction];
}

static inline glm::ivec2 directionToIvec2(Direction direction) {
    glm::ivec2 points[DIRECTION_COUNT] = {};
    points[DIRECTION_EAST]  = { 1,  0};
    points[DIRECTION_WEST]  = {-1,  0};
    points[DIRECTION_UP]    = { 0,  0};
    points[DIRECTION_DOWN]  = { 0,  0};
    points[DIRECTION_NORTH] = { 0, -1};
    points[DIRECTION_SOUTH] = { 0,  1};
    return points[direction];
}

static inline glm::ivec3 directionToIvec3(Direction direction) {
    glm::ivec3 points[DIRECTION_COUNT] = {};
    points[DIRECTION_EAST]  = { 1,  0,  0};
    points[DIRECTION_WEST]  = {-1,  0,  0};
    points[DIRECTION_UP]    = { 0,  1,  0};
    points[DIRECTION_DOWN]  = { 0, -1,  0};
    points[DIRECTION_NORTH] = { 0,  0,  -1};
    points[DIRECTION_SOUTH] = { 0,  0, 1};
    return points[direction];
}

static inline Direction invertDirection(Direction direction) {
    Direction inverted[DIRECTION_COUNT] = {};
    inverted[DIRECTION_UP]    = DIRECTION_DOWN;
    inverted[DIRECTION_SOUTH] = DIRECTION_NORTH;
    inverted[DIRECTION_EAST]  = DIRECTION_WEST;
    inverted[DIRECTION_NORTH] = DIRECTION_SOUTH;
    inverted[DIRECTION_WEST]  = DIRECTION_EAST;
    inverted[DIRECTION_DOWN]  = DIRECTION_UP;
    return inverted[direction];
}

static inline Direction directionCardinalRightAngle(Direction direction) {
    Direction right[DIRECTION_COUNT] = {};
    right[DIRECTION_SOUTH] = DIRECTION_WEST;
    right[DIRECTION_EAST]  = DIRECTION_SOUTH;
    right[DIRECTION_NORTH] = DIRECTION_EAST;
    right[DIRECTION_WEST]  = DIRECTION_NORTH;
    return right[direction];
}

static inline const char* directionName(Direction direction) {
    const char* directionNames[DIRECTION_COUNT] = {};
    directionNames[DIRECTION_UP]    = "up";
    directionNames[DIRECTION_SOUTH] = "south";
    directionNames[DIRECTION_EAST]  = "east";
    directionNames[DIRECTION_NORTH] = "north";
    directionNames[DIRECTION_WEST]  = "west";
    directionNames[DIRECTION_DOWN]  = "down";
    return directionNames[direction];
}

#endif
