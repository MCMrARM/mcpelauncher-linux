#pragma once

#include <map>
#include <libudev.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include "joystick.hh"
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
    static const int BUTTON_LS = 8; 
    static const int BUTTON_RS = 9;
    static const int BUTTON_LB = 10; 
    static const int BUTTON_RB = 11; 
    static const int BUTTON_SELECT = 12; 
    static const int BUTTON_START = 13;

    enum Stick {LEFT_STICK=0, RIGHT_STICK=1};
    enum Trigger {LEFT_TRIGGER=0, RIGHT_TRIGGER=1};

    int LAXIS_X=0, 
        LAXIS_Y=1, 
        MUTUAL_AXIS=-1, 
        RAXIS_X= 3, 
        RAXIS_Y=4;

    struct StickValueInfo {
        float x = 0.f, y = 0.f;
    };

    //int getFreeDeviceIndex();

    //void onDeviceAdded(struct udev_device* dev);

    std::string getSDLJoystickGUID(struct libevdev* dev);

public:
    static LinuxGamepadManager instance;
    LinuxGamepadManager();
    void init();
    void pool();
    
    Joystick *joystick;
    int lastdpad[2]; 
    StickValueInfo sticks[2];
    int buttons[16];
    float round(float v);

};
