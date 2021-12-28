/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CONTROLLERWIDGET_H
#define CONTROLLERWIDGET_H

#include "CustomButton.hpp"
#include "ControllerImageWidget.hpp"

using namespace UserInterface::Widget;

#include "ui_ControllerWidget.h"
#include "common.hpp"

#include <SDL.h>

namespace UserInterface
{
namespace Widget
{
class ControllerWidget : public QWidget, Ui::ControllerWidget
{
    Q_OBJECT

private:
    QList<QString> inputDeviceNameList;
    CustomButton* currentButton = nullptr;

    struct buttonWidgetMapping
    {
        enum N64ControllerButton button;
        CustomButton* buttonWidget;
    };

    struct axisWidgetMapping
    {
        enum InputAxisDirection direction;
        CustomButton* buttonWidget;
    };

    QList<buttonWidgetMapping> buttonWidgetMappings;
    QList<axisWidgetMapping> joystickWidgetMappings;

    void initializeButtons();
public:
    ControllerWidget(QWidget* parent);
    ~ControllerWidget();

    void AddInputDevice(QString deviceName, int deviceNum);
    void RemoveInputDevice(QString deviceName, int deviceNum);

    void DrawControllerImage();
    void ClearControllerImage();

    void GetCurrentInputDevice(QString& deviceName, int& deviceNum);
    bool IsPluggedIn();

private slots:
    void on_deadZoneSlider_valueChanged(int value);
    void on_analogStickRangeSlider_valueChanged(int value);
    
    void on_inputDeviceComboBox_currentIndexChanged(int value);
    void on_inputDeviceRefreshButton_clicked();
    
    void on_controllerPluggedCheckBox_toggled(bool value);
    void on_setupButton_clicked();

public slots:
    void on_CustomButton_released(CustomButton* button);
    void on_CustomButton_TimerFinished(CustomButton* button);

    void on_MainDialog_SdlEvent(SDL_Event* event);
    void on_MainDialog_SdlEventPollFinished();
signals:
    void CurrentInputDeviceChanged(QString deviceName, int deviceNum);
    void RefreshInputDevicesButtonClicked();
};
}
}

#endif // CONTROLLERWIDGET_H