#include "window_glfw.h"

#include <codecvt>
#include <iomanip>

GLFWGameWindow::GLFWGameWindow(const std::string& title, int width, int height, GraphicsApi api) :
        GameWindow(title, width, height, api) {
    glfwDefaultWindowHints();
    if (api == GraphicsApi::OPENGL_ES2) {
        glfwWindowHint(GLFW_OPENGL_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    }
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetWindowSizeCallback(window, _glfwWindowSizeCallback);
    glfwSetCursorPosCallback(window, _glfwCursorPosCallback);
    glfwSetMouseButtonCallback(window, _glfwMouseButtonCallback);
    glfwSetWindowCloseCallback(window, _glfwWindowCloseCallback);
    glfwSetKeyCallback(window, _glfwKeyCallback);
    glfwSetCharCallback(window, _glfwCharCallback);
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
    glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
}

void GLFWGameWindow::setFullscreen(bool fullscreen) {
    // TODO:
}

void GLFWGameWindow::_glfwWindowSizeCallback(GLFWwindow* window, int w, int h) {
    GLFWGameWindow* user = (GLFWGameWindow*) glfwGetWindowUserPointer(window);
    user->onWindowSizeChanged(w, h);
}

void GLFWGameWindow::_glfwCursorPosCallback(GLFWwindow* window, double x, double y) {
    GLFWGameWindow* user = (GLFWGameWindow*) glfwGetWindowUserPointer(window);
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
        user->onMouseRelativePosition(x - user->lastMouseX, y - user->lastMouseY);
        user->lastMouseX = x;
        user->lastMouseY = y;
    } else {
        user->onMousePosition(x, y);
    }
}

void GLFWGameWindow::_glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    GLFWGameWindow* user = (GLFWGameWindow*) glfwGetWindowUserPointer(window);
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    user->onMouseButton(x, y, button + 1, action == GLFW_PRESS ? MouseButtonAction::PRESS : MouseButtonAction::RELEASE);
}

int GLFWGameWindow::getKeyMinecraft(int keyCode) {
    if (keyCode == GLFW_KEY_ESCAPE)
        return 27;
    if (keyCode == GLFW_KEY_LEFT_SHIFT)
        return 16;
    return keyCode;
}

void GLFWGameWindow::_glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    GLFWGameWindow* user = (GLFWGameWindow*) glfwGetWindowUserPointer(window);
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_BACKSPACE)
            user->onKeyboardText("\x08");
        if (key == GLFW_KEY_ENTER)
            user->onKeyboardText("\n");
    }
    KeyAction enumAction = (action == GLFW_PRESS ? KeyAction::PRESS :
                            (action == GLFW_REPEAT ? KeyAction::REPEAT : KeyAction::RELEASE));
    user->onKeyboard(getKeyMinecraft(key), enumAction);
}

void GLFWGameWindow::_glfwCharCallback(GLFWwindow* window, unsigned int ch) {
    GLFWGameWindow* user = (GLFWGameWindow*) glfwGetWindowUserPointer(window);
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
    user->onKeyboardText(cvt.to_bytes(ch));
}

void GLFWGameWindow::_glfwWindowCloseCallback(GLFWwindow* window) {
    GLFWGameWindow* user = (GLFWGameWindow*) glfwGetWindowUserPointer(window);
    glfwSetWindowShouldClose(window, GLFW_FALSE);
    user->onClose();
}