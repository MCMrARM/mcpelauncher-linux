#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <limits>
#include <sstream>
#include <fstream>
#include <iomanip>
#include "gamepad.h"
#include "path_helper.h"
#include "minecraft/GameControllerManager.h"

LinuxGamepadManager LinuxGamepadManager::instance;

const char* LinuxGamepadManager::MAPPINGS_FILE = "gamepad-mappings.txt";

LinuxGamepadManager::LinuxGamepadManager() {
    fallbackMapping = std::shared_ptr<MappingInfo>(new MappingInfo());
    fallbackMapping->buttons[BTN_A - BTN_GAMEPAD].button = BUTTON_A;
    fallbackMapping->buttons[BTN_B - BTN_GAMEPAD].button = BUTTON_B;
    fallbackMapping->buttons[BTN_X - BTN_GAMEPAD].button = BUTTON_X;
    fallbackMapping->buttons[BTN_Y - BTN_GAMEPAD].button = BUTTON_Y;
    fallbackMapping->buttons[BTN_TL - BTN_GAMEPAD].button = BUTTON_LB;
    fallbackMapping->buttons[BTN_TR - BTN_GAMEPAD].button = BUTTON_RB;
    fallbackMapping->buttons[BTN_THUMBL - BTN_GAMEPAD].button = BUTTON_LEFT_STICK;
    fallbackMapping->buttons[BTN_THUMBR - BTN_GAMEPAD].button = BUTTON_RIGHT_STICK;
    fallbackMapping->buttons[BTN_START - BTN_GAMEPAD].button = BUTTON_START;
    fallbackMapping->axis[ABS_X].stick = fallbackMapping->axis[ABS_Y].stick = 0;
    fallbackMapping->axis[ABS_Y].stickY = true;
    fallbackMapping->axis[ABS_RX].stick = fallbackMapping->axis[ABS_RY].stick = 1;
    fallbackMapping->axis[ABS_RY].stickY = true;
    fallbackMapping->axis[ABS_Z].trigger = 0;
    fallbackMapping->axis[ABS_RZ].trigger = 1;
    fallbackMapping->hats[ABS_HAT0X - ABS_HAT0X][-1].button = BUTTON_DPAD_LEFT;
    fallbackMapping->hats[ABS_HAT0X - ABS_HAT0X][1].button = BUTTON_DPAD_RIGHT;
    fallbackMapping->hats[ABS_HAT0Y - ABS_HAT0X][-1].button = BUTTON_DPAD_UP;
    fallbackMapping->hats[ABS_HAT0Y - ABS_HAT0X][1].button = BUTTON_DPAD_DOWN;

    for (int i = 0; i < 4; i++) {
        devices[i].manager = this;
        devices[i].index = i;
        devices[i].mapping = fallbackMapping;
    }

    std::ifstream mappingsFile(PathHelper::getPrimaryDataDirectory() + MAPPINGS_FILE);
    if (mappingsFile)
        parseMappings(mappingsFile);
}

void LinuxGamepadManager::parseMappings(std::istream& ifs) {
    mappings.clear();
    std::string line;
    while (std::getline(ifs, line)) {
        size_t o = line.find(' ');
        if (o == std::string::npos)
            continue;
        std::string key = line.substr(0, o);
        std::string mapping = line.substr(o + 1);
        mappings[key] = std::shared_ptr<MappingInfo>(new MappingInfo());
        mappings[key]->parse(mapping);
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

    std::string typeId = manager->getGamepadTypeId(edev);
    if (manager->mappings.count(typeId))
        mapping = manager->mappings.at(typeId);
    else
        mapping = manager->fallbackMapping;
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
                if (mapping == nullptr)
                    continue;
                MappingInfo::Entry const& entry = mapping->buttons[code];
                onButton(entry, e.value != 0);
            }
        } else if (e.type == EV_ABS) {
            if (e.code >= ABS_X && e.code < ABS_X + 16) {
                float value = (float) e.value / axisInfo[e.code].max;
                if (value >= -0.1f && value <= 0.1f)
                    value = 0.f;
                if (manager->rawStickCallback != nullptr)
                    manager->rawStickCallback(index, e.code, value);
                if (mapping == nullptr)
                    continue;
                MappingInfo::Entry const& entry = mapping->axis[e.code];
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
                if (mapping == nullptr)
                    continue;
                auto const& entries = mapping->hats[e.code - ABS_HAT0X];
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
    if (mapping.button == BUTTON_START)
        GameControllerManager::sGamePadManager->feedJoinGame(index, true);
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

void LinuxGamepadManager::MappingInfo::parse(std::string const& str) {
    for (size_t start = 0; start < str.length(); ) {
        size_t end = str.find(';', start);
        if (end == std::string::npos)
            end = str.length();

        size_t s = str.find('=', start);
        if (s + 1 >= end || s == std::string::npos)
            throw std::runtime_error("Parse error");
        std::string key = str.substr(start, s - start);
        s++;

        printf("Key = %s\n", key.c_str());

        Entry mapping;
        if (key == "button:a")
            mapping.button = BUTTON_A;
        if (key == "button:b")
            mapping.button = BUTTON_B;
        if (key == "button:x")
            mapping.button = BUTTON_X;
        if (key == "button:y")
            mapping.button = BUTTON_Y;
        if (key == "button:select")
            mapping.button = BUTTON_SELECT;
        if (key == "button:start")
            mapping.button = BUTTON_START;
        if (key == "button:leftstick")
            mapping.button = BUTTON_LEFT_STICK;
        if (key == "button:rightstick")
            mapping.button = BUTTON_RIGHT_STICK;
        if (key == "button:lb")
            mapping.button = BUTTON_LB;
        if (key == "button:rb")
            mapping.button = BUTTON_RB;
        if (key == "axis:trleft")
            mapping.trigger = 0;
        if (key == "axis:trright")
            mapping.trigger = 1;
        if (key == "axis:leftx" || key == "axis:lefty")
            mapping.stick = 0;
        if (key == "axis:rightx" || key == "axis:righty")
            mapping.stick = 1;
        if (key == "axis:lefty" || key == "axis:righty")
            mapping.stickY = true;

        char t = str[s];
        s++;
        if (t == 'b') {
            size_t bs = str.find(',', s);
            if (bs == std::string::npos || bs >= end) {
                int btnId = std::stoi(str.substr(s, end - s));
                buttons[btnId] = mapping;
            } else {
                int leftBtnId = std::stoi(str.substr(s, bs - s));;
                int rightBtnId = std::stoi(str.substr(bs + 1, end - bs - 1));;
                buttons[leftBtnId] = mapping;
                buttons[leftBtnId].stickVal = -1;
                buttons[rightBtnId] = mapping;
                buttons[rightBtnId].stickVal = 1;
                if (key == "axis:dpadx") {
                    buttons[leftBtnId].button = BUTTON_DPAD_LEFT;
                    buttons[rightBtnId].button = BUTTON_DPAD_RIGHT;
                }
                if (key == "axis:dpady") {
                    buttons[leftBtnId].button = BUTTON_DPAD_DOWN;
                    buttons[rightBtnId].button = BUTTON_DPAD_UP;
                }
            }
        } else if (t == 's') {
            bool invert = str[s] == '!';
            if (invert)
                s++;
            int stickId = std::stoi(str.substr(s, end - s));
            axis[stickId] = mapping;
            axis[stickId].stickVal = (char) (invert ? -1 : 1);
        } else if (t == 'h') {
            size_t hs = str.find(',', s);
            if (hs == std::string::npos || hs >= end) {
                size_t vs = str.find(':', s);
                if (vs == std::string::npos || vs >= end)
                    throw std::runtime_error("Parse error");
                int hatId = std::stoi(str.substr(s, vs - s));
                int hatVal = std::stoi(str.substr(vs + 1, end - vs - 1));
                hats[hatId][hatVal] = mapping;
            } else {
                size_t vs = str.find(':', s);
                if (vs == std::string::npos || vs >= hs)
                    throw std::runtime_error("Parse error");
                int leftHatId = std::stoi(str.substr(s, vs - s));
                int leftHatVal = std::stoi(str.substr(vs + 1, hs - vs - 1));
                hats[leftHatId][leftHatVal] = mapping;
                hats[leftHatId][leftHatVal].stickVal = -1;

                vs = str.find(':', hs + 1);
                if (vs == std::string::npos || vs >= end)
                    throw std::runtime_error("Parse error");
                int rightHatId = std::stoi(str.substr(hs + 1, vs - hs - 1));
                int rightHatVal = std::stoi(str.substr(vs + 1, end - vs - 1));
                hats[rightHatId][rightHatVal] = mapping;
                hats[rightHatId][rightHatVal].stickVal = 1;

                if (key == "axis:dpadx") {
                    hats[leftHatId][leftHatVal].button = BUTTON_DPAD_LEFT;
                    hats[rightHatId][rightHatVal].button = BUTTON_DPAD_RIGHT;
                }
                if (key == "axis:dpady") {
                    hats[leftHatId][leftHatVal].button = BUTTON_DPAD_DOWN;
                    hats[rightHatId][rightHatVal].button = BUTTON_DPAD_UP;
                }
            }
        }

        start = end + 1;
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

LinuxGamepadManager::GamepadTypeId LinuxGamepadManager::getGamepadTypeId(struct libevdev* dev) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0')
       << std::setw(4) << libevdev_get_id_bustype(dev) << ':'
       << std::setw(4) << libevdev_get_id_vendor(dev) << ':'
       << std::setw(4) << libevdev_get_id_product(dev) << ':'
       << std::setw(4) << libevdev_get_id_version(dev);
    return ss.str();
}

LinuxGamepadManager::GamepadTypeId LinuxGamepadManager::getGamepadTypeId(int index) {
    struct libevdev* dev = devices[index].edev;
    if (dev == nullptr)
        return std::string();
    return getGamepadTypeId(dev);
}

void LinuxGamepadManager::setGamepadMapping(GamepadTypeId gamepad, std::string const& mapping) {
    std::shared_ptr<MappingInfo> mappingInfo (new MappingInfo());
    mappingInfo->parse(mapping);
    mappings[gamepad] = mappingInfo;

    std::ifstream mappingsFile(PathHelper::getPrimaryDataDirectory() + MAPPINGS_FILE);
    std::map<std::string, std::string> mappingStrings;
    if (mappingsFile) {
        std::string line;
        while (std::getline(mappingsFile, line)) {
            size_t o = line.find(' ');
            if (o == std::string::npos)
                continue;
            mappingStrings[line.substr(0, o)] = line.substr(o + 1);
        }
    }
    mappingStrings[gamepad] = mapping;
    mappingsFile.close();
    std::ofstream outMappingsFile(PathHelper::getPrimaryDataDirectory() + MAPPINGS_FILE);
    for (auto const& p : mappingStrings)
        outMappingsFile << p.first << " " << p.second << "\n";
    outMappingsFile.close();
}