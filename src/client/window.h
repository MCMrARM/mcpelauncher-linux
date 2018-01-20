#pragma once

#include <string>
#include <functional>

enum class GraphicsApi {
    OPENGL, OPENGL_ES2
};
enum class KeyAction {
    PRESS, REPEAT, RELEASE
};
enum class MouseButtonAction {
    PRESS, RELEASE
};

class GameWindow {

public:
    using DrawCallback = std::function<void ()>;
    using WindowSizeCallback = std::function<void (int, int)>;
    using MouseButtonCallback = std::function<void (double, double, int, MouseButtonAction)>;
    using MousePositionCallback = std::function<void (double, double)>;
    using MouseScrollCallback = std::function<void (double, double, double, double)>;
    using KeyboardCallback = std::function<void (int, KeyAction)>;
    using KeyboardTextCallback = std::function<void (std::string const&)>;
    using PasteCallback = std::function<void (std::string const&)>;
    using CloseCallback = std::function<void ()>;

private:
    DrawCallback drawCallback;
    WindowSizeCallback windowSizeCallback;
    MouseButtonCallback mouseButtonCallback;
    MousePositionCallback mousePositionCallback, mouseRelativePositionCallback;
    MouseScrollCallback mouseScrollCallback;
    KeyboardCallback keyboardCallback;
    KeyboardTextCallback keyboardTextCallback;
    PasteCallback pasteCallback;
    CloseCallback closeCallback;

public:

    GameWindow(std::string const& title, int width, int height, GraphicsApi api) {}

    virtual ~GameWindow() {}

    virtual void setIcon(std::string const& iconPath) = 0;

    virtual void show() = 0;

    virtual void close() = 0;

    virtual void runLoop() = 0;

    virtual void setCursorDisabled(bool disabled) = 0;

    virtual void setFullscreen(bool fullscreen) = 0;

    void setDrawCallback(DrawCallback callback) { drawCallback = std::move(callback); }

    void setWindowSizeCallback(WindowSizeCallback callback) { windowSizeCallback = std::move(callback); }

    void setMouseButtonCallback(MouseButtonCallback callback) { mouseButtonCallback = std::move(callback); }

    void setMousePositionCallback(MousePositionCallback callback) { mousePositionCallback = std::move(callback); }

    void setMouseScrollCallback(MouseScrollCallback callback) { mouseScrollCallback = std::move(callback); }

    void setKeyboardCallback(KeyboardCallback callback) { keyboardCallback = std::move(callback); }

    void setKeyboardTextCallback(KeyboardTextCallback callback) { keyboardTextCallback = std::move(callback); }

    void setPasteCallback(PasteCallback callback) { pasteCallback = std::move(callback); }

    // Used when the cursor is disabled
    void setMouseRelativePositionCallback(MousePositionCallback callback) { mouseRelativePositionCallback = std::move(callback); }

    void setCloseCallback(CloseCallback callback) { closeCallback = std::move(callback); }


protected:

    void onDraw() {
        if (drawCallback != nullptr)
            drawCallback();
    }
    void onWindowSizeChanged(int w, int h) {
        if (windowSizeCallback != nullptr)
            windowSizeCallback(w, h);
    }
    void onMouseButton(double x, double y, int button, MouseButtonAction action) {
        if (mouseButtonCallback != nullptr)
            mouseButtonCallback(x, y, button, action);
    }
    void onMousePosition(double x, double y) {
        if (mousePositionCallback != nullptr)
            mousePositionCallback(x, y);
    }
    void onMouseRelativePosition(double x, double y) {
        if (mouseRelativePositionCallback != nullptr)
            mouseRelativePositionCallback(x, y);
    }
    void onMouseScroll(double x, double y, double dx, double dy) {
        if (mouseScrollCallback != nullptr)
            mouseScrollCallback(x, y, dx, dy);
    }
    void onKeyboard(int key, KeyAction action) {
        if (keyboardCallback != nullptr)
            keyboardCallback(key, action);
    }
    void onKeyboardText(std::string const& c) {
        if (keyboardTextCallback != nullptr)
            keyboardTextCallback(c);
    }
    void onPaste(std::string const& c) {
        if (pasteCallback != nullptr)
            pasteCallback(c);
    }
    void onClose() {
        if (closeCallback != nullptr)
            closeCallback();
    }

};
