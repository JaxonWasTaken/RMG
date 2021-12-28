/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "SDLThread.hpp"

#include <SDL.h>

using namespace Thread;

SDLThread::SDLThread(QObject *parent) : QThread(parent)
{

}

SDLThread::~SDLThread()
{
    if (this->isRunning())
    {
        this->StopLoop();
    }
}

void SDLThread::StopLoop(void)
{
    this->keepLoopRunning = false;
    while (this->isRunning())
    {
        // wait until we're not running anymore
    }
}

SDLThreadAction SDLThread::GetCurrentAction(void)
{
    return this->currentAction;
}

void SDLThread::SetAction(SDLThreadAction action)
{
    this->currentAction = action;
}

#include <iostream>
void SDLThread::run(void)
{
    // try to initialize SDL
    if (SDL_WasInit(SDL_INIT_GAMECONTROLLER) ||
#ifdef _WIN32
        // for some weird reason,
        // SDL also needs the video subsystem
        // to be initialized to get SDL_NumJoysticks
        // to work properly on windows
        SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_VIDEO) < 0)
#else
        SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0)
#endif
    { // nothing we can do now
        return;
    }

    while (this->keepLoopRunning)
    {
        switch (this->currentAction)
        {
            default:
            case SDLThreadAction::None:
                break;
            case SDLThreadAction::SDLPumpEvents:
            {
                SDL_PumpEvents();
            } break;
            case SDLThreadAction::GetInputDevices:
            {
                // force re-fresh joystick list
                SDL_JoystickUpdate();

                for (int i = 0; i < SDL_NumJoysticks(); i++)
                {
                    if (!SDL_IsGameController(i))
                    { // skip for now
                        continue;
                    }

                    const char* name = SDL_GameControllerNameForIndex(i);
                    if (name != nullptr)
                    {
                        emit this->OnInputDeviceFound(QString(name), i);
                    }
                }
                this->currentAction = SDLThreadAction::None;
                emit this->OnDeviceSearchFinished();
            } break;
        }

        // sleep for 10ms
        QThread::msleep(10);
    }

    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}
