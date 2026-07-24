#ifndef POINT_HPP
#define POINT_HPP

#include <functional>
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

class CompareIvec3XZOnlyFO {
public:
    bool operator()(const glm::ivec3& left, const glm::ivec3& right) const;
};

class CompareIvec2FO {
public:
    bool operator()(const glm::ivec2& left, const glm::ivec2& right) const;
};

struct HashIvec3XZOnly {
    constexpr std::size_t operator()(const glm::ivec3& vector) const {
        std::size_t h1 = std::hash<int>{}(vector.x);
        std::size_t h2 = std::hash<int>{}(vector.z);
        return h1 ^ (h2 << 1);
    }
};


#endif
