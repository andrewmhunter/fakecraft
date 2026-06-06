#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <string>
#include <iostream>
#include "src_cpp/graphics.hpp"

std::string vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexcoord;\n"
    "layout (location = 3) in vec4 aColor;\n"
    "uniform mat4 mvp;\n"
    "out vec4 fColor;\n"
    "out vec2 fTexcoord;\n"
    "void main()\n"
    "{\n"
    "   fColor = aColor;\n"
    "   fTexcoord = aTexcoord;\n"
    "   gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\n";

std::string fragmentShaderSource = "#version 330 core\n"
    "in vec4 fColor;\n"
    "in vec2 fTexcoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D theTexture;\n"
    "void main()\n"
    "{\n"
    "   FragColor = fColor * texture(theTexture, fTexcoord);\n"
    "}\n";

int main() {
    if (!glfwInit()) {
        return 1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Game", nullptr, nullptr);
    if (window == nullptr) {
        return 1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return 1;
    }

    //stbi_set_flip_vertically_on_load(true);

    initMeshes();

    Shader program = Shader::buildStrings(vertexShaderSource, fragmentShaderSource);

    Mesh cmesh{};

    cmesh.positions.push(glm::vec3{0.f, 0.5f, 0.f});
    cmesh.positions.push(glm::vec3{0.5f, -0.5f, 0.f});
    cmesh.positions.push(glm::vec3{-0.5f, -0.5f, 0.f});

    cmesh.colors.push(glm::vec4{1.f, 0.f, 0.f, 1.f});
    cmesh.colors.push(glm::vec4{0.f, 1.f, 0.f, 1.f});
    cmesh.colors.push(glm::vec4{0.f, 0.f, 1.f, 1.f});

    cmesh.makeTriangle(-2, -1, 0);
    //cmesh.indicies.push(glm::uvec3{0, 1, 2});

    GPUMesh gmesh = cmesh.upload();

    Mesh cmesh2{};
    cmesh2.pushFace({-0.5f, 0.5f, 0.f}, {-0.5f, -0.5f, 0.f}, {0.5f, -0.5f, 0.f}, {0.5f, 0.5f, 0.f},
            {0.f, 0.f}, {1.f, 1.f}, glm::vec4{1.f}, glm::vec3{0.f, 0.f, 1.f});
    GPUMesh gmesh2 = cmesh2.upload();

    Texture texture{"assets/resources/terrain.png"};

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glViewport(0, 0, 800, 600);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    float angle = 0.f;

    glm::vec3 target = glm::vec3{0.f};
    glm::vec3 up = glm::vec3{0.f, 1.f, 0.f};
    glm::vec3 position = glm::vec3{0.f, 1.f, 1.f}; 
    //position = glm::vec3{0.f, -1.f, 1.f}; 

    glm::mat4 view = glm::lookAt(position, target, up);
    glm::mat4 projection = glm::perspective(glm::radians(70.f), 800.f / 600.f, 0.1f, 1000.f);

    const int count = 2;
    GLuint textures[count];
    glGenTextures(count, textures);

    double startTime = glfwGetTime();

    for (int i = 0; i < 1000; ++i) {
        glBindTexture(GL_TEXTURE_2D, textures[i % count]);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    double endTime = glfwGetTime();

    std::cout << "Time: " << (endTime - startTime) << '\n';

    glfwTerminate();
    return 0;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = glm::lookAt(glm::vec3{glm::rotate(glm::mat4{1.f}, angle, up) * glm::vec4{position, 1.f}}, target, up);

        program.use();
        texture.bind();
        program.setUniformMat4("mvp", projection * view);

        //gmesh2.draw();
        cubeMesh.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();

        angle += 0.005f;
    }

    glfwTerminate();

    return 0;
}

