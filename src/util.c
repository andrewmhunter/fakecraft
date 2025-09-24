#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <m-lib/m-string.h>
#include "util.h"

Font font;

void renderText(int x, int y, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char* string = vmemprintf(format, args);
    va_end(args);

    DrawTextEx(font, string, (Vector2){x + 2, y + 2}, 16, 2, GRAY);
    DrawTextEx(font, string, (Vector2){x, y}, 16, 2, WHITE);

    free(string);
}

char* memprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char* string = vmemprintf(format, args);
    va_end(args);
    return string;
}

char* vmemprintf(const char* format, va_list args) {
    va_list sizeArgs;
    va_copy(sizeArgs, args);
    int size = vsnprintf(NULL, 0, format, sizeArgs) + 1;
    char* string = malloc(size);
    vsnprintf(string, size, format, args);
    va_end(sizeArgs);
    return string;
}

void saveScreenshot(void) {
    // https://stackoverflow.com/questions/1531055/time-into-string-with-hhmmss-format-c-programming
    time_t currentTime;
    struct tm* local;
    time(&currentTime);
    local = localtime(&currentTime);
    char fileName[64];
    strftime(fileName, sizeof(fileName) - 1, "screenshots/screenshot%FT%T.png", local);
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

const char* formatVector3(Vector3 vector) {
    return TextFormat("x: %+.02f, y: %+.02f, z: %+.02f", vector.x, vector.y, vector.z);
}

