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
    
}

void LinuxGamepadManager::init() {

    joystick = new Joystick();
    
    //functional keys
    buttons[0] = BUTTON_A;
    buttons[1] = BUTTON_B;
    buttons[2] = BUTTON_X;
    buttons[3] = BUTTON_Y;
    //upper buttons
    buttons[4] = BUTTON_LB;
    buttons[5] = BUTTON_RB;
    //lower triggers
    buttons[6] = 0;
    buttons[7] = 1;
    //start/select
    buttons[8] = BUTTON_SELECT;
    buttons[9] = BUTTON_START;
    //stick click
    buttons[10] = BUTTON_LS;
    buttons[11] = BUTTON_RS;
    
    

    if (joystick->isFound())
    {
        GameControllerManager::sGamePadManager->setGameControllerConnected(0, true);
    } 
    else 
        printf("open failed.\n");
}


void LinuxGamepadManager::pool() {
    //main loop important to do everything there
    float val;
    JoystickEvent event;   

    while (true)
    {
        // Attempt to sample an event from the joystick
        
        if (joystick->sample(&event))
        {
            if (event.isButton())
            {
                //printf("Button %u is %s\n",event.number,event.value == 0 ? "up" : "down");
                //usage buttons
                if(event.number == 6 || event.number == 7){
                    GameControllerManager::sGamePadManager->feedTrigger(0, buttons[event.number], (float) event.value);
                } else 
                    GameControllerManager::sGamePadManager->feedButton(0, buttons[event.number], event.value, true);
                break;
 
            }
            else if (event.isAxis())
            {
                //printf("Axis %u is at position %d\n", event.number, event.value);
                if(event.number == 5) { //dpad usage
                    if(event.value > 0) lastdpad[0] = BUTTON_DPAD_RIGHT;
                    if(event.value < 0) lastdpad[0] = BUTTON_DPAD_LEFT;
                    GameControllerManager::sGamePadManager->feedButton(0, lastdpad[0], event.value !=0 ? 1 : 0, true);
                }
                if(event.number == 6) { //dpad usage
                    if(event.value > 0) lastdpad[1] = BUTTON_DPAD_DOWN;
                    if(event.value < 0) lastdpad[1] = BUTTON_DPAD_UP;
                    GameControllerManager::sGamePadManager->feedButton(0, lastdpad[1], event.value !=0 ? 1 : 0, true);
                }

                val = round(event.value / 32768.0);
                if(event.number == LAXIS_X) { 
                    sticks[LEFT_STICK].x = val;
                    if(MUTUAL_AXIS != -1) {
                        joystick->sample(&event);
                        val = round(event.value / 32768.0);
                        if(event.number == MUTUAL_AXIS) sticks[LEFT_STICK].x = val;
                    }
                    GameControllerManager::sGamePadManager->feedStick(0, LEFT_STICK, 3, sticks[LEFT_STICK].x, -sticks[LEFT_STICK].y);
                    
                    break;
                }
                        
                if(event.number == LAXIS_Y) {
                    sticks[LEFT_STICK].y = val;
                    GameControllerManager::sGamePadManager->feedStick(0, LEFT_STICK, 3, sticks[LEFT_STICK].x, -sticks[LEFT_STICK].y);
                    
                }
                       
                if(event.number == RAXIS_X) {
                    sticks[1].x = val;
                    GameControllerManager::sGamePadManager->feedStick(0, RIGHT_STICK, 3, sticks[RIGHT_STICK].x, -sticks[RIGHT_STICK].y);
                }

                if( event.number == RAXIS_Y) { 
                    sticks[RIGHT_STICK].y = val; 
                    if(MUTUAL_AXIS != -1) {
                        joystick->sample(&event);
                        val = round(event.value / 32768.0);
                        if(event.number == MUTUAL_AXIS) sticks[RIGHT_STICK].y = val;
                    }
                    GameControllerManager::sGamePadManager->feedStick(0, RIGHT_STICK, 3, sticks[RIGHT_STICK].x, -sticks[RIGHT_STICK].y);
                    break;
                }

            }
            break;
        }
        break;
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

float LinuxGamepadManager::round(float v){
    if(v<0.f && v>-0.1f) return 0.f;
    if(v>0.f && v<0.1f) return 0.f;
    if(v<=-0.9f) return -1.f;
    if(v>=0.9f) return 1.f;
    return v;   
}

