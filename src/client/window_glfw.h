#pragma once

#include "window.h"
#include <GLFW/glfw3.h>

class GLFWGameWindow : public GameWindow {

private:
    GLFWwindow* window;
    double lastMouseX = 0.0, lastMouseY = 0.0;

    static int getKeyMinecraft(int keyCode);

    static void _glfwWindowSizeCallback(GLFWwindow* window, int w, int h);
    static void _glfwCursorPosCallback(GLFWwindow* window, double x, double y);
    static void _glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void _glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void _glfwCharCallback(GLFWwindow* window, unsigned int ch);
    static void _glfwWindowCloseCallback(GLFWwindow* window);

public:
    GLFWGameWindow(const std::string& title, int width, int height, GraphicsApi api);

    ~GLFWGameWindow() override;

    void setIcon(std::string const& iconPath) override;

    void show() override;

    void close() override;

    void runLoop() override;

    void setCursorDisabled(bool disabled) override;

    void setFullscreen(bool fullscreen) override;

};