#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <cstddef>
#include <glad/glad.h>
#include <glm/detail/qualifier.hpp>
#include <glm/glm.hpp>
#include <map>
#include <span>
#include <string>
#include <vector>
#include <cassert>
#include <span>
#include <cstdint>
#include <functional>
#include <optional>

namespace color {
    static constexpr glm::vec4 fromRGBA(std::uint32_t hexcode) {
        return glm::vec4{
            ((hexcode >> 24) & 0xff) / 255.f,
            ((hexcode >> 16) & 0xff) / 255.f,
            ((hexcode >> 8) & 0xff) / 255.f,
            (hexcode & 0xff) / 255.f
        };
    }

    static constexpr glm::vec4 fromRGB(std::uint32_t hexcode) {
        return fromRGBA((hexcode << 8) | 0xff);
    }

    static constexpr glm::vec4 fromComponents(glm::ivec4 components) {
        return glm::vec4{components} / 255.f;
    }

    static constexpr glm::vec4 fromComponents(glm::ivec3 components) {
        return fromComponents(glm::ivec4{components, 1});
    }

    static constexpr glm::vec4 fromComponents(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255) {
        return fromComponents(glm::ivec4{red, green, blue, alpha});
    }

    static constexpr glm::vec4 white{1.f, 1.f, 1.f, 1.f};
    static constexpr glm::vec4 black{0.f, 0.f, 0.f, 1.f};
    static constexpr glm::vec4 gray{0.5f, 0.5f, 0.5f, 1.f};

    static constexpr glm::vec4 red{1.f, 0.f, 0.f, 1.f};
    static constexpr glm::vec4 green{0.f, 1.f, 0.f, 1.f};
    static constexpr glm::vec4 blue{0.f, 0.f, 1.f, 1.f};

    static constexpr glm::vec4 cyan{0.f, 1.f, 1.f, 1.f};
    static constexpr glm::vec4 magenta{1.f, 0.f, 1.f, 1.f};
    static constexpr glm::vec4 yellow{1.f, 1.f, 0.f, 1.f};

    static constexpr glm::vec4 skyblue = fromComponents(102, 191, 255);
}

using OpenGLObjectLifetimeFunction = std::function<void(GLint, GLuint*)>;

class OpenGLObject {
private:
    OpenGLObjectLifetimeFunction freeFunction;
    static constexpr GLuint InvalidValue = 0;

public:
    GLuint object;

    OpenGLObject(OpenGLObjectLifetimeFunction allocFunction, OpenGLObjectLifetimeFunction freeFunction);

    OpenGLObject(OpenGLObject&& other);
    OpenGLObject(const OpenGLObject& other) = delete;

    OpenGLObject& operator=(OpenGLObject&& other);
    OpenGLObject& operator=(const OpenGLObject& other) = delete;

    ~OpenGLObject();
};


class Image {
public:
    int width;
    int height;
    std::uint8_t* data;

    Image(std::string fileName);

    Image(const Image& image) = delete;
    Image(Image&& image);

    Image& operator=(const Image& image) = delete;
    Image& operator=(Image&& image);

    ~Image();

    glm::vec4 getPixel(glm::ivec2 position) const;
    glm::vec4 getPixel(int x, int y) const;
};

class Texture {
private:
    OpenGLObject textureId;

public:
    Texture(const std::string& fileName);
    Texture(const Image& image);

    void bind();
    void bind(int textureUnit);
};

class Shader {
private:
    Shader(GLenum shaderType, const std::string& source);

public:
    OpenGLObject shaderId;

    static Shader fromSource(GLenum shaderType, const std::string& source);
    static Shader fromFile(GLenum shaderType, const std::string& fileName);
};

class ShaderProgram {
private:
    OpenGLObject programId;
    std::map<std::string, GLint> uniformCache{};

    GLint uniformLocation(const std::string& name);

public:
    ShaderProgram(std::span<std::reference_wrapper<const Shader>> shaders);

    static ShaderProgram loadFiles(const std::string& vertexFileName, const std::string& fragmentFileName);

    void use() const;

    void setUniformFloat(const std::string& uniform, float value);
    void setUniformInt(const std::string& uniform, int value);
    void setUniformUInt(const std::string& uniform, unsigned int value);
    void setUniformVec3(const std::string& uniform, glm::vec3 value);
    void setUniformVec4(const std::string& uniform, glm::vec4 value);
    void setUniformMat4(const std::string& uniform, glm::mat4 value);
};

/*class ShaderProgram {
private:
    GLuint shaderProgram;

    ShaderProgram(const std::string& vertexShaderString, const std::string& fragmentShaderString);

    GLuint loadShader(const std::string& fileName, GLenum shaderType) const;
    GLint uniformLocation(const std::string& name);

public:
    static ShaderProgram buildFiles(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);
    static ShaderProgram buildStrings(const std::string& vertexShader, const std::string& fragmentShader);

    void setUniformFloat(const std::string& uniform, float value);
    void setUniformInt(const std::string& uniform, int value);
    void setUniformUInt(const std::string& uniform, unsigned int value);
    void setUniformVec3(const std::string& uniform, glm::vec3 value);
    void setUniformVec4(const std::string& uniform, glm::vec4 value);
    void setUniformMat4(const std::string& uniform, glm::mat4 value);

    void use() const;
    void unload();
};*/

class GPUMesh {
private:
public:
    GLenum primative;
    OpenGLObject vertexArrayObject;
    OpenGLObject vertexBufferObject;
    OpenGLObject elementBufferObject;
    int elementCount;

    GPUMesh(GLenum primative);
    //GPUMesh();

    void bind() const;

    void draw() const;
};

template<typename T, GLenum GLType>
class VecVertexData {
private:
    using VecType = T;
    std::vector<typename T::value_type> contents;

public:
    void push(typename T::value_type value) {
        contents.push_back(value);
    }

    void push(T vec) {
        for (int i = 0; i < T::length(); ++i) {
            push(vec[i]);
        }
    }

    const typename T::value_type* data() const {
        return contents.data();
    }

    std::span<const typename T::value_type> getSpan() const {
        return contents;
    }

    std::size_t elementLength() const {
        return contents.size();
    }

    std::size_t vertexLength() const {
        assert(contents.size() % T::length() == 0);
        return elementLength() / T::length();
    }

    std::size_t sizeBytes() const {
        return elementLength() * sizeof(typename T::value_type);
    }

    std::size_t stride() const {
        return T::length() * sizeof(typename T::value_type);
    }

    bool hasData() const {
        return elementLength() != 0;
    }

    std::size_t bufferData(GLuint attributeIndex, std::size_t offset) const {
        if (!hasData()) {
            return offset;
        }

        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeBytes(), data());
        glVertexAttribPointer(attributeIndex, T::length(), GLType, false, stride(), (void*)offset);
        glEnableVertexAttribArray(attributeIndex);
        return offset + sizeBytes();
    }
};

class Mesh {
public:
    GLenum primative{GL_TRIANGLES};

    VecVertexData<glm::vec3, GL_FLOAT> positions;
    VecVertexData<glm::vec3, GL_FLOAT> normals;
    VecVertexData<glm::vec2, GL_FLOAT> texcoords;
    VecVertexData<glm::vec4, GL_FLOAT> colors;
    VecVertexData<glm::uvec3, GL_UNSIGNED_INT> indicies;

    Mesh(GLenum primative);
    Mesh();

    GPUMesh upload() const;
    void reupload(GPUMesh& gpuMesh) const;

    void pushVertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texcoord, glm::vec4 color);
    void pushFace(glm::vec3 position0, glm::vec3 position1, glm::vec3 position2, glm::vec3 position3,
        glm::vec2 texcoord0, glm::vec2 texcoord1, glm::vec4 color, glm::vec3 normal);
    void pushFace(glm::vec3 position0, glm::vec3 position1, glm::vec3 position2, glm::vec3 position3,
        std::pair<glm::vec2, glm::vec2> texcoord, glm::vec4 color, glm::vec3 normal);

    void makeTriangle(int offset0, int offset1, int offset2);

    void pushTexturedPrism(glm::mat4 transformation,
        std::span<const std::pair<glm::vec2, glm::vec2>, 6> texcoords);

    void pushTexturedPrism(glm::mat4 transformation,
        std::span<const std::pair<glm::vec2, glm::vec2>, 3> texcoords);

    void pushTexturedPrism(glm::mat4 transformation,
        std::pair<glm::vec2, glm::vec2> texcoords);
};

void wireframeEnable();
void wireframeDisable();

void blendModeInvert();
void blendModeNormal();
void blendModeReplace();

void drawCube(ShaderProgram& shader, glm::vec3 position, glm::vec3 size);
void drawRectangle(ShaderProgram& shader, glm::vec2 position, glm::vec2 size);

void initMeshes();
void unloadMeshes();

extern std::optional<GPUMesh> cubeMesh;
extern std::optional<GPUMesh> rectangleMesh;
extern std::optional<GPUMesh> cubeMeshWires;

void initializeOpenGLDebugContext();

#endif

