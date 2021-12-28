/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "MainDialog.hpp"
#include "Widget/ControllerWidget.hpp"

#include <SDL.h>
#include <QTimer>

using namespace UserInterface;

MainDialog::MainDialog(QWidget* parent, Thread::SDLThread* sdlThread) : QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    this->setupUi(this);
    this->setWindowIcon(QIcon(":Resource/RMG.png"));

    // setup SDL thread
    this->sdlThread = sdlThread;
    connect(this->sdlThread, &Thread::SDLThread::OnInputDeviceFound, this,
        &MainDialog::on_SDLThread_DeviceFound);
    connect(this->sdlThread, &Thread::SDLThread::OnDeviceSearchFinished, this,
        &MainDialog::on_SDLThread_DeviceSearchFinished);

    // each tab needs its own ControllerWidget
    for (int i = 0; i < this->tabWidget->count(); i++)
    {
        Widget::ControllerWidget* widget = new Widget::ControllerWidget(this);
        this->tabWidget->widget(i)->layout()->addWidget(widget);
        controllerWidgets.push_back(widget);
        connect(widget, &Widget::ControllerWidget::CurrentInputDeviceChanged, this,
            &MainDialog::on_ControllerWidget_CurrentInputDeviceChanged);
        connect(widget, &Widget::ControllerWidget::RefreshInputDevicesButtonClicked, this,
            &MainDialog::on_ControllerWidget_RefreshInputDevicesButtonClicked);
    }

    // always add keyboard device
    for (auto& controllerWidget : this->controllerWidgets)
    {
        controllerWidget->AddInputDevice("Keyboard", -1);
    }

    // fill device list at least once
    this->on_ControllerWidget_RefreshInputDevicesButtonClicked();

    this->inputPollTimer = new QTimer(this);
    connect(this->inputPollTimer, &QTimer::timeout, this, &MainDialog::on_InputPollTimer_triggered);
    this->inputPollTimer->start(50);
}

MainDialog::~MainDialog()
{
    this->closeController();
}

void MainDialog::addInputDevice(QString deviceName, int deviceNum)
{
    for (auto& controllerWidget : this->controllerWidgets)
    {
        controllerWidget->AddInputDevice(deviceName, deviceNum);
    }
}

void MainDialog::removeInputDevice(QString deviceName, int deviceNum)
{
    for (auto& controllerWidget : this->controllerWidgets)
    {
        controllerWidget->RemoveInputDevice(deviceName, deviceNum);
    }
}

void MainDialog::openController(QString deviceName, int deviceNum)
{
    // we don't need to open a keyboard
    if (deviceNum == -1)
    {
        currentDeviceName = "";
        currentDeviceNum = -1;
        return;
    }

    currentController = SDL_GameControllerOpen(deviceNum);
    currentDeviceNum = deviceNum;

    /* maybe throw this in SDLThread??

    // find each controller    
    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        const char* name = nullptr;

        // skip non-gamecontrollers
        if (!SDL_IsGameController(i))
        {
            continue;
        }
        
        name = SDL_GameControllerNameForIndex(i);
        if (name != nullptr)
        {
            if (deviceName == QString(name) &&
                deviceNum == i)
            {
                currentController = SDL_GameControllerOpen(i);
                currentDeviceNum = deviceNum;
                currentDeviceName = deviceName;
                return;
            }
        }
    }
    */
}

void MainDialog::closeController()
{
    if (this->currentController != nullptr)
    {
        SDL_GameControllerClose(this->currentController);
        this->currentController = nullptr;
    }
}

void MainDialog::on_InputPollTimer_triggered()
{
    Widget::ControllerWidget* controllerWidget;
    controllerWidget = controllerWidgets.at(this->tabWidget->currentIndex());

    // if the current controller widget
    // is disabled (by the user), 
    // we don't need to do anything
    if (!controllerWidget->IsPluggedIn())
    {
        this->sdlThread->SetAction(SDLThreadAction::None);
        return;
    }

    // verify/update the SDL thread action state
    if (this->sdlThread->GetCurrentAction() == SDLThreadAction::GetInputDevices)
    { // don't do anything when we're querying the input devices
        return;
    }
    else if (this->sdlThread->GetCurrentAction() == SDLThreadAction::None)
    { // when the SDL thread is doing nothing, pump the sdl events instead
        sdlThread->SetAction(SDLThreadAction::SDLPumpEvents);
    }

    // check if controller has been disconnected,
    // if so, keep trying to re-open it
    if (!SDL_GameControllerGetAttached(this->currentController))
    {
        this->closeController();
        this->openController(this->currentDeviceName, this->currentDeviceNum);
    }

    // process SDL events
    SDL_Event event;
    while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, 0, SDL_LASTEVENT) == 1)
    {
        controllerWidget->on_MainDialog_SdlEvent(&event);
    }

    controllerWidget->on_MainDialog_SdlEventPollFinished();
}

void MainDialog::on_ControllerWidget_CurrentInputDeviceChanged(QString deviceName, int deviceNum)
{
    this->closeController();
    this->openController(deviceName, deviceNum);
}

void MainDialog::on_ControllerWidget_RefreshInputDevicesButtonClicked()
{
    if (this->updatingDeviceList)
    {
        return;
    }

    this->updatingDeviceList = true;
    inputDeviceList.clear();
    this->sdlThread->SetAction(SDLThreadAction::GetInputDevices);
}

void MainDialog::on_tabWidget_currentChanged(int index)
{
    QString deviceName;
    int deviceNum;

    Widget::ControllerWidget* controllerWidget = controllerWidgets.at(index);
    controllerWidget->ClearControllerImage();

    this->closeController();

    controllerWidget->GetCurrentInputDevice(deviceName, deviceNum);

    this->openController(deviceName, deviceNum);
}

void MainDialog::on_SDLThread_DeviceFound(QString deviceName, int deviceNum)
{
    inputDevice_t inputDevice = {deviceName, deviceNum};
    this->inputDeviceList.append(inputDevice);
}

void MainDialog::on_SDLThread_DeviceSearchFinished(void)
{
    // compare inputDeviceList with oldInputDeviceList
    // and signal the changes
    for (auto& inputDevice : this->inputDeviceList)
    {
        if (!this->oldInputDeviceList.contains(inputDevice))
        {
            this->addInputDevice(inputDevice.deviceName, inputDevice.deviceNum);
        }
    }
    for (auto& inputDevice : this->oldInputDeviceList)
    {
        if (!this->inputDeviceList.contains(inputDevice))
        {
            this->removeInputDevice(inputDevice.deviceName, inputDevice.deviceNum);
        }
    }

    // update old device list
    this->oldInputDeviceList = this->inputDeviceList;

    // we can refresh safely again
    this->updatingDeviceList = false;
}

