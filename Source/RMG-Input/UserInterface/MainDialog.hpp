/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QDialog>

#include "ui_MainDialog.h"
#include "Widget/ControllerWidget.hpp"
#include "Thread/SDLThread.hpp"

namespace UserInterface
{
class MainDialog : public QDialog, private Ui::MainDialog
{
Q_OBJECT
private:
    struct inputDevice_t
    {
        QString deviceName;
        int deviceNum;

        bool operator== (inputDevice_t other)
        {
            return other.deviceNum == deviceNum &&
                other.deviceName == deviceName;
        }
    };

    QTimer* inputPollTimer;
    Thread::SDLThread* sdlThread;

    QList<inputDevice_t> oldInputDeviceList;
    QList<inputDevice_t> inputDeviceList;
    bool updatingDeviceList = false;

    QList<Widget::ControllerWidget*> controllerWidgets;
    SDL_GameController* currentController = nullptr;
    QString currentDeviceName;
    int currentDeviceNum = 0;

    void addInputDevice(QString, int);
    void removeInputDevice(QString, int);

    void openController(QString, int);
    void closeController();

public:
    MainDialog(QWidget *parent, Thread::SDLThread*);
    ~MainDialog(void);

public slots:
    void on_InputPollTimer_triggered();

    void on_ControllerWidget_CurrentInputDeviceChanged(QString, int);
    void on_ControllerWidget_RefreshInputDevicesButtonClicked();
   
    void on_tabWidget_currentChanged(int);

    void on_SDLThread_DeviceFound(QString, int);
    void on_SDLThread_DeviceSearchFinished(void);
};
}

#endif // MAINWINDOW_HPP
