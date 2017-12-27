#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <limits>
#include <sstream>
#include <iomanip>
#include "gamepad.h"
#include "minecraft/GameControllerManager.h"

LinuxGamepadManager LinuxGamepadManager::instance;

LinuxGamepadManager::LinuxGamepadManager() {
    for (int i = 0; i < 4; i++) {
        devices[i].manager = this;
        devices[i].index = i;
    }
}

void LinuxGamepadManager::init() {
    if (!udev) {
        udev = udev_new();
        if (!udev)
            throw std::runtime_error("Failed to initialize udev");
    }
    struct udev_enumerate* enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);
    struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry* dev_list_entry;
    udev_list_entry_foreach(dev_list_entry, devices) {
        const char* path = udev_list_entry_get_name(dev_list_entry);
        struct udev_device* dev = udev_device_new_from_syspath(udev, path);
        if (dev == nullptr)
            continue;
        onDeviceAdded(dev);
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);
}

void LinuxGamepadManager::Device::assign(int fd, libevdev* edev) {
    this->fd = fd;
    this->edev = edev;

    for (unsigned int i = 0; i < 16; i++) {
        const input_absinfo* absinfo = libevdev_get_abs_info(edev, i);
        if (absinfo == nullptr)
            continue;
        axisInfo[i].min = absinfo->minimum;
        axisInfo[i].max = absinfo->maximum;
    }

    // hardcode a mapping for now
    mapping.buttons[BTN_A - BTN_GAMEPAD].button = BUTTON_A;
    mapping.buttons[BTN_B - BTN_GAMEPAD].button = BUTTON_B;
    mapping.buttons[BTN_X - BTN_GAMEPAD].button = BUTTON_X;
    mapping.buttons[BTN_Y - BTN_GAMEPAD].button = BUTTON_Y;
    mapping.buttons[BTN_TL - BTN_GAMEPAD].button = BUTTON_LB;
    mapping.buttons[BTN_TR - BTN_GAMEPAD].button = BUTTON_RB;
    mapping.buttons[BTN_SELECT - BTN_GAMEPAD].button = BUTTON_SELECT;
    mapping.buttons[BTN_START - BTN_GAMEPAD].button = BUTTON_START;
    mapping.axis[ABS_X].stick = mapping.axis[ABS_Y].stick = 0;
    mapping.axis[ABS_Y].stickY = true;
    mapping.axis[ABS_RX].stick = mapping.axis[ABS_RY].stick = 1;
    mapping.axis[ABS_RY].stickY = true;
    mapping.axis[ABS_Z].trigger = 0;
    mapping.axis[ABS_RZ].trigger = 1;
    mapping.hats[ABS_HAT0X - ABS_HAT0X][-1].button = BUTTON_DPAD_LEFT;
    mapping.hats[ABS_HAT0X - ABS_HAT0X][1].button = BUTTON_DPAD_RIGHT;
    mapping.hats[ABS_HAT0Y - ABS_HAT0X][-1].button = BUTTON_DPAD_UP;
    mapping.hats[ABS_HAT0Y - ABS_HAT0X][1].button = BUTTON_DPAD_DOWN;
}

void LinuxGamepadManager::Device::release() {
    libevdev_free(edev);
    if (fd != -1)
        close(fd);
    fd = -1;
    edev = nullptr;
    assignedInMinecraft = false;
}

void LinuxGamepadManager::Device::pool() {
    struct input_event e;
    while (true) {
        int r = libevdev_next_event(edev, LIBEVDEV_READ_FLAG_NORMAL, &e);
        if (r == -EAGAIN)
            break;
        if (r != 0) {
            printf("LinuxGamepadManager::Device::pool error\n");
            break;
        }
        if (e.type == EV_KEY) {
            int code = e.code - BTN_GAMEPAD;
            if (code >= 0 && code < 16) {
                if (manager->rawButtonCallback != nullptr)
                    manager->rawButtonCallback(index, code, e.value != 0);
                MappingInfo::Entry const& entry = mapping.buttons[code];
                onButton(entry, e.value != 0);
            }
        } else if (e.type == EV_ABS) {
            if (e.code >= ABS_X && e.code < ABS_X + 16) {
                MappingInfo::Entry const& entry = mapping.axis[e.code];
                float value = (float) e.value / axisInfo[e.code].max;
                if (value >= -0.1f && value <= 0.1f)
                    value = 0.f;
                if (manager->rawStickCallback != nullptr)
                    manager->rawStickCallback(index, e.code, value);
                if (entry.stick != -1 && GameControllerManager::sGamePadManager != nullptr) {
                    StickValueInfo& val = sticks[entry.stick];
                    if (entry.stickY)
                        val.y = value;
                    else
                        val.x = value;
                    GameControllerManager::sGamePadManager->feedStick(index, entry.stick, 3, val.x, -val.y);
                }
                if (entry.trigger != -1 && GameControllerManager::sGamePadManager != nullptr) {
                    GameControllerManager::sGamePadManager->feedTrigger(index, entry.trigger, value);
                }
                if (entry.button != -1 && GameControllerManager::sGamePadManager != nullptr) {
                    GameControllerManager::sGamePadManager->feedButton(index, entry.button, value >= 0.9f, true);
                }
            }
            if (e.code >= ABS_HAT0X && e.code <= ABS_HAT0X + 8) {
                if (manager->rawHatCallback != nullptr)
                    manager->rawHatCallback(index, e.code - ABS_HAT0X, e.value);
                auto const& entries = mapping.hats[e.code - ABS_HAT0X];
                int oldVal = hatValues[e.code - ABS_HAT0X];
                if (oldVal != e.value && entries.count(oldVal) > 0) {
                    MappingInfo::Entry const& entry = entries.at(oldVal);
                    onButton(entry, false);
                }
                hatValues[e.code - ABS_HAT0X] = e.value;
                if (entries.count(e.value) > 0) {
                    MappingInfo::Entry const& entry = entries.at(e.value);
                    onButton(entry, true);
                }
            }
        }
    }
}

void LinuxGamepadManager::Device::onButton(MappingInfo::Entry const& mapping, bool pressed) {
    if (GameControllerManager::sGamePadManager == nullptr)
        return;
    if (mapping.button != -1)
        GameControllerManager::sGamePadManager->feedButton(index, mapping.button, pressed ? 1 : 0, true);
    // stick is absolutely pointless to assign to a button
    if (mapping.trigger != -1)
        GameControllerManager::sGamePadManager->feedTrigger(index, mapping.trigger, pressed ? 1.f : 0.f);
}

void LinuxGamepadManager::pool() {
    for (int i = 0; i < 4; i++) {
        if (devices[i].fd == -1)
            continue;
        devices[i].pool();
    }
}

int LinuxGamepadManager::getFreeDeviceIndex() {
    for (int i = 0; i < 4; i++) {
        if (devices[i].fd == -1)
            return i;
    }
    return -1;
}

void LinuxGamepadManager::onDeviceAdded(struct udev_device* dev) {
    const char* val = udev_device_get_property_value(dev, "ID_INPUT_JOYSTICK");
    if (val != nullptr && strcmp(val, "1") == 0) {
        const char* devPath = udev_device_get_devnode(dev);
        if (devPath == nullptr)
            return;

        int fd = open(devPath, O_RDONLY | O_NONBLOCK);
        struct libevdev* edev = nullptr;
        int err = libevdev_new_from_fd(fd, &edev);
        if (err) {
            printf("libevdev_new_from_fd error %i (%s)\n", err, devPath);
            close(fd);
            return;
        }

        int no = getFreeDeviceIndex();
        if (no == -1) {
            printf("Joystick connected but will not be added; no free device index\n");
            return;
        }
        printf("Joystick #%i added (%s)\n", no, devPath);
        devices[no].assign(fd, edev);
        if (GameControllerManager::sGamePadManager != nullptr) {
            devices[no].assignedInMinecraft = true;
            GameControllerManager::sGamePadManager->setGameControllerConnected(no, true);
        }
    }
}

#define ToBigEndianShort(val) ((((val)&0xff)<<8)|(((val)>>8)&0xff))

std::string LinuxGamepadManager::getSDLJoystickGUID(struct libevdev* dev) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0')
       << std::setw(4) << ToBigEndianShort(libevdev_get_id_bustype(dev))
       << std::setw(4) << 0
       << std::setw(4) << ToBigEndianShort(libevdev_get_id_vendor(dev))
       << std::setw(4) << 0
       << std::setw(4) << ToBigEndianShort(libevdev_get_id_product(dev))
       << std::setw(4) << 0
       << std::setw(4) << ToBigEndianShort(libevdev_get_id_version(dev))
       << std::setw(4) << 0;
    return ss.str();
}