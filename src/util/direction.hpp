#ifndef DIRECTION_HPP
#define DIRECTION_HPP

#include <glm/glm.hpp>
#include "engine/logger.hpp"

namespace Dir {
    enum Direction {
        south,
        east,
        north,
        west,
        up,
        down,
    };
}

using Direction = Dir::Direction;

constexpr std::array<Direction, 4> cardinalDirections{
    Direction::south,
    Direction::east,
    Direction::north,
    Direction::west
};

constexpr std::array<Direction, 6> directions{
    Direction::south,
    Direction::east,
    Direction::north,
    Direction::west,
    Direction::up,
    Direction::down
};

constexpr int directionCount = static_cast<int>(Direction::down) + 1;

#define DIRECTION_CARDINAL_COUNT 4
#define DIRECTION_FIRST 0


constexpr glm::ivec3 directionToPoint(Direction direction) {
    switch (direction) {
    case Direction::east:
        return { 1, 0, 0};
    case Direction::west:
        return {-1, 0, 0};
    case Direction::up:
        return {0, 1, 0};
    case Direction::down:
        return {0, -1, 0};
    case Direction::north:
        return {0, 0, -1};
    case Direction::south:
        return {0, 0, 1};
    }
    Logger::unreachable();
}

constexpr glm::ivec2 directionToIvec2(Direction direction) {
    switch (direction) {
    case Direction::east:
        return {1, 0};
    case Direction::west:
        return {-1, 0};
    case Direction::up:
        return {0, 0};
    case Direction::down:
        return {0, 0};
    case Direction::north:
        return {0, -1};
    case Direction::south:
        return {0, 1};
    }
    Logger::unreachable();
}

constexpr glm::ivec3 directionToIvec3(Direction direction) {
    switch (direction) {
    case Direction::east:
        return {1, 0, 0};
    case Direction::west:
        return {-1, 0, 0};
    case Direction::up:
        return {0, 1, 0};
    case Direction::down:
        return {0, -1, 0};
    case Direction::north:
        return {0, 0, -1};
    case Direction::south:
        return {0, 0, 1};
    }
    Logger::unreachable();
}

constexpr Direction invertDirection(Direction direction) {
    switch (direction) {
    case Direction::up:
        return Direction::down;
    case Direction::south:
        return Direction::north;
    case Direction::east:
        return Direction::west;
    case Direction::north:
        return Direction::south;
    case Direction::west:
        return Direction::east;
    case Direction::down:
        return Direction::up;
    }
    Logger::unreachable();
}

constexpr const char* directionName(Direction direction) {
    switch (direction) {
    case Direction::up:
        return "up";
    case Direction::south:
        return "south";
    case Direction::east:
        return "east";
    case Direction::north:
        return "north";
    case Direction::west:
        return "west";
    case Direction::down:
        return "down";
    }
    Logger::unreachable();
}

constexpr Direction directionCardinalRightAngle(Direction direction) {
    switch (direction) {
    case Direction::south:
        return Direction::west;
    case Direction::east:
        return Direction::south;
    case Direction::north:
        return Direction::east;
    case Direction::west:
        return Direction::north;
    default:
        Logger::error(std::format(
            "Function should only be called with cardinal direction (called with direction {})",
            directionName(direction)
        ));
        return Direction::north;
    }
    Logger::unreachable();
}


#endif
