#include <cctype>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <ranges>
#include <GLFW/glfw3.h>
#include "util.hpp"
#include "engine/camera.hpp"
#include "engine/logger.hpp"
#include "stb_image_write.h"


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


void saveScreenshot() {
    // https://stackoverflow.com/questions/1531055/time-into-string-with-hhmmss-format-c-programming

    std::time_t currentTime;
    std::tm* local;
    std::time(&currentTime);
    local = std::localtime(&currentTime);
    char fileName[64];
    std::strftime(fileName, sizeof(fileName) - 1, "screenshots/screenshot%FT%T.png", local);
    fileName[sizeof(fileName) - 1] = '\0';

    int windowWidth = Camera::globalCamera.windowSize.x;
    int windowHeight = Camera::globalCamera.windowSize.y;

    char* image = new char[3 * windowWidth * windowHeight];
    glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, image);
    // glReadPixels starts from the bottom left corner, stbt_write_png starts at the top left
    // so the image must be vertically flipped or it will save upside down
    for (int row = 0; row < windowHeight / 2; ++row) {
        for (int column = 0; column < windowWidth * 3; ++column) {
            std::swap(image[row * windowWidth * 3 + column], image[(windowHeight - row - 1) * windowWidth * 3 + column]);
        }
    }
    if (stbi_write_png(fileName, windowWidth, windowHeight, 3, image, 3 * windowWidth)) {
        Logger::info(std::format("Saved screenshot {}", fileName));
    } else {
        Logger::error(std::format("Failed to save screenshot {}", fileName));
    }
    delete[] image;
}
