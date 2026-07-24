#include "point.hpp"

bool CompareIvec3FO::operator()(const glm::ivec3& left, const glm::ivec3& right) const {
    return left < right;
}

bool CompareIvec3XZOnlyFO::operator()(const glm::ivec3& left, const glm::ivec3& right) const {
    if (left.x != right.x) {
        return left.x < right.x;
    }
    return left.z < right.z;
}


bool CompareIvec2FO::operator()(const glm::ivec2& left, const glm::ivec2& right) const {
    return left < right;
}
