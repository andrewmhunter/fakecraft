#ifndef UTIL_HPP
#define UTIL_HPP

#include <format>
#include <glm/glm.hpp>


#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))


template<int Length, typename T>
struct std::formatter<glm::vec<Length, T>> : std::formatter<T> {
    using Type = glm::vec<Length, T>;

    using std::formatter<T>::parse;

    std::format_context::iterator format(const Type& value, std::format_context& ctx) const {
        auto out = ctx.out();

        for (int i = 0; i < Length - 1; ++i) {
            out = std::formatter<T>::format(value[i], ctx);
            out = std::format_to(out, ", ");
            ctx.advance_to(out);
        }

        return std::formatter<T>::format(value[Length - 1], ctx);
    }
};


void saveScreenshot(void);

// Random numbers

void randomizeSeed();
int randomInt(int max);
int randomRange(int min, int max);
bool randomChance(int numerator, int denominator);

// https://stackoverflow.com/a/14997413
static inline int positiveModulo(int x, int y) {
    return (x % y + y) % y;
}

//https://www.geeksforgeeks.org/dsa/floor-and-ceil-of-integer-division/
static inline int floorDiv(int x, int y) {
    int value = x / y;
    if ((x ^ y) < 0 && x % y != 0) {
        value--;
    }
    return value;
}

static inline int floorDivFast(int x, int y) {
    int value = x / y;
    value -= (value < 0 && x % y != 0);
    return value;
}

static inline int sign(int number) {
    if (number == 0) {
        return 0;
    }
    if (number < 0) {
        return -1;
    }
    return 1;
}

static inline int wrapInt(int value, int min, int max) {
    int range = max - min;
    value = positiveModulo(value - min, range) + min;
    return value;
}

static inline int clampInt(int value, int min, int max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

static inline float squaref(float x) {
    return x * x;
}

#endif

