#include "window_glfw.h"


GLFWGameWindow::GLFWGameWindow(const std::string& title, int width, int height, GraphicsApi api) :
        GameWindow(title, width, height, api) {
    glfwDefaultWindowHints();
    if (api == GraphicsApi::OPENGL_ES2) {
        glfwWindowHint(GLFW_OPENGL_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    }
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfwMakeContextCurrent(window);
}

GLFWGameWindow::~GLFWGameWindow() {
    glfwDestroyWindow(window);
}

void GLFWGameWindow::setIcon(std::string const& iconPath) {
    // TODO:
}

void GLFWGameWindow::show() {
    glfwShowWindow(window);
}

void GLFWGameWindow::close() {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void GLFWGameWindow::runLoop() {
    while (!glfwWindowShouldClose(window)) {
        onDraw();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void GLFWGameWindow::setCursorDisabled(bool disabled) {
    glfwSetInputMode(window, GLFW_CURSOR, disabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void GLFWGameWindow::setFullscreen(bool fullscreen) {
    // TODO:
}