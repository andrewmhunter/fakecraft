#ifndef FAKECRAFT_INPUT_HPP
#define FAKECRAFT_INPUT_HPP

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

void inputStatePrepare();

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

bool keyDown(int key);
bool keyPressed(int key);

bool mouseButtonDown(int button);
bool mouseButtonPressed(int button);

glm::dvec2 getMouseDelta();

double getMouseScroll();

#endif

