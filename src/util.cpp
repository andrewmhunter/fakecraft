#include <cctype>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <ranges>
#include <GLFW/glfw3.h>
#include "util.hpp"
#include "logger.hpp"

void saveScreenshot(void) {
    // https://stackoverflow.com/questions/1531055/time-into-string-with-hhmmss-format-c-programming

    std::time_t currentTime;
    std::tm* local;
    std::time(&currentTime);
    local = std::localtime(&currentTime);
    char fileName[64];
    std::strftime(fileName, sizeof(fileName) - 1, "screenshot%FT%T.png", local);
    fileName[sizeof(fileName) - 1] = '\0';
    // TODO:
    //TakeScreenshot(fileName);

    char finalFileName[128];
    std::snprintf(finalFileName, sizeof(finalFileName) - 1, "screenshots/%s", fileName);
    std::rename(fileName, finalFileName);

    Logger::info(std::format("Saved screenshot {}", finalFileName));
}

void randomizeSeed() {
    std::srand(std::time(NULL));
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


std::string toLower(std::string_view input) {
    return input
        | std::views::transform([](char ch){
            return std::tolower(ch);
        })
        | std::ranges::to<std::string>();
}

std::string toUpper(std::string_view input) {
    std::string output;
    for (char ch : input) {
        output.push_back(std::toupper(ch));
    }
    return output;
}

static auto findIfSpace(auto begin, auto end) {
    return std::find_if(begin, end, [](char ch) {
        return !std::isspace(ch);
    });
}

void trimLeft(std::string& input) {
    input.erase(input.begin(), findIfSpace(input.begin(), input.end()));
}

void trimRight(std::string& input) {
    auto last = findIfSpace(input.rbegin(), input.rend()).base();
    input.erase(last, input.end());
}

void trim(std::string& input) {
    trimLeft(input);
    trimRight(input);
}
