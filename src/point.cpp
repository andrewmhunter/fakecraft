#include <raylib.h>
#include "point.hpp"

std::strong_ordering operator<=>(const glm::ivec3& left, const glm::ivec3& right) {
    if (left.x != right.x) {
        return left.x <=> right.x;
    }
    if (left.y != right.y) {
        return left.y <=> right.y;
    }
    return left.z <=> right.z;
}

bool CompareIvec3FO::operator()(const glm::ivec3& left, const glm::ivec3& right) const {
    return left < right;
}

std::strong_ordering operator<=>(const glm::ivec2& left, const glm::ivec2& right) {
    if (left.x != right.x) {
        return left.x <=> right.x;
    }
    return left.y <=> right.y;
}

bool CompareIvec2FO::operator()(const glm::ivec2& left, const glm::ivec2& right) const {
    return left < right;
}
