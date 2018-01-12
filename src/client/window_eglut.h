#pragma once

#include "window.h"

class EGLUTWindow : public GameWindow {

private:
    static EGLUTWindow* currentWindow;

    std::string title;
    int width, height;
    GraphicsApi graphicsApi;
    std::string iconPath;
    int winId = -1;
    bool cursorDisabled = false;
    bool moveMouseToCenter = false;
    int lastMouseX = -1, lastMouseY = -1;
    bool modCTRL = false;

    static int getKeyMinecraft(int keyCode);

    static void _eglutIdleFunc();
    static void _eglutDisplayFunc();
    static void _eglutReshapeFunc(int w, int h);
    static void _eglutMouseFunc(int x, int y);
    static void _eglutMouseButtonFunc(int x, int y, int btn, int action);
    static void _eglutKeyboardFunc(char str[5], int action);
    static void _eglutKeyboardSpecialFunc(int key, int action);
    static void _eglutPasteFunc(const char* str, int len);
    static void _eglutCloseWindowFunc();

public:
    EGLUTWindow(const std::string& title, int width, int height, GraphicsApi api);

    ~EGLUTWindow() override;

    void setIcon(std::string const& iconPath) override {
        this->iconPath = iconPath;
    }

    void show() override;

    void close() override;

    void runLoop() override;

    void setCursorDisabled(bool disabled) override;

    void setFullscreen(bool fullscreen) override;

};