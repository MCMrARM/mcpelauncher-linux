#pragma once

#include "window.h"
#include <GLFW/glfw3.h>

class GLFWGameWindow : public GameWindow {

private:
    GLFWwindow* window;

/*
    static int getKeyMinecraft(int keyCode);

    static void _eglutIdleFunc();
    static void _eglutDisplayFunc();
    static void _eglutReshapeFunc(int w, int h);
    static void _eglutMouseFunc(int x, int y);
    static void _eglutMouseButtonFunc(int x, int y, int btn, int action);
    static void _eglutKeyboardFunc(char str[5], int action);
    static void _eglutKeyboardSpecialFunc(int key, int action);
    static void _eglutPasteFunc(const char* str, int len);
    static void _eglutCloseWindowFunc();*/

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