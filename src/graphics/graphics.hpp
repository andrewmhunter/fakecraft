#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <cstddef>
#include <glad/glad.h>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
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
    using Color = glm::vec4;

    static constexpr Color fromRGBA(std::uint32_t hexcode) {
        return Color{
            ((hexcode >> 24) & 0xff) / 255.f,
            ((hexcode >> 16) & 0xff) / 255.f,
            ((hexcode >> 8) & 0xff) / 255.f,
            (hexcode & 0xff) / 255.f
        };
    }

    static constexpr Color fromRGB(std::uint32_t hexcode) {
        return fromRGBA((hexcode << 8) | 0xff);
    }

    static constexpr Color fromComponents(glm::ivec4 components) {
        return glm::vec4{components} / 255.f;
    }

    static constexpr Color fromComponents(glm::ivec3 components) {
        return fromComponents(glm::ivec4{components, 1});
    }

    static constexpr Color fromComponents(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255) {
        return fromComponents(glm::ivec4{red, green, blue, alpha});
    }

    static constexpr Color white{1.f, 1.f, 1.f, 1.f};
    static constexpr Color black{0.f, 0.f, 0.f, 1.f};
    static constexpr Color gray{0.5f, 0.5f, 0.5f, 1.f};

    static constexpr Color red{1.f, 0.f, 0.f, 1.f};
    static constexpr Color green{0.f, 1.f, 0.f, 1.f};
    static constexpr Color blue{0.f, 0.f, 1.f, 1.f};

    static constexpr Color cyan{0.f, 1.f, 1.f, 1.f};
    static constexpr Color magenta{1.f, 0.f, 1.f, 1.f};
    static constexpr Color yellow{1.f, 1.f, 0.f, 1.f};

    static constexpr Color skyblue = fromComponents(102, 191, 255);
}


template<typename T>
constexpr GLenum getGLEnumType() {
    static_assert(false, "Type does not have a supported OpenGL enum");
    return 0;
}


template<>
constexpr GLenum getGLEnumType<GLfloat>() {
    return GL_FLOAT;
}

template<>
constexpr GLenum getGLEnumType<GLdouble>() {
    return GL_DOUBLE;
}

template<>
constexpr GLenum getGLEnumType<GLbyte>() {
    return GL_BYTE;
}

template<>
constexpr GLenum getGLEnumType<GLubyte>() {
    return GL_UNSIGNED_BYTE;
}

template<>
constexpr GLenum getGLEnumType<GLshort>() {
    return GL_SHORT;
}

template<>
constexpr GLenum getGLEnumType<GLushort>() {
    return GL_UNSIGNED_SHORT;
}

template<>
constexpr GLenum getGLEnumType<GLint>() {
    return GL_INT;
}

template<>
constexpr GLenum getGLEnumType<GLuint>() {
    return GL_UNSIGNED_INT;
}


constexpr int uboNBaseAlignment = 4;
constexpr int uboVec4BaseAlignment = uboNBaseAlignment * 4;

struct GlobalsBlock {
    alignas(uboVec4BaseAlignment) glm::mat4 projectionView;
    alignas(uboVec4BaseAlignment) glm::vec3 cameraPosition;
};

void setGlobalsBlock(const GlobalsBlock& block);
void setProjectionView(const glm::mat4& projectionView, const glm::vec3& cameraPosition = glm::vec3{0.f});

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

    void setModel(glm::mat4 model);
    void setColor(glm::vec4 color);

    GLuint getId() const;
};

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

template<typename T, glm::length_t L = 1>
class VertexData {
private:
    std::vector<T> contents;

public:
    void push(T value) {
        contents.push_back(value);
    }

    const T* data() const {
        return contents.data();
    }

    std::span<const T> getSpan() const {
        return contents;
    }

    std::size_t elementLength() const {
        return contents.size();
    }

    std::size_t vertexLength() const {
        assert(contents.size() % L == 0);
        return elementLength() / L;
    }

    std::size_t sizeBytes() const {
        return elementLength() * sizeof(T);
    }

    std::size_t stride() const {
        return L * sizeof(T);
    }

    bool hasData() const {
        return elementLength() != 0;
    }

    std::size_t bufferData(GLuint attributeIndex, std::size_t offset) const {
        if (!hasData()) {
            return offset;
        }

        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeBytes(), data());
        if (std::is_integral_v<T>) {
            glVertexAttribIPointer(attributeIndex, L, getGLEnumType<T>(), stride(), (void*)offset);
        } else {
            glVertexAttribPointer(attributeIndex, L, getGLEnumType<T>(), false, stride(), (void*)offset);
        }
        glEnableVertexAttribArray(attributeIndex);
        return offset + sizeBytes();
    }
};

template<typename T>
class VecVertexData : public VertexData<typename T::value_type, T::length()> {
public:
    void pushVec(T vec) {
        for (int i = 0; i < T::length(); ++i) {
            this->push(vec[i]);
        }
    }
};


constexpr int maxBones = 8;

class Mesh {
public:
    GLenum primative{GL_TRIANGLES};

    VecVertexData<glm::vec3> positions;
    VecVertexData<glm::vec3> normals;
    VecVertexData<glm::vec2> texcoords;
    VecVertexData<glm::vec4> colors;
    VertexData<int> boneIds;
    VecVertexData<glm::uvec3> indicies;

    Mesh(GLenum primative);
    Mesh();

    GPUMesh upload() const;

    void pushVertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texcoord, glm::vec4 color, int boneId = 0);
    void pushFace(glm::vec3 position0, glm::vec3 position1, glm::vec3 position2, glm::vec3 position3,
        glm::vec2 texcoord0, glm::vec2 texcoord1, glm::vec4 color, glm::vec3 normal, int boneId = 0);
    void pushFace(glm::vec3 position0, glm::vec3 position1, glm::vec3 position2, glm::vec3 position3,
        std::pair<glm::vec2, glm::vec2> texcoord, glm::vec4 color, glm::vec3 normal, int boneId = 0);

    void makeTriangle(int offset0, int offset1, int offset2);

    void pushTexturedPrism(glm::mat4 transformation,
        std::span<const std::pair<glm::vec2, glm::vec2>, 6> texcoords, int boneId = 0);

    void pushTexturedPrism(glm::mat4 transformation,
        std::span<const std::pair<glm::vec2, glm::vec2>, 3> texcoords, int boneId = 0);

    void pushTexturedPrism(glm::mat4 transformation,
        std::pair<glm::vec2, glm::vec2> texcoords, int boneId = 0);
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

