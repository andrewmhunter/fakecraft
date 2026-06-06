#ifndef FAKECRAFT_TEXT_HPP
#define FAKECRAFT_TEXT_HPP

#include <format>
#include <array>
#include <glm/glm.hpp>
#include "graphics.hpp"

class Font {
private:

public:
    std::array<int, UCHAR_MAX + 1> characterWidths{};
    int characterHeight{};
    Texture texture;

    Font(std::string textureFile);
    Font(const Image& fontImage);
};

class TextBatch {
private:
    std::reference_wrapper<const Font> font;
    Mesh mesh{GL_TRIANGLES};

public:
    TextBatch(const Font& font);

    void drawString(int scale, glm::ivec2 position, glm::vec4 color, std::string string);
    void drawString(glm::ivec2 position, std::string string);

    template<typename... Args>
    void drawText(int scale, glm::ivec2 position, glm::vec4 color, std::format_string<Args...> fmt, Args... args) {
        drawString(scale, position, color, std::format(fmt, args...));
    }

    void draw();
};

#endif

