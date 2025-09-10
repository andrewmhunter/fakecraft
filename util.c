#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <time.h>
#include <m-lib/m-string.h>
#include "util.h"

Font font;

void renderText(int x, int y, const char* format, ...) {
    va_list args;
    va_start(args, format);

    string_t string;
    string_init_vprintf(string, format, args);
    va_end(args);

    DrawTextEx(font, string_get_cstr(string), (Vector2){x + 2, y + 2}, 16, 2, GRAY);
    DrawTextEx(font, string_get_cstr(string), (Vector2){x, y}, 16, 2, WHITE);

    string_clear(string);
}

void saveScreenshot(void) {
    // https://stackoverflow.com/questions/1531055/time-into-string-with-hhmmss-format-c-programming
    time_t currentTime;
    struct tm* local;
    time(&currentTime);
    local = localtime(&currentTime);
    char fileName[64];
    strftime(fileName, sizeof(fileName) - 1, "screenshot%FT%T.png", local);
    fileName[sizeof(fileName) - 1] = '\0';
    TakeScreenshot(fileName);
}

void randomizeSeed() {
    srand(time(NULL));
}

int randomInt(int max) {
    return rand() % max;
}

int randomRange(int min, int max) {
    return randomInt(max - min) + min;
}

bool randomChance(int numerator, int denominator) {
    return randomInt(denominator) < numerator;
}

const char* formatPoint(Point point) {
    return TextFormat("x: %d, y: %d, z: %d", point.x, point.y, point.z);
}

const char* formatVector3(Vector3 vector) {
    return TextFormat("x: %+.02f, y: %+.02f, z: %+.02f", vector.x, vector.y, vector.z);
}

