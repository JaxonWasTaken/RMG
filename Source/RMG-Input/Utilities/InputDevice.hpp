/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef INPUTDEVICE_HPP
#define INPUTDEVICE_HPP

#include "common.hpp"

#include <string>
#include <SDL.h>

namespace Utilities
{
class InputDevice
{
public:
    InputDevice();
    ~InputDevice();

    InputDeviceType     GetDeviceType();
    SDL_Joystick*       GetJoystickHandle();
    SDL_GameController* GetGameControllerHandle();

    // returns whether a device has been opened
    bool HasOpenDevice();

    // tries to open device with given name & num
    bool OpenDevice(std::string name, int num);

    // tries to close opened device
    bool CloseDevice();

private:
    InputDeviceType     deviceType = InputDeviceType::Invalid;
    SDL_Joystick*       joystick = nullptr;
    SDL_GameController* gameController = nullptr;
};
} // namespace Utilities

#endif // INPUTDEVICE_HPP