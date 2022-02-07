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

void InputDevice::SetSDLThread(Thread::SDLThread* sdlThread)
{
    this->sdlThread = sdlThread;
    connect(this->sdlThread, &Thread::SDLThread::OnInputDeviceFound, this,
        &InputDevice::on_SDLThread_DeviceFound);
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

bool InputDevice::StartRumble(void)
{
    if (this->haptic != nullptr)
    {
        return SDL_HapticRumblePlay(this->haptic, 1, SDL_HAPTIC_INFINITY) == 0;
    }

    return false;
}

bool InputDevice::StopRumble(void)
{
    if (this->haptic != nullptr)
    {
        return SDL_HapticRumbleStop(this->haptic) == 0;
    }

    return false;
}

bool InputDevice::IsAttached(void)
{
    return SDL_JoystickGetAttached(this->joystick) == SDL_TRUE;
}

bool InputDevice::HasOpenDevice()
{
    return this->joystick != nullptr || this->gameController != nullptr;
}

bool InputDevice::OpenDevice(std::string name, int num)
{
    this->sdlThread->SetAction(SDLThreadAction::GetInputDevices);

    // wait until it's done searching
    while (this->sdlThread->GetCurrentAction() == SDLThreadAction::GetInputDevices)
    {
        QThread::msleep(50);
    }

    bool foundNameMatch = false;

    do
    {
        for (const auto& device : this->foundDevices)
        {
            if (device.name == name)
            {
                if (device.number == num || foundNameMatch)
                {
                    this->joystick = SDL_JoystickOpen(device.number);
                    this->deviceType = InputDeviceType::Joystick;
                    if (SDL_IsGameController(device.number))
                    {
                        this->gameController = SDL_GameControllerOpen(device.number);
                        this->deviceType = InputDeviceType::Gamepad;
                    }
                    this->haptic = SDL_HapticOpenFromJoystick(this->joystick);
                    if (this->haptic != nullptr)
                    {
                        SDL_HapticRumbleInit(this->haptic);
                    }
                    foundNameMatch = false;
                }
                else
                {
                    foundNameMatch = true;
                }
            }
        }
    } while (foundNameMatch);

    return this->joystick != nullptr || this->gameController == nullptr;
}

bool InputDevice::CloseDevice()
{
    if (this->joystick != nullptr)
    {
        SDL_JoystickClose(this->joystick);
        this->joystick = nullptr;
    }

    if (this->gameController != nullptr)
    {
        SDL_GameControllerClose(this->gameController);
        this->gameController = nullptr;
    }

    if (this->haptic != nullptr)
    {
        SDL_HapticClose(this->haptic);
        this->haptic = nullptr;
    }

    return true;
}

void InputDevice::on_SDLThread_DeviceFound(QString name, int number)
{
    SDLDevice device;
    device.name = name.toStdString();
    device.number = number;

    this->foundDevices.push_back(device);
}
