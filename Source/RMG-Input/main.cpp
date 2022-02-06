/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#define CORE_PLUGIN
#define M64P_PLUGIN_PROTOTYPES 1
#define INPUT_PLUGIN_API_VERSION 0x020100

#include <UserInterface/MainDialog.hpp>
#include "Thread/SDLThread.hpp"
#include "Utilities/InputDevice.hpp"
#include "common.hpp"

#include <RMG-Core/Core.hpp>

#include <QApplication>
#include <SDL.h>

#include <iostream>
#include <chrono>

//
// Local Defines
//

#define NUM_CONTROLLERS 4
#define MAX_AXIS_VALUE  85

//
// Local Structures
//

struct InputMapping
{
    InputType Type = InputType::Invalid;
    int Data       = 0;
    int ExtraData  = 0;
};

struct InputProfile
{
    bool PluggedIn    = false;
    int DeadzoneValue = 0;
    int RangeValue    = 100;

    N64ControllerPak ControllerPak = N64ControllerPak::None;

    // input device information
    int DeviceType    = 0;
    std::string DeviceName;
    int DeviceNum     = -1;

    // input device
    Utilities::InputDevice InputDevice;

    // buttons
    InputMapping Button_A;
    InputMapping Button_B;
    InputMapping Button_Start;
    InputMapping Button_DpadUp;
    InputMapping Button_DpadDown;
    InputMapping Button_DpadLeft;
    InputMapping Button_DpadRight;
    InputMapping Button_CButtonUp;
    InputMapping Button_CButtonDown;
    InputMapping Button_CButtonLeft;
    InputMapping Button_CButtonRight;
    InputMapping Button_LeftTrigger;
    InputMapping Button_RightTrigger;
    InputMapping Button_ZTrigger;

    // analog stick
    InputMapping AnalogStick_Up;
    InputMapping AnalogStick_Down;
    InputMapping AnalogStick_Left;
    InputMapping AnalogStick_Right;
};

//
// Local variables
//

// You may wonder why I create a thread with SDL
// instead of initializing SDL in PluginStartup,
// that's because we have a config GUI which uses peepevents
// and SDL needs SDL_PumpEvents/SDL_NumJoystick to be run in 
// the same thread as the one it was initialized in,
// so when you open the config GUI once, it'll work fine
// but when you'd open it twice, it wouldn't work anymore
// This seems like the cleanest solution and it works well
static Thread::SDLThread *l_SDLThread = nullptr;

// input profiles
static InputProfile l_InputProfiles[NUM_CONTROLLERS];

// keyboard state
static bool l_KeyboardState[SDL_NUM_SCANCODES];

//
// Local Functions
//

static void load_inputmapping_settings(InputMapping* mapping, std::string section,
    SettingsID inputTypeSettingsId, SettingsID dataSettingsId, SettingsID extraDataSettingsId)
{
    mapping->Type = (InputType)CoreSettingsGetIntValue(inputTypeSettingsId, section);
    mapping->Data = CoreSettingsGetIntValue(dataSettingsId, section);
    mapping->ExtraData = CoreSettingsGetIntValue(extraDataSettingsId, section);
}


static void load_settings(void)
{
    for (int i = 0; i < NUM_CONTROLLERS; i++)
    {
        InputProfile* profile = &l_InputProfiles[i];
        std::string section = "Rosalie's Mupen GUI - Input Plugin Profile " + std::to_string(i);

        // when the settings section doesn't exist,
        // disable profile
        if (!CoreSettingsSectionExists(section))
        {
            profile->PluggedIn = false;
            continue;
        }

        profile->PluggedIn = CoreSettingsGetBoolValue(SettingsID::Input_PluggedIn, section);
        profile->DeadzoneValue = CoreSettingsGetIntValue(SettingsID::Input_Deadzone, section);
        profile->RangeValue = CoreSettingsGetIntValue(SettingsID::Input_Range, section);
        profile->DeviceType = CoreSettingsGetIntValue(SettingsID::Input_DeviceType, section);
        profile->DeviceName = CoreSettingsGetStringValue(SettingsID::Input_DeviceName, section);
        profile->DeviceNum = CoreSettingsGetIntValue(SettingsID::Input_DeviceNum, section);

        // load inputmapping settings
        load_inputmapping_settings(&profile->Button_A, section, SettingsID::Input_A_InputType, SettingsID::Input_A_Data, SettingsID::Input_A_ExtraData);
        load_inputmapping_settings(&profile->Button_B, section, SettingsID::Input_B_InputType, SettingsID::Input_B_Data, SettingsID::Input_B_ExtraData);
        load_inputmapping_settings(&profile->Button_Start, section, SettingsID::Input_Start_InputType, SettingsID::Input_Start_Data, SettingsID::Input_Start_ExtraData);
        load_inputmapping_settings(&profile->Button_DpadUp, section, SettingsID::Input_DpadUp_InputType, SettingsID::Input_DpadUp_Data, SettingsID::Input_DpadUp_ExtraData);
        load_inputmapping_settings(&profile->Button_DpadDown, section, SettingsID::Input_DpadDown_InputType, SettingsID::Input_DpadDown_Data, SettingsID::Input_DpadDown_ExtraData);
        load_inputmapping_settings(&profile->Button_DpadLeft, section, SettingsID::Input_DpadLeft_InputType, SettingsID::Input_DpadLeft_Data, SettingsID::Input_DpadLeft_ExtraData);
        load_inputmapping_settings(&profile->Button_DpadRight, section, SettingsID::Input_DpadRight_InputType, SettingsID::Input_DpadRight_Data, SettingsID::Input_DpadRight_ExtraData);
        load_inputmapping_settings(&profile->Button_CButtonUp, section, SettingsID::Input_CButtonUp_InputType, SettingsID::Input_CButtonUp_Data, SettingsID::Input_CButtonUp_ExtraData);
        load_inputmapping_settings(&profile->Button_CButtonDown, section, SettingsID::Input_CButtonDown_InputType, SettingsID::Input_CButtonDown_Data, SettingsID::Input_CButtonDown_ExtraData);
        load_inputmapping_settings(&profile->Button_CButtonLeft, section, SettingsID::Input_CButtonLeft_InputType, SettingsID::Input_CButtonLeft_Data, SettingsID::Input_CButtonLeft_ExtraData);
        load_inputmapping_settings(&profile->Button_CButtonRight, section, SettingsID::Input_CButtonRight_InputType, SettingsID::Input_CButtonRight_Data, SettingsID::Input_CButtonRight_ExtraData);
        load_inputmapping_settings(&profile->Button_LeftTrigger, section, SettingsID::Input_LeftTrigger_InputType, SettingsID::Input_LeftTrigger_Data, SettingsID::Input_LeftTrigger_ExtraData);
        load_inputmapping_settings(&profile->Button_RightTrigger, section, SettingsID::Input_RightTrigger_InputType, SettingsID::Input_RightTrigger_Data, SettingsID::Input_RightTrigger_ExtraData);
        load_inputmapping_settings(&profile->Button_ZTrigger, section, SettingsID::Input_ZTrigger_InputType, SettingsID::Input_ZTrigger_Data, SettingsID::Input_ZTrigger_ExtraData);
        load_inputmapping_settings(&profile->AnalogStick_Up, section, SettingsID::Input_AnalogStickUp_InputType, SettingsID::Input_AnalogStickUp_Data, SettingsID::Input_AnalogStickUp_ExtraData);
        load_inputmapping_settings(&profile->AnalogStick_Down, section, SettingsID::Input_AnalogStickDown_InputType, SettingsID::Input_AnalogStickDown_Data, SettingsID::Input_AnalogStickDown_ExtraData);
        load_inputmapping_settings(&profile->AnalogStick_Left, section, SettingsID::Input_AnalogStickLeft_InputType, SettingsID::Input_AnalogStickLeft_Data, SettingsID::Input_AnalogStickLeft_ExtraData);
        load_inputmapping_settings(&profile->AnalogStick_Right, section, SettingsID::Input_AnalogStickRight_InputType, SettingsID::Input_AnalogStickRight_Data, SettingsID::Input_AnalogStickRight_ExtraData);
    }
}

static void open_controllers(void)
{
    for (int i = 0; i < NUM_CONTROLLERS; i++)
    {
        InputProfile* profile = &l_InputProfiles[i];

        if (profile->InputDevice.HasOpenDevice())
        {
            profile->InputDevice.CloseDevice();
        }

        if (profile->DeviceNum != -1)
        {
            profile->InputDevice.OpenDevice(profile->DeviceName, profile->DeviceNum);
        }
    }
}

static void close_controllers(void)
{
    for (int i = 0; i < NUM_CONTROLLERS; i++)
    {
        InputProfile* profile = &l_InputProfiles[i];

        if (profile->InputDevice.HasOpenDevice())
        {
            profile->InputDevice.CloseDevice();
        }
    }
}

static int get_button_state(InputProfile* profile, InputMapping* inputMapping)
{
    switch (inputMapping->Type)
    {
        case InputType::GamepadButton:
        {
            return SDL_GameControllerGetButton(profile->InputDevice.GetGameControllerHandle(), (SDL_GameControllerButton)inputMapping->Data);
        };
        case InputType::GamepadAxis:
        {
            int axis_value = SDL_GameControllerGetAxis(profile->InputDevice.GetGameControllerHandle(), (SDL_GameControllerAxis)inputMapping->Data);
            return (abs(axis_value) >= (SDL_AXIS_PEAK / 2) && (inputMapping->ExtraData ? axis_value > 0 : axis_value < 0)) ? 1 : 0;
        };
        case InputType::Keyboard:
        {
            return l_KeyboardState[inputMapping->Data] ? 1 : 0;
        };
        default:
            break;
    }

    return 0;
}

static int get_axis_state(InputProfile* profile, InputMapping* inputMapping, int direction, int value)
{
    switch (inputMapping->Type)
    {
        case InputType::GamepadButton:
        {
            int buttonState = SDL_GameControllerGetButton(profile->InputDevice.GetGameControllerHandle(), (SDL_GameControllerButton)inputMapping->Data);
            return buttonState ? MAX_AXIS_VALUE * direction : value;
        };
        case InputType::GamepadAxis:
        {
            int axis_value = SDL_GameControllerGetAxis(profile->InputDevice.GetGameControllerHandle(), (SDL_GameControllerAxis)inputMapping->Data);
            if (inputMapping->ExtraData ? axis_value > 0 : axis_value < 0)
            {
                axis_value = ((float)((float)axis_value / SDL_AXIS_PEAK * 100) * ((float)MAX_AXIS_VALUE / 100));
                axis_value = abs(axis_value) * direction;
                return axis_value;
            }
            else
            {
                return value;
            }
        };
        case InputType::Keyboard:
        {
            return l_KeyboardState[inputMapping->Data] ? MAX_AXIS_VALUE * direction : value;
        };
        default:
            break;
    }

    return value;
}

static bool is_in_deadzone(int x, int y, int deadzoneValue)
{
    const double dist = sqrt(pow(x, 2) + pow(y, 2));
    return dist <= deadzoneValue;
}

//
// Basic Plugin Functions
//

EXPORT m64p_error CALL PluginStartup(m64p_dynlib_handle CoreLibHandle, void *Context, void (*DebugCallback)(void *, int, const char *))
{
    if (l_SDLThread != nullptr)
    {
        return M64ERR_ALREADY_INIT;
    }

    if (!CoreInit(CoreLibHandle))
    {
        return M64ERR_SYSTEM_FAIL;
    }

    l_SDLThread = new Thread::SDLThread(nullptr);
    l_SDLThread->start();

    load_settings();

    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginShutdown(void)
{
    if (l_SDLThread == nullptr)
    {
        return M64ERR_NOT_INIT;
    }

    close_controllers();

    l_SDLThread->StopLoop();
    l_SDLThread->deleteLater();
    l_SDLThread = nullptr;

    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginGetVersion(m64p_plugin_type *pluginType, int *pluginVersion, 
    int *apiVersion, const char **pluginNamePtr, int *capabilities)
{
    if (pluginType != nullptr)
    {
        *pluginType = M64PLUGIN_INPUT;
    }

    if (pluginVersion != nullptr)
    {
        *pluginVersion = 0x010000;
    }

    if (apiVersion != nullptr)
    {
        *apiVersion = INPUT_PLUGIN_API_VERSION;
    }

    if (pluginNamePtr != nullptr)
    {
        *pluginNamePtr = "Rosalie's Mupen GUI - Input Plugin";
    }

    if (capabilities != nullptr)
    {
        *capabilities = 0;
    }

    return M64ERR_SUCCESS;
}

//
// Custom Plugin Functions
//

#include <iostream>
EXPORT m64p_error CALL PluginConfig()
{
    if (l_SDLThread == nullptr)
    {
        return M64ERR_NOT_INIT;
    }

    // close controllers
    close_controllers();

    l_SDLThread->SetAction(SDLThreadAction::SDLPumpEvents);

    UserInterface::MainDialog dialog(nullptr, l_SDLThread);
    dialog.exec();

    l_SDLThread->SetAction(SDLThreadAction::None);
    
    // reload settings
    load_settings();

    // open controllers
    open_controllers();

    return M64ERR_SUCCESS;
}

//
// Input Plugin Functions
//

EXPORT void CALL ControllerCommand(int Control, unsigned char* Command)
{

}

EXPORT void CALL GetKeys(int Control, BUTTONS* Keys)
{
    InputProfile* profile = &l_InputProfiles[Control];

    Keys->A_BUTTON     = get_button_state(profile, &profile->Button_A);
    Keys->B_BUTTON     = get_button_state(profile, &profile->Button_B);
    Keys->START_BUTTON = get_button_state(profile, &profile->Button_Start);
    Keys->U_DPAD       = get_button_state(profile, &profile->Button_DpadUp);
    Keys->D_DPAD       = get_button_state(profile, &profile->Button_DpadDown);
    Keys->L_DPAD       = get_button_state(profile, &profile->Button_DpadLeft);
    Keys->R_DPAD       = get_button_state(profile, &profile->Button_DpadRight);
    Keys->U_CBUTTON    = get_button_state(profile, &profile->Button_CButtonUp);
    Keys->D_CBUTTON    = get_button_state(profile, &profile->Button_CButtonDown);
    Keys->L_CBUTTON    = get_button_state(profile, &profile->Button_CButtonLeft);
    Keys->R_CBUTTON    = get_button_state(profile, &profile->Button_CButtonRight);
    Keys->L_TRIG       = get_button_state(profile, &profile->Button_LeftTrigger);
    Keys->R_TRIG       = get_button_state(profile, &profile->Button_RightTrigger);
    Keys->Z_TRIG       = get_button_state(profile, &profile->Button_ZTrigger);

    Keys->Y_AXIS       = get_axis_state(profile, &profile->AnalogStick_Up, 1, Keys->Y_AXIS);
    Keys->Y_AXIS       = get_axis_state(profile, &profile->AnalogStick_Down, -1, Keys->Y_AXIS);
    Keys->X_AXIS       = get_axis_state(profile, &profile->AnalogStick_Left, -1, Keys->X_AXIS);
    Keys->X_AXIS       = get_axis_state(profile, &profile->AnalogStick_Right, 1, Keys->X_AXIS);

    // take deadzone into account
    if (is_in_deadzone(Keys->X_AXIS, Keys->Y_AXIS, profile->DeadzoneValue))
    {
        Keys->Y_AXIS = 0;
        Keys->X_AXIS = 0;
    }
}

EXPORT void CALL InitiateControllers(CONTROL_INFO ControlInfo)
{
    for (int i = 0; i < NUM_CONTROLLERS; i++)
    {
        InputProfile* profile = &l_InputProfiles[i];

        ControlInfo.Controls[i].Present = profile->PluggedIn ? 1 : 0;
        // TODO
        ControlInfo.Controls[i].Plugin = PLUGIN_MEMPAK;
        ControlInfo.Controls[i].RawData = 0;
    }

    for (int i = 0; i < SDL_NUM_SCANCODES; i++)
    {
        l_KeyboardState[i] = 0;
    }

    open_controllers();
}

EXPORT void CALL ReadController(int Control, unsigned char *Command)
{
}

EXPORT int CALL RomOpen(void)
{
    return 1;
}

EXPORT void CALL RomClosed(void)
{
    close_controllers();
}

EXPORT void CALL SDL_KeyDown(int keymod, int keysym)
{
    l_KeyboardState[keysym] = true;
}

EXPORT void CALL SDL_KeyUp(int keymod, int keysym)
{
    l_KeyboardState[keysym] = false;
}
