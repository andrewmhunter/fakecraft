#include <set>
#include <GLFW/glfw3.h>
#include "input.hpp"

std::set<int> keysPressed{};
std::set<int> mouseButtonsPressed{};

glm::dvec2 lastMousePosition{0.};
glm::dvec2 mouseDelta{0.};

double scrollValue = 0.;

void inputStatePrepare() {
    glm::dvec2 currentMousePosition{};
    glfwGetCursorPos(glfwGetCurrentContext(), &currentMousePosition.x, &currentMousePosition.y);
    mouseDelta = currentMousePosition - lastMousePosition;
    lastMousePosition = currentMousePosition;

    keysPressed.clear();
    mouseButtonsPressed.clear();
    scrollValue = 0.;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)window;
    (void)scancode;
    (void)mods;

    if (action == GLFW_PRESS) {
        keysPressed.insert(key);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    (void)window;
    (void)mods;

    if (action == GLFW_PRESS) {
        mouseButtonsPressed.insert(button);
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    (void)xpos;
    (void)ypos;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    (void)xoffset;

    scrollValue = yoffset;
}


bool keyDown(int key) {
    return glfwGetKey(glfwGetCurrentContext(), key) == GLFW_PRESS;
}

bool keyPressed(int key) {
    return keysPressed.contains(key);
}

bool mouseButtonDown(int button) {
    return glfwGetMouseButton(glfwGetCurrentContext(), button) == GLFW_PRESS;
}

bool mouseButtonPressed(int button) {
    return mouseButtonsPressed.contains(button);
}

glm::dvec2 getMouseDelta() {
    return mouseDelta;
}

double getMouseScroll() {
    return scrollValue;
}

