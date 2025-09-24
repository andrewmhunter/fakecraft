#include <raylib.h>
#include "point.h"

const char* formatPoint(Point point) {
    return TextFormat("x: %d, y: %d, z: %d", point.x, point.y, point.z);
}

