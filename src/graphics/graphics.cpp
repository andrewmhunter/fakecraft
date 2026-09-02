#include "graphics.hpp"
#include <functional>
#include <optional>
#include <sstream>
#include <fstream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <format>
#include <thread>
#include <utility>
#include <array>
#include <stb_image.h>
#include "engine/logger.hpp"

std::thread::id mainThreadId = std::this_thread::get_id();

GLuint uboGlobalsBlock;

OpenGLObject::OpenGLObject(OpenGLObjectLifetimeFunction allocFunction, OpenGLObjectLifetimeFunction freeFunction) : freeFunction{freeFunction} {
    if (std::this_thread::get_id() != mainThreadId) {
        Logger::error("OpenGL resource allocated on thread other than main thread");
    }
    allocFunction(1, &this->object);
}

OpenGLObject::OpenGLObject(OpenGLObject&& other) : freeFunction{other.freeFunction}, object{other.object} {
    other.object = InvalidValue;
    other.freeFunction = nullptr;
}

OpenGLObject& OpenGLObject::operator=(OpenGLObject&& other) {
    if (this == &other) {
        return *this;
    }

    if (object != InvalidValue) {
        freeFunction(1, &object);
    }

    object = other.object;
    freeFunction = other.freeFunction;
    other.object = InvalidValue;
    other.freeFunction = nullptr;

    return *this;
}

OpenGLObject::~OpenGLObject() {
    if (object != InvalidValue) {
        if (std::this_thread::get_id() != mainThreadId) {
            Logger::error("OpenGL resource freed on thread other than main thread");
        }
        freeFunction(1, &object);
    }
}


Image::Image(std::string fileName) {
    int channels = 0;
    data = stbi_load(fileName.c_str(), &width, &height, &channels, 4);

    if (data == nullptr) {
        // TODO: Fix
        Logger::fatal("Failed to load image");
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

glm::vec4 Image::getPixel(glm::ivec2 position) const {
    return getPixel(position.x, position.y);
}

glm::vec4 Image::getPixel(int x, int y) const {
    Logger::assertion(x < width);
    Logger::assertion(x >= 0);
    Logger::assertion(y >= 0);

    int index = (y * height + x) * 4;

    return glm::vec4{data[index], data[index + 1], data[index + 2], data[index + 3]};
}


/*
 * Texture
 */

Texture::Texture(const std::string& fileName) : Texture{Image{fileName}} {
}

Texture::Texture(const Image& image) : textureId{glGenTextures, glDeleteTextures} {
    glBindTexture(GL_TEXTURE_2D, textureId.object);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::bind() const {
    if (textureId.object == 0) {
        Logger::error("Texture invalid");
    }

    glBindTexture(GL_TEXTURE_2D, textureId.object);
}

void Texture::bind(int textureUnit) const {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    bind();
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
 * Shader
 */

 Shader::Shader(GLenum shaderType, const std::string& source)
    : shaderId{[shaderType](GLint, GLuint* id){*id = glCreateShader(shaderType);},
        [](GLint, GLuint* id){glDeleteShader(*id);}}
{
    const char* c_str = source.c_str();
    glShaderSource(shaderId.object, 1, &c_str, nullptr);
    glCompileShader(shaderId.object);

    int success = 0;
    glGetShaderiv(shaderId.object, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shaderId.object, sizeof(infoLog), nullptr, infoLog);
        Logger::fatal(std::format("Shader '{}' compilation failed: {}", source, infoLog));
    }
}

Shader Shader::fromSource(GLenum shaderType, const std::string& source) {
    return Shader{shaderType, source};
}

Shader Shader::fromFile(GLenum shaderType, const std::string& fileName) {
    return Shader{shaderType, loadFile(fileName)};
}


ShaderProgram::ShaderProgram(const Shader& vertex, const Shader& fragment)
    : programId{[](GLint, GLuint* id){*id = glCreateProgram();},
        [](GLint, GLuint* id){glDeleteProgram(*id);}}
{
    glAttachShader(programId.object, vertex.shaderId.object);
    glAttachShader(programId.object, fragment.shaderId.object);

    glLinkProgram(programId.object);

    int success = 0;
    glGetProgramiv(programId.object, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programId.object, sizeof(infoLog), nullptr, infoLog);

        Logger::fatal(std::format("Shader program link failed: {}", infoLog));
    }

    GLuint globalsBlockIndex = glGetUniformBlockIndex(programId.object, "Globals");
    glUniformBlockBinding(programId.object, globalsBlockIndex, 0);
}

ShaderProgram ShaderProgram::loadFiles(const std::string& vertexFileName, const std::string& fragmentFileName) {
    Shader vertexShader = Shader::fromFile(GL_VERTEX_SHADER, vertexFileName);
    Shader fragmentShader = Shader::fromFile(GL_FRAGMENT_SHADER, fragmentFileName);
    return ShaderProgram{vertexShader, fragmentShader};
}


void ShaderProgram::use() const {
    glUseProgram(programId.object);
}

GLint ShaderProgram::uniformLocation(const std::string& name) {
    if (uniformCache.contains(name)) {
        return uniformCache[name];
    }

    GLint location = glGetUniformLocation(programId.object, name.c_str());
    if (location == -1) {
        Logger::warning(std::format("Uniform location of \"{}\" not found", name));
    }
    uniformCache[name] = location;
    return location;
}

void ShaderProgram::setUniformFloat(const std::string& uniform, float value) {
    glProgramUniform1f(programId.object, uniformLocation(uniform), value);
}

void ShaderProgram::setUniformInt(const std::string& uniform, int value) {
    glProgramUniform1i(programId.object, uniformLocation(uniform), value);
}

void ShaderProgram::setUniformUInt(const std::string& uniform, unsigned int value) {
    glProgramUniform1ui(programId.object, uniformLocation(uniform), value);
}

void ShaderProgram::setUniformVec3(const std::string& uniform, glm::vec3 value) {
    glProgramUniform3fv(programId.object, uniformLocation(uniform), 1, glm::value_ptr(value));
}

void ShaderProgram::setUniformVec4(const std::string& uniform, glm::vec4 value) {
    glProgramUniform4fv(programId.object, uniformLocation(uniform), 1, glm::value_ptr(value));
}

void ShaderProgram::setUniformMat4(const std::string& uniform, glm::mat4 value) {
    glProgramUniformMatrix4fv(programId.object, uniformLocation(uniform), 1, false, glm::value_ptr(value));
}

void ShaderProgram::setModel(glm::mat4 model) {
    setUniformMat4("model", model);
}

void ShaderProgram::setColor(glm::vec4 color) {
    setUniformVec4("color", color);
}

GLuint ShaderProgram::getId() const {
    return programId.object;
}

/*
 * GPUMesh
 */

GPUMesh::GPUMesh(GLenum primative)
    : primative{primative},
    vertexArrayObject{glGenVertexArrays, glDeleteVertexArrays},
    vertexBufferObject{glGenBuffers, glDeleteBuffers},
    elementBufferObject{glGenBuffers, glDeleteBuffers},
    elementCount{0}
{
    glBindVertexArray(vertexArrayObject.object);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject.object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject.object);
}

void GPUMesh::bind() const {
    if (vertexArrayObject.object == 0 || !glIsVertexArray(vertexArrayObject.object)) {
        Logger::fatal(std::format("GPUMesh invalid {}", vertexArrayObject.object));
    }

    glBindVertexArray(vertexArrayObject.object);
}

void GPUMesh::draw() const {
    bind();
    glDrawElements(primative, elementCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

/*
 * Mesh
 */

void ChunkMesh::pushFace(glm::vec3 position0, glm::vec3 position1, glm::vec3 position2, glm::vec3 position3,
    glm::vec2 texcoord0, glm::vec2 texcoord1, glm::vec4 color, glm::vec3 normal, int boneId
) {
    pushQuad(
        std::make_tuple(position0, texcoord0, normal, color, glm::ivec1{boneId}),
        std::make_tuple(position1, glm::vec2{texcoord0.x, texcoord1.y}, normal, color, glm::ivec1{boneId}),
        std::make_tuple(position2, texcoord1, normal, color, glm::ivec1{boneId}),
        std::make_tuple(position3, glm::vec2{texcoord1.x, texcoord0.y}, normal, color, glm::ivec1{boneId})
    );
}

void ChunkMesh::pushFace(glm::vec3 position0, glm::vec3 position1, glm::vec3 position2, glm::vec3 position3,
    std::pair<glm::vec2, glm::vec2> texcoord, glm::vec4 color, glm::vec3 normal, int boneId
) {
    pushFace(position0, position1, position2, position3, texcoord.first, texcoord.second, color, normal, boneId);
}

void ChunkMesh::pushTexturedPrism(glm::mat4 transformation,
    std::span<const std::pair<glm::vec2, glm::vec2>, 6> texcoords, int boneId
) {
    glm::vec4 color = color::white;

    glm::vec3 vlbb = transformation * glm::vec4{-0.5, -0.5f, -0.5f, 1.f};
    glm::vec3 vlbf = transformation * glm::vec4{-0.5, -0.5f,  0.5f, 1.f};
    glm::vec3 vltb = transformation * glm::vec4{-0.5,  0.5f, -0.5f, 1.f};
    glm::vec3 vltf = transformation * glm::vec4{-0.5,  0.5f,  0.5f, 1.f};
    glm::vec3 vrbb = transformation * glm::vec4{ 0.5, -0.5f, -0.5f, 1.f};
    glm::vec3 vrbf = transformation * glm::vec4{ 0.5, -0.5f,  0.5f, 1.f};
    glm::vec3 vrtb = transformation * glm::vec4{ 0.5,  0.5f, -0.5f, 1.f};
    glm::vec3 vrtf = transformation * glm::vec4{ 0.5,  0.5f,  0.5f, 1.f};

    pushFace(vltf, vlbf, vrbf, vrtf, texcoords[0], color, {0.f, 0.f, 1.f}, boneId);
    pushFace(vrtb, vrbb, vlbb, vltb, texcoords[1], color, {0.f, 0.f, -1.f}, boneId);

    pushFace(vrtf, vrbf, vrbb, vrtb, texcoords[2], color, {1.f, 0.f, 0.f}, boneId);
    pushFace(vltb, vlbb, vlbf, vltf, texcoords[3], color, {-1.f, 0.f, 0.f}, boneId);

    pushFace(vltb, vltf, vrtf, vrtb, texcoords[4], color, {0.f, 1.f, 0.f}, boneId);
    pushFace(vlbf, vlbb, vrbb, vrbf, texcoords[5], color, {0.f, -1.f, 0.f}, boneId);
}

void ChunkMesh::pushTexturedPrism(glm::mat4 transformation,
    std::span<const std::pair<glm::vec2, glm::vec2>, 3> texcoords, int boneId
) {
    std::array<std::pair<glm::vec2, glm::vec2>, 6> texcoords6{
        texcoords[0], texcoords[0],
        texcoords[1], texcoords[1],
        texcoords[2], texcoords[2],
    };

    pushTexturedPrism(transformation, texcoords6, boneId);
}

void ChunkMesh::pushTexturedPrism(glm::mat4 transformation,
    std::pair<glm::vec2, glm::vec2> texcoords, int boneId
) {
    std::array<std::pair<glm::vec2, glm::vec2>, 3> texcoords3{texcoords, texcoords, texcoords};
    pushTexturedPrism(transformation, texcoords3, boneId);
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

std::optional<GPUMesh> cubeMesh{};
std::optional<GPUMesh> rectangleMesh{};
std::optional<GPUMesh> cubeMeshWires{};


void initMeshes() {
    glGenBuffers(1, &uboGlobalsBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, uboGlobalsBlock);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GlobalsBlock), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboGlobalsBlock);

    ChunkMesh cubeCpuMesh{};
    cubeCpuMesh.pushTexturedPrism(glm::mat4{1.f}, {{0.f, 0.f}, {1.f, 1.f}});
    cubeMesh = cubeCpuMesh.upload();

    ChunkMesh rectangleCpuMesh{};
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


    /*Mesh cubeWiresCpuMesh{GL_LINES};
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);
    cubeWiresCpuMesh.pushVertex({-0.5, -0.5, -0.5}, {0, 1, 0}, {0, 0}, color::white);

    cubeWiresCpuMesh.indicies.push(0);

    cubeMeshWires = cubeWiresCpuMesh.upload();*/
}

void setGlobalsBlock(const GlobalsBlock& block) {
    glBindBuffer(GL_UNIFORM_BUFFER, uboGlobalsBlock);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GlobalsBlock), &block, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void setProjectionView(const glm::mat4& projectionView, const glm::vec3& cameraPosition) {
    GlobalsBlock globalsBlock{projectionView, cameraPosition};
    setGlobalsBlock(globalsBlock);
}

void unloadMeshes() {
    cubeMesh = std::nullopt;
    rectangleMesh = std::nullopt;
    cubeMeshWires = std::nullopt;
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

void setCullFaces(bool shouldCull) {
    if (shouldCull) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

void drawCube(ShaderProgram& shader, glm::vec3 position, glm::vec3 size) {
    glm::mat4 transform{1.f};
    transform = glm::translate(transform, position);
    transform = glm::scale(transform, size);
    shader.setModel(transform);
    cubeMesh->draw();
}

void drawRectangle(ShaderProgram& shader, glm::vec2 position, glm::vec2 size) {
    glm::mat4 transform{1.f};
    transform = glm::translate(transform, glm::vec3{position, 0.f});
    transform = glm::scale(transform, glm::vec3{size, 1.f});
    shader.setModel(transform);
    rectangleMesh->draw();
}


// From learnopengl
static void openglDebugCallback(GLenum source, GLenum type, unsigned int id,
        GLenum severity, GLsizei length, const char *message, const void *userParam
) {
    (void)length;

    static const std::map<GLenum, std::string_view> sources {
        {GL_DEBUG_SOURCE_API,             "API"},
        {GL_DEBUG_SOURCE_WINDOW_SYSTEM,   "Window System"},
        {GL_DEBUG_SOURCE_SHADER_COMPILER, "Shader Compiler"},
        {GL_DEBUG_SOURCE_THIRD_PARTY,     "Third Party"},
        {GL_DEBUG_SOURCE_APPLICATION,     "Application"},
        {GL_DEBUG_SOURCE_OTHER,           "Other"},
    };

    static const std::map<GLenum, std::string_view> types {
        {GL_DEBUG_TYPE_ERROR,               "Error"},
        {GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "Deprecated Behaviour"},
        {GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,  "Undefined Behaviour"},
        {GL_DEBUG_TYPE_PORTABILITY,         "Portability"},
        {GL_DEBUG_TYPE_PERFORMANCE,         "Performance"},
        {GL_DEBUG_TYPE_MARKER,              "Marker"},
        {GL_DEBUG_TYPE_PUSH_GROUP,          "Push Group"},
        {GL_DEBUG_TYPE_POP_GROUP,           "Pop Group"},
        {GL_DEBUG_TYPE_OTHER,               "Other"},
    };

    static const std::map<GLenum, LogLevel> severities {
        {GL_DEBUG_SEVERITY_HIGH,         LogLevel::fatal},
        {GL_DEBUG_SEVERITY_MEDIUM,       LogLevel::error},
        {GL_DEBUG_SEVERITY_LOW,          LogLevel::warning},
        {GL_DEBUG_SEVERITY_NOTIFICATION, LogLevel::info},
    };

    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) {
        return;
    }

    std::string text = std::format(
        "OpenGL({}), source: {}, type: {}: {} {}",
        id, sources.at(source), types.at(type), message, userParam
    );

    Logger::log(severities.at(severity), text);
}

void initializeOpenGLDebugContext() {
    int glFlags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &glFlags);
    if (glFlags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(openglDebugCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
        Logger::debug("OpenGL debug context initialized");
    } else {
        Logger::warning("No debug context");
    }
}
