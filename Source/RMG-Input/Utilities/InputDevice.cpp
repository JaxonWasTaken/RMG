/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "InputDevice.hpp"

using namespace Utilities;

InputDevice::InputDevice()
{

}

InputDevice::~InputDevice()
{
    this->CloseDevice();
}

InputDeviceType InputDevice::GetDeviceType()
{
    return this->deviceType;
}

SDL_Joystick* InputDevice::GetJoystickHandle()
{
    return this->joystick;
}

SDL_GameController* InputDevice::GetGameControllerHandle()
{
    return this->gameController;
}

bool InputDevice::HasOpenDevice()
{
    return this->joystick != nullptr || this->gameController != nullptr;
}

bool InputDevice::OpenDevice(std::string name, int num)
{
    // TODO, joystick support etc
    this->gameController = SDL_GameControllerOpen(num);
    if (this->gameController != nullptr)
    {
        this->deviceType = InputDeviceType::Gamepad;
    }
    return this->gameController == nullptr;
}

bool InputDevice::CloseDevice()
{
    switch (this->GetDeviceType())
    {
        case InputDeviceType::Joystick:
            SDL_JoystickClose(this->joystick);
            this->joystick = nullptr;
            break;

        case InputDeviceType::Gamepad:
            SDL_GameControllerClose(this->gameController);
            this->gameController = nullptr;
            break;

        default:
            break;
    }


    return true;
}