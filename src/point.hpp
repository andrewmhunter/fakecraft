#ifndef POINT_HPP
#define POINT_HPP

#include <glm/glm.hpp>

static inline glm::ivec3 vector3ToPoint(glm::vec3 vector) {
    return glm::ivec3{static_cast<int>(floor(vector.x)), static_cast<int>(floor(vector.y)), static_cast<int>(floor(vector.z))};
}

std::strong_ordering operator<=>(const glm::ivec3& left, const glm::ivec3& right);

class CompareIvec3FO {
public:
    bool operator()(const glm::ivec3& left, const glm::ivec3& right) const;
};

std::strong_ordering operator<=>(const glm::ivec2& left, const glm::ivec2& right);

class CompareIvec2FO {
public:
    bool operator()(const glm::ivec2& left, const glm::ivec2& right) const;
};

/*template<typename T, glm::qualifier Q>
std::strong_ordering operator<=>(const glm::vec<3, T, Q>& left, const glm::vec<3, T, Q>& right) {
    if (left.x != right.x) {
        return left.x <=> right.x;
    }
    if (left.y != right.y) {
        return left.y <=> right.y;
    }
    return left.z <=> right.z;
}*/

/*template<typename T, glm::qualifier Q>
bool operator<(const glm::vec<3, T, Q>& left, const glm::vec<3, T, Q>& right) {
    if (left.x != right.x) {
        return left.x < right.x;
    }
    if (left.y != right.y) {
        return left.y < right.y;
    }
    return left.z < right.z;
}*/

#endif
