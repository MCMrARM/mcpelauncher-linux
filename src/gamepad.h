#pragma once

#include <map>
#include <libudev.h>
#include <libevdev-1.0/libevdev/libevdev.h>

class LinuxGamepadManager {

private:
    static const int BUTTON_A = 0;
    static const int BUTTON_B = 1;
    static const int BUTTON_X = 2;
    static const int BUTTON_Y = 3;
    static const int BUTTON_DPAD_UP = 4;
    static const int BUTTON_DPAD_DOWN = 5;
    static const int BUTTON_DPAD_LEFT = 6;
    static const int BUTTON_DPAD_RIGHT = 7;
    static const int BUTTON_LB = 10;
    static const int BUTTON_RB = 11;

    struct MappingInfo {
        struct Entry {
            int button = -1;
            int stick = -1;
            bool stickY = false;
            int trigger = -1;
        };
        Entry buttons[16];
        Entry axis[16];
        std::map<int, Entry> hats[8];
    };

    struct AbsInfo {
        int min, max;
    };
    struct StickValueInfo {
        float x = 0.f, y = 0.f;
    };

    struct Device {
        int index;
        int fd = -1;
        struct libevdev* edev;
        MappingInfo mapping;
        AbsInfo axisInfo[16];
        StickValueInfo sticks[2];
        int hatValues[8];

        void assign(int fd, libevdev* edev);
        void release();

        void pool();

        void onButton(MappingInfo::Entry const& mapping, bool pressed);
    };

    struct udev* udev = nullptr;
    Device devices[4];

    int getFreeDeviceIndex();

    void onDeviceAdded(struct udev_device* dev);

    std::string getSDLJoystickGUID(struct libevdev* dev);

public:
    static LinuxGamepadManager instance;

    LinuxGamepadManager();

    void init();

    void pool();

};