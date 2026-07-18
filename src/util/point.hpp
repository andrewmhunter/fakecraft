#ifndef POINT_HPP
#define POINT_HPP

#include <glm/common.hpp>
#include <glm/glm.hpp>

static inline glm::ivec3 vector3ToPoint(glm::vec3 vector) {
    return glm::ivec3{glm::floor(vector)};
}

template<typename T>
constexpr auto operator<=>(const glm::vec<3, T>& left, const glm::vec<3, T>& right) {
    if (left.x != right.x) {
        return left.x <=> right.x;
    }
    if (left.y != right.y) {
        return left.y <=> right.y;
    }
    return left.z <=> right.z;
}

class CompareIvec3FO {
public:
    bool operator()(const glm::ivec3& left, const glm::ivec3& right) const;
};

template<typename T>
constexpr auto operator<=>(const glm::vec<2, T>& left, const glm::vec<2, T>& right) {
    if (left.x != right.x) {
        return left.x <=> right.x;
    }
    return left.y <=> right.y;
}

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
