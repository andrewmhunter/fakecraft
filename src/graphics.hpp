#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <span>
#include <string>
#include <vector>
#include <cassert>
#include <span>
#include <cstdint>

namespace color {
    static inline constexpr glm::vec4 fromRGB(std::uint32_t hexcode) {
        return glm::vec4{
            ((hexcode >> 16) & 0xff) / 255.f,
            ((hexcode >> 8) & 0xff) / 255.f,
            (hexcode & 0xff) / 255.f, 1.f
        };
    }

    static inline constexpr glm::vec4 fromRGBA(std::uint32_t hexcode) {
        return glm::vec4{
            ((hexcode >> 24) & 0xff) / 255.f,
            ((hexcode >> 16) & 0xff) / 255.f,
            ((hexcode >> 8) & 0xff) / 255.f,
            (hexcode & 0xff) / 255.f
        };
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

    static constexpr glm::vec4 skyblue = glm::vec4{102, 191, 255, 255} / 255.f;
}

template<void (*Alloc)(GLint, GLuint*), void (*Free)(GLint, GLuint*)>
class OpenGLObject {
private:
    static constexpr GLuint InvalidValue = 0;
    using Type = OpenGLObject<Alloc, Free>;

public:
    GLuint object;

    OpenGLObject() {
        Alloc(1, &this->object);
    }

    OpenGLObject(Type&& other) : object{other.object} {
        other.object = InvalidValue;
    }

    OpenGLObject(const Type& other) = delete;

    Type& operator=(Type&& other) {
        if (this == &other) {
            return *this;
        }

        if (object != InvalidValue) {
            Free(1, &object);
        }

        object = other.object;
        other.object = InvalidValue;

        return *this;
    }

    Type& operator=(const Type& other) = delete;


    ~OpenGLObject() {
        if (object != InvalidValue) {
            Free(1, &object);
        }
    }
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

    glm::vec4 getPixel(int x, int y) const;
};

class Texture {
private:
    GLuint textureId;
public:
    Texture(const std::string& fileName);
    Texture(const Image& image);
    Texture();

    void bind();
    void bind(int textureUnit);
    void unload();
};

class Shader {
private:
    GLuint shaderProgram;

    Shader(const std::string& vertexShaderString, const std::string& fragmentShaderString);

    GLuint loadShader(const std::string& fileName, GLenum shaderType) const;
    GLint uniformLocation(const std::string& name);

public:
    static Shader buildFiles(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);
    static Shader buildStrings(const std::string& vertexShader, const std::string& fragmentShader);

    void setUniformFloat(const std::string& uniform, float value);
    void setUniformInt(const std::string& uniform, int value);
    void setUniformUInt(const std::string& uniform, unsigned int value);
    void setUniformVec3(const std::string& uniform, glm::vec3 value);
    void setUniformVec4(const std::string& uniform, glm::vec4 value);
    void setUniformMat4(const std::string& uniform, glm::mat4 value);

    void use() const;
    void unload();
};

class GPUMesh {
private:
public:
    GLenum primative;
    GLuint vertexArrayObject;
    GLuint vertexBufferObject;
    GLuint elementBufferObject;
    int elementCount;

    GPUMesh(GLenum primative, GLuint vertexArrayObject, GLuint vertexBufferObject, GLuint elementBufferObject, int elementCount);
    GPUMesh();

    static GPUMesh generate(GLenum primative);

    void bind() const;
    void unload();

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

void drawCube(Shader shader, glm::vec3 position, glm::vec3 size);
void drawRectangle(Shader shader, glm::vec2 position, glm::vec2 size);

void initMeshes();

extern GPUMesh cubeMesh;
extern GPUMesh rectangleMesh;
extern GPUMesh cubeMeshWires;


#endif

