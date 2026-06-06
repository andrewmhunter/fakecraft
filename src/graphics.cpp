#include "graphics.hpp"
#include <sstream>
#include <fstream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <format>
#include <utility>
#include <array>
#include <stb_image.h>
#include "logger.hpp"

Image::Image(std::string fileName) {
    int channels = 0;
    data = stbi_load(fileName.c_str(), &width, &height, &channels, 4);

    if (data == nullptr) {
        // TODO: Fix
        FATAL("Failed to load image");
        std::cout << std::format("Failed to load image '{}'\n", fileName);
    }
}

Image::Image(Image&& image)
:   width{image.width},
    height{image.height},
    data{image.data}
{
    image.width = 0;
    image.height = 0;
    image.data = nullptr;
}

Image& Image::operator=(Image&& image) {
    if (this == &image) {
        return *this;
    }

    width = image.width;
    height = image.height;
    data = image.data;

    image.width = 0;
    image.height = 0;
    image.data = nullptr;

    return *this;
}

Image::~Image() {
    stbi_image_free(data);
}

glm::vec4 Image::getPixel(int x, int y) const {
    ASSERT(x < width);
    ASSERT(x >= 0);
    ASSERT(y >= 0);

    int index = (y * height + x) * 4;

    return glm::vec4{data[index], data[index + 1], data[index + 2], data[index + 3]};
}


/*
 * Texture
 */

Texture::Texture(const std::string& fileName) : Texture{Image{fileName}} {
}

Texture::Texture(const Image& image) {
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::Texture() : textureId{0} {
}

void Texture::bind() {
    if (textureId == 0) {
        ERROR("Texture invalid");
    }

    glBindTexture(GL_TEXTURE_2D, textureId);
}

void Texture::bind(int textureUnit) {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    bind();
}

void Texture::unload() {
    if (textureId == 0) {
        return;
    }
    glDeleteTextures(1, &textureId);
    textureId = 0;
}

std::string loadFile(const std::string& fileName) {
    std::ifstream file{fileName};
    if (file.fail()) {
        std::cout << std::format("Failed to open file '{}'\n", fileName);
    }

    std::stringstream ss{};
    ss << file.rdbuf();
    
    return ss.str();
}

/*
 * FCShader
 */

GLuint Shader::loadShader(const std::string& string, GLenum shaderType) const {
    const char* c_str = string.c_str();

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &c_str, NULL);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
        std::cout << std::format("Shader '{}' compilation failed: {}\n", string, infoLog);
    }

    return shader;
}

GLint Shader::uniformLocation(const std::string& name) {
    return glGetUniformLocation(shaderProgram, name.c_str());
}

Shader::Shader(const std::string& vertexShaderString, const std::string& fragmentShaderString) {
    GLuint vertexShader = this->loadShader(vertexShaderString, GL_VERTEX_SHADER);
    GLuint fragmentShader = this->loadShader(fragmentShaderString, GL_FRAGMENT_SHADER);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);

        std::cout << std::format("Shader program '{}' & '{}' link failed: {}\n",
                vertexShader, fragmentShader, infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader Shader::buildFiles(const std::string& vertexShaderFilePath,
        const std::string& fragmentShaderFilePath
) {
    return Shader::buildStrings(loadFile(vertexShaderFilePath), loadFile(fragmentShaderFilePath));
}

Shader Shader::buildStrings(const std::string& vertexShader, const std::string& fragmentShader) {
    return Shader{vertexShader, fragmentShader};
}

void Shader::use() const {
    if (shaderProgram == 0) {
        ERROR("Shader invalid");
    }

    glUseProgram(shaderProgram);
}

void Shader::unload() {
    if (shaderProgram == 0) {
        return;
    }

    glDeleteProgram(shaderProgram);
    shaderProgram = 0;
}

void Shader::setUniformFloat(const std::string& uniform, float value) {
    glProgramUniform1f(shaderProgram, uniformLocation(uniform), value);
}

void Shader::setUniformInt(const std::string& uniform, int value) {
    glProgramUniform1i(shaderProgram, uniformLocation(uniform), value);
}

void Shader::setUniformUInt(const std::string& uniform, unsigned int value) {
    glProgramUniform1ui(shaderProgram, uniformLocation(uniform), value);
}

void Shader::setUniformVec3(const std::string& uniform, glm::vec3 value) {
    glProgramUniform3fv(shaderProgram, uniformLocation(uniform), 1, glm::value_ptr(value));
}

void Shader::setUniformVec4(const std::string& uniform, glm::vec4 value) {
    glProgramUniform4fv(shaderProgram, uniformLocation(uniform), 1, glm::value_ptr(value));
}

void Shader::setUniformMat4(const std::string& uniform, glm::mat4 value) {
    glProgramUniformMatrix4fv(shaderProgram, uniformLocation(uniform), 1, false, glm::value_ptr(value));
}


/*
 * GPUMesh
 */

GPUMesh::GPUMesh(GLenum primative, GLuint vertexArrayObject, GLuint vertexBufferObject, GLuint elementBufferObject, int elementCount)
    : primative{primative}, vertexArrayObject{vertexArrayObject}, vertexBufferObject{vertexBufferObject},
    elementBufferObject{elementBufferObject}, elementCount{elementCount}
{}

GPUMesh::GPUMesh() : GPUMesh{GL_TRIANGLES, 0, 0, 0, 0} {}

GPUMesh GPUMesh::generate(GLenum primative) {
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    return GPUMesh{primative, vao, 0, 0, 0};
}

void GPUMesh::bind() const {
    if (vertexArrayObject == 0 || !glIsVertexArray(vertexArrayObject)) {
        FATAL("GPUMesh invalid %d", vertexArrayObject);
    }

    glBindVertexArray(vertexArrayObject);
}

void GPUMesh::draw() const {
    bind();
    glDrawElements(primative, elementCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void GPUMesh::unload() {
    if (vertexArrayObject == 0) {
        return;
    }

    //if (vertexArrayObject == 150) {
    //    ERROR("Unloading mesh 150");
    //}

    if (vertexBufferObject != 0) {
        glDeleteBuffers(1, &vertexBufferObject);
    }

    if (elementBufferObject != 0) {
        glDeleteBuffers(1, &elementBufferObject);
    }

    glDeleteVertexArrays(1, &vertexArrayObject);


    vertexBufferObject = 0;
    elementBufferObject = 0;
    vertexArrayObject = 0;
}

/*
 * Mesh
 */

Mesh::Mesh(GLenum primative) : primative{primative} {}

Mesh::Mesh() {}

GPUMesh Mesh::upload() const {
    GPUMesh gpuMesh = GPUMesh::generate(primative);
    gpuMesh.bind();
    gpuMesh.elementCount = indicies.elementLength();

    std::size_t vertexBufferSize = positions.sizeBytes() + normals.sizeBytes()
        + texcoords.sizeBytes() + colors.sizeBytes();

    glGenBuffers(1, &gpuMesh.vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, gpuMesh.vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, nullptr, GL_DYNAMIC_DRAW);

    std::size_t offset = 0;
    offset = positions.bufferData(0, offset);
    offset = texcoords.bufferData(1, offset);
    offset = normals.bufferData(2, offset);
    offset = colors.bufferData(3, offset);

    glGenBuffers(1, &gpuMesh.elementBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpuMesh.elementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.sizeBytes(), indicies.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    return gpuMesh;
}

void Mesh::pushVertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texcoord, glm::vec4 color) {
    positions.push(position);
    normals.push(normal);
    texcoords.push(texcoord);
    colors.push(color);
}

void Mesh::pushFace(glm::vec3 position0, glm::vec3 position1, glm::vec3 position2, glm::vec3 position3,
    glm::vec2 texcoord0, glm::vec2 texcoord1, glm::vec4 color, glm::vec3 normal
) {
    pushVertex(position0, normal, texcoord0, color);
    pushVertex(position1, normal, {texcoord0.x, texcoord1.y}, color);
    pushVertex(position2, normal, texcoord1, color);
    pushVertex(position3, normal, {texcoord1.x, texcoord0.y}, color);

    makeTriangle(-3, -2, 0);
    makeTriangle(-2, -1, 0);
}

void Mesh::pushFace(glm::vec3 position0, glm::vec3 position1, glm::vec3 position2, glm::vec3 position3,
    std::pair<glm::vec2, glm::vec2> texcoords, glm::vec4 color, glm::vec3 normal
) {
    pushFace(position0, position1, position2, position3, texcoords.first, texcoords.second, color, normal);
}

void Mesh::makeTriangle(int offset0, int offset1, int offset2) {
    std::size_t currentIndex = positions.vertexLength() - 1;
    indicies.push(currentIndex + offset0);
    indicies.push(currentIndex + offset1);
    indicies.push(currentIndex + offset2);
}

void Mesh::pushTexturedPrism(glm::mat4 transformation,
        std::span<const std::pair<glm::vec2, glm::vec2>, 6> texcoords
) {
    assert(texcoords.size() == 6);

    glm::vec4 color = color::white;

    glm::vec3 vlbb = transformation * glm::vec4{-0.5, -0.5f, -0.5f, 1.f};
    glm::vec3 vlbf = transformation * glm::vec4{-0.5, -0.5f,  0.5f, 1.f};
    glm::vec3 vltb = transformation * glm::vec4{-0.5,  0.5f, -0.5f, 1.f};
    glm::vec3 vltf = transformation * glm::vec4{-0.5,  0.5f,  0.5f, 1.f};
    glm::vec3 vrbb = transformation * glm::vec4{ 0.5, -0.5f, -0.5f, 1.f};
    glm::vec3 vrbf = transformation * glm::vec4{ 0.5, -0.5f,  0.5f, 1.f};
    glm::vec3 vrtb = transformation * glm::vec4{ 0.5,  0.5f, -0.5f, 1.f};
    glm::vec3 vrtf = transformation * glm::vec4{ 0.5,  0.5f,  0.5f, 1.f};

    pushFace(vltf, vlbf, vrbf, vrtf, texcoords[0], color, {0.f, 0.f, 1.f});
    pushFace(vrtb, vrbb, vlbb, vltb, texcoords[1], color, {0.f, 0.f, -1.f});

    pushFace(vrtf, vrbf, vrbb, vrtb, texcoords[2], color, {1.f, 0.f, 0.f});
    pushFace(vltb, vlbb, vlbf, vltf, texcoords[3], color, {-1.f, 0.f, 0.f});

    pushFace(vltb, vltf, vrtf, vrtb, texcoords[4], color, {0.f, 1.f, 0.f});
    pushFace(vlbf, vlbb, vrbb, vrbf, texcoords[5], color, {0.f, -1.f, 0.f});
}

void Mesh::pushTexturedPrism(glm::mat4 transformation,
        std::span<const std::pair<glm::vec2, glm::vec2>, 3> texcoords
) {
    std::array<std::pair<glm::vec2, glm::vec2>, 6> texcoords6{
        texcoords[0], texcoords[0],
        texcoords[1], texcoords[1],
        texcoords[2], texcoords[2],
    };

    pushTexturedPrism(transformation, texcoords6);
}

void Mesh::pushTexturedPrism(glm::mat4 transformation,
        std::pair<glm::vec2, glm::vec2> texcoords
) {
    std::array<std::pair<glm::vec2, glm::vec2>, 3> texcoords3{texcoords, texcoords, texcoords};
    pushTexturedPrism(transformation, texcoords3);
}


/*
 * Util
 */

void wireframeEnable() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void wireframeDisable() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

GPUMesh cubeMesh{};
GPUMesh rectangleMesh{};
GPUMesh cubeMeshWires{};

void initMeshes() {
    Mesh cubeCpuMesh{};
    cubeCpuMesh.pushTexturedPrism(glm::mat4{1.f}, {{0.f, 0.f}, {1.f, 1.f}});
    cubeMesh = cubeCpuMesh.upload();

    Mesh rectangleCpuMesh{};
    rectangleCpuMesh.pushFace(
        {-0.5, 0.5, 0.},
        {-0.5, -0.5, 0},
        {0.5, -0.5, 0},
        {0.5, 0.5, 0},
        {0, 0}, {1, 1},
        color::white,
        {0, 0, 1}
    );
    rectangleMesh = rectangleCpuMesh.upload();


    /*Mesh cubeWiresCpuMesh{};
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);
    cubeMeshWires = cubeWiresCpuMesh.upload();*/
}

void blendModeInvert() {
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_SUBTRACT);
}

void blendModeNormal() {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
}

void blendModeReplace() {
    glBlendFunc(GL_ONE, GL_ZERO);
    glBlendEquation(GL_FUNC_ADD);
}

void drawCube(Shader shader, glm::vec3 position, glm::vec3 size) {
    glm::mat4 transform{1.f};
    transform = glm::translate(transform, position);
    transform = glm::scale(transform, size);
    shader.setUniformMat4("model", transform);
    cubeMesh.draw();
}

void drawRectangle(Shader shader, glm::vec2 position, glm::vec2 size) {
    glm::mat4 transform{1.f};
    transform = glm::translate(transform, glm::vec3{position, 0.f});
    transform = glm::scale(transform, glm::vec3{size, 1.f});
    shader.setUniformMat4("model", transform);
    rectangleMesh.draw();
}

