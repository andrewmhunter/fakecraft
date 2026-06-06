#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <GLFW/glfw3.h>
#include <string.h>
#include "util.hpp"
#include "logger.hpp"

void saveScreenshot(void) {
    // https://stackoverflow.com/questions/1531055/time-into-string-with-hhmmss-format-c-programming

    time_t currentTime;
    struct tm* local;
    time(&currentTime);
    local = localtime(&currentTime);
    char fileName[64];
    strftime(fileName, sizeof(fileName) - 1, "screenshot%FT%T.png", local);
    fileName[sizeof(fileName) - 1] = '\0';
    // TODO:
    //TakeScreenshot(fileName);

    char finalFileName[128];
    snprintf(finalFileName, sizeof(finalFileName) - 1, "screenshots/%s", fileName);
    rename(fileName, finalFileName);

    INFO("Saved screenshot %s", finalFileName);
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

