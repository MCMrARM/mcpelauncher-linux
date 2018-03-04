#pragma once

#include <map>
#include <memory>
#include <functional>
#include <istream>
#include <libudev.h>
#include <libevdev-1.0/libevdev/libevdev.h>

class LinuxGamepadManager {

public:
    typedef std::string GamepadTypeId;

private:
    static const int BUTTON_A = 0;
    static const int BUTTON_B = 1;
    static const int BUTTON_X = 2;
    static const int BUTTON_Y = 3;
    static const int BUTTON_DPAD_UP = 4;
    static const int BUTTON_DPAD_DOWN = 5;
    static const int BUTTON_DPAD_LEFT = 6;
    static const int BUTTON_DPAD_RIGHT = 7;
    static const int BUTTON_LEFT_STICK = 8;
    static const int BUTTON_RIGHT_STICK = 9;
    static const int BUTTON_LB = 10;
    static const int BUTTON_RB = 11;
    static const int BUTTON_SELECT = 12;
    static const int BUTTON_START = 13;

    static const char* MAPPINGS_FILE;

    struct MappingInfo {
        struct Entry {
            int button = -1;
            int stick = -1;
            bool stickY = false;
            char stickVal = 0; // -1, 0, 1
            int trigger = -1;
        };
        Entry buttons[32];
        Entry axis[16];
        std::map<int, Entry> hats[8];

        void parse(std::string const& str);
    };

    struct AbsInfo {
        int min, max;
    };
    struct StickValueInfo {
        float x = 0.f, y = 0.f;
    };

    struct Device {
        LinuxGamepadManager* manager;
        int index;
        int fd = -1;
        std::string devPath;
        struct libevdev* edev;
        std::shared_ptr<MappingInfo> mapping;
        AbsInfo axisInfo[16];
        StickValueInfo sticks[2];
        int hatValues[8];
        bool assignedInMinecraft = false;

        void assign(std::string devPath, int fd, libevdev* edev);
        void release();

        void pool();

        void onButton(MappingInfo::Entry const& mapping, bool pressed);
    };

    struct udev* udev = nullptr;
    struct udev_monitor* udevMonitor = nullptr;
    int udevMonitorFd;
    Device devices[4];
    std::shared_ptr<MappingInfo> fallbackMapping;
    std::map<GamepadTypeId, std::shared_ptr<MappingInfo>> mappings;
    std::function<void (int, int, bool)> rawButtonCallback;
    std::function<void (int, int, float)> rawStickCallback;
    std::function<void (int, int, int)> rawHatCallback;

    int getFreeDeviceIndex();

    void onDeviceAdded(struct udev_device* dev);

    void onDeviceRemoved(struct udev_device* dev);

    std::string getSDLJoystickGUID(struct libevdev* dev);

    std::string getGamepadTypeId(struct libevdev* dev);

    void parseMappings(std::istream& ifs);

public:
    static LinuxGamepadManager instance;

    LinuxGamepadManager();

    void init();

    void pool();

    GamepadTypeId getGamepadTypeId(int index);

    void setGamepadMapping(GamepadTypeId gamepad, std::string const& mapping);

    void setRawButtonCallback(std::function<void (int, int, bool)> cb) {
        rawButtonCallback = cb;
    }

    void setRawStickCallback(std::function<void (int, int, float)> cb) {
        rawStickCallback = cb;
    }

    void setRawHatCallback(std::function<void (int, int, int)> cb) {
        rawHatCallback = cb;
    }

};
