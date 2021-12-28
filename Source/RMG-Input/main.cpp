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

#include <RMG-Core/Core.hpp>

#include <QApplication>
#include <SDL.h>

#include <iostream>
#include <chrono>

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

//
// Basic Plugin Functions
//

#include <iostream>

EXPORT m64p_error CALL PluginStartup(m64p_dynlib_handle CoreLibHandle, void *Context, void (*DebugCallback)(void *, int, const char *))
{
    if (l_SDLThread != nullptr)
    {
        return M64ERR_ALREADY_INIT;
    }

    l_SDLThread = new Thread::SDLThread(nullptr);
    l_SDLThread->start();

    std::cout << __FUNCTION__ << std::endl;
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginShutdown(void)
{
    if (l_SDLThread == nullptr)
    {
        return M64ERR_NOT_INIT;
    }

    l_SDLThread->StopLoop();
    l_SDLThread->deleteLater();
    l_SDLThread = nullptr;

    std::cout << __FUNCTION__ << std::endl;
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

    l_SDLThread->SetAction(SDLThreadAction::SDLPumpEvents);

    UserInterface::MainDialog dialog(nullptr, l_SDLThread);
    dialog.exec();

    l_SDLThread->SetAction(SDLThreadAction::None);
    
    return M64ERR_SUCCESS;
}

//
// Input Plugin Functions
//

EXPORT void CALL ControllerCommand(int Control, unsigned char* Command)
{

}

EXPORT void CALL GetKeys(int Control, BUTTONS * Keys)
{

}

EXPORT void CALL InitiateControllers(CONTROL_INFO ControlInfo)
{
    ControlInfo.Controls->Present = 1;
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

}

EXPORT void CALL SDL_KeyDown(int keymod, int keysym)
{

}

EXPORT void CALL SDL_KeyUp(int keymod, int keysym)
{

}
