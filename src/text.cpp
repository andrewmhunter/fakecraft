#include "text.hpp"
#include "config.hpp"
#include "graphics.hpp"
#include "logger.hpp"
#include "resource_manager.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>

static int getCharacterWidth(
        const Image& fontImage,
        int index,
        int baseCharacterWidth,
        int characterHeight,
        int defaultValue
) {
    int initalX = (index % 16) * baseCharacterWidth;
    int initalY = (index / 16) * characterHeight;

    for (int x = baseCharacterWidth - 1; x >= 0; --x) {
        for (int y = 0; y < characterHeight; ++y) {
            if (fontImage.getPixel(x + initalX, y + initalY).a > 0.f) {
                return x + 1;
            }
        }
    }

    return defaultValue;
}

Font::Font(std::string textureFile) : Font{Image{textureFile}} {}

Font::Font(const Image& fontImage) : texture{fontImage} {
    Logger::assertion(fontImage.height % 16 == 0);
    Logger::assertion(fontImage.width % 16 == 0);

    int baseCharacterWidth = fontImage.width / 16;
    characterHeight = fontImage.height / 16;

    int spaceWidth = 4;

    for (int i = 0; i <= UCHAR_MAX; ++i) {
        characterWidths[i] = getCharacterWidth(fontImage, i, baseCharacterWidth, characterHeight, spaceWidth);
    }
}

TextBatch::TextBatch() : font{ResourceManager::instance().font} {}
TextBatch::TextBatch(const Font& font) : font{font} {}

void TextBatch::drawString(glm::ivec2 position, std::string string) {
    drawString(Config::settings->gui.scale, position, color::white, string);
}

void TextBatch::drawString(int scale, glm::ivec2 position, glm::vec4 color, std::string string) {
    glm::ivec2 charSize{8 * scale};
    offset = position;

    for (unsigned char ch : string) {
        if (ch == '\n') {
            offset.x = position.x;
            offset.y += charSize.y;
            continue;
        }

        glm::vec2 charIndex{ch % 16, ch / 16};

        glm::vec2 texcoord0 = charIndex / 16.f;
        glm::vec2 texcoord1 = (charIndex + 1.f) / 16.f;

        glm::vec2 position1 = offset + charSize;

        mesh.pushFace(
            glm::vec3{offset.x, position1.y, 0.f}, glm::vec3{offset, 0.f},
            glm::vec3{position1.x, offset.y, 0.f}, glm::vec3{position1, 0.f},

            texcoord0, texcoord1,
            color,
            {0.f, 1.f, 0.f}
        );

        offset.x += (font.get().characterWidths.at(ch) + 1) * scale;
    }
}

void TextBatch::draw() {
    GPUMesh gpuMesh = mesh.upload();
    gpuMesh.draw();
}

void TextBatch::drawHighlighted(ShaderProgram& shader, int scale) {
    GPUMesh gpuMesh = mesh.upload();

    shader.setUniformVec4("color", glm::vec4{0.f, 0.f, 0.f, 0.5f});
    glm::vec3 offset = glm::vec3{static_cast<float>(scale), static_cast<float>(-scale), 0.f};
    shader.setUniformMat4("model", glm::translate(glm::mat4{1.f}, offset));
    gpuMesh.draw();

    shader.setUniformVec4("color", color::white);
    shader.setUniformMat4("model", glm::mat4{1.f});
    gpuMesh.draw();
}

void TextBatch::drawHighlighted(ShaderProgram& shader) {
    drawHighlighted(shader, Config::settings->gui.scale);
}
