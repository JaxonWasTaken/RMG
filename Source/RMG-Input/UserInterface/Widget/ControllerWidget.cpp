/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "ControllerWidget.hpp"
#include "SDL_events.h"
#include "SDL_gamecontroller.h"

#include <QPixmap>
#include <QResizeEvent>
#include <QPainter>
#include <SDL.h>
#include <QSvgRenderer>
#include <iostream>

#define SDL_AXIS_PEAK 32768

using namespace UserInterface::Widget;

ControllerWidget::ControllerWidget(QWidget* parent) : QWidget(parent)
{
    this->setupUi(this);

    this->deadZoneSlider->setValue(15);
    this->analogStickRangeSlider->setValue(100);
    this->controllerPluggedCheckBox->setChecked(false);

    this->DrawControllerImage();

    this->buttonWidgetMappings.append(
    {
        { N64ControllerButton::A, this->aButton, },
        { N64ControllerButton::B, this->bButton, },
        { N64ControllerButton::Start, this->startButton },
        { N64ControllerButton::DpadUp, this->dpadUpButton },
        { N64ControllerButton::DpadDown, this->dpadDownButton },
        { N64ControllerButton::DpadLeft, this->dpadLeftButton },
        { N64ControllerButton::DpadRight, this->dpadRightButton },
        { N64ControllerButton::CButtonUp, this->cbuttonUpButton },
        { N64ControllerButton::CButtonDown, this->cbuttonDownButton },
        { N64ControllerButton::CButtonLeft, this->cbuttonLeftButton  },
        { N64ControllerButton::CButtonRight, this->cbuttonRightButton },
        { N64ControllerButton::LeftTrigger, this->leftTriggerButton },
        { N64ControllerButton::RightTrigger, this->rightTriggerButton },
    });

    this->joystickWidgetMappings.append(
    {
        { InputAxisDirection::Up, this->analogStickUpButton },
        { InputAxisDirection::Down, this->analogStickDownButton },
        { InputAxisDirection::Left, this->analogStickLeftButton },
        { InputAxisDirection::Right, this->analogStickRightButton },
    });

    this->initializeButtons();
}

ControllerWidget::~ControllerWidget()
{

}

void ControllerWidget::initializeButtons()
{
    CustomButton* buttonList[] =
    {
        // dpad
        this->dpadUpButton,
        this->dpadDownButton,
        this->dpadLeftButton,
        this->dpadRightButton,
        // analog stick
        this->analogStickUpButton,
        this->analogStickDownButton,
        this->analogStickLeftButton,
        this->analogStickRightButton,
        // cbuttons
        this->cbuttonUpButton,
        this->cbuttonDownButton,
        this->cbuttonLeftButton,
        this->cbuttonRightButton,
        // triggers
        this->leftTriggerButton,
        this->rightTriggerButton,
        this->zTriggerButton,
        // buttons
        this->aButton,
        this->bButton,
        this->startButton,
    };

    for (auto& button : buttonList)
    {
        button->Initialize(this);
        button->setText(" ");
    }
}

void ControllerWidget::AddInputDevice(QString deviceName, int deviceNum)
{
    QString name = deviceName;

    if (deviceNum != -1)
    {
        name += " (";
        name += QString::number(deviceNum);
        name += ")";
    }

    this->inputDeviceNameList.append(deviceName);
    this->inputDeviceComboBox->addItem(name, deviceNum);
}

void ControllerWidget::RemoveInputDevice(QString deviceName, int deviceNum)
{
    inputDeviceNameList.removeOne(deviceName);

    for (int i = 0; i < this->inputDeviceComboBox->count(); i++)
    {
        int tmpNum = this->inputDeviceComboBox->itemData(i).toInt();
        QString tmpName = this->inputDeviceComboBox->itemText(i);

        if (tmpName.contains(deviceName) && deviceNum == tmpNum)
        {
            this->inputDeviceComboBox->removeItem(i);
            break;
        }
    }
}

void ControllerWidget::DrawControllerImage()
{
    this->controllerImageWidget->UpdateImage();
}

void ControllerWidget::ClearControllerImage()
{
    this->controllerImageWidget->ClearControllerState();
}

void ControllerWidget::GetCurrentInputDevice(QString& deviceName, int& deviceNum)
{
    int currentIndex = this->inputDeviceComboBox->currentIndex();
    deviceName = this->inputDeviceNameList.at(currentIndex);
    deviceNum = this->inputDeviceComboBox->itemData(currentIndex).toInt();
}

void ControllerWidget::on_deadZoneSlider_valueChanged(int value)
{
    QString title = tr("Deadzone: ");
    title += QString::number(value);
    title += "%";

    this->deadZoneGroupBox->setTitle(title);
    this->controllerImageWidget->SetDeadzone(value);
}

void ControllerWidget::on_analogStickRangeSlider_valueChanged(int value)
{
    QString text = QString::number(value);
    text += "%";
    this->analogStickRangeLabel->setText(text);
}

void ControllerWidget::on_inputDeviceComboBox_currentIndexChanged(int value)
{
    QString deviceName = this->inputDeviceNameList[value];
    int deviceNum = this->inputDeviceComboBox->itemData(value).toInt();

    this->ClearControllerImage();

    emit this->CurrentInputDeviceChanged(deviceName, deviceNum);
}

void ControllerWidget::on_inputDeviceRefreshButton_clicked()
{
    emit this->RefreshInputDevicesButtonClicked();
}

void ControllerWidget::on_controllerPluggedCheckBox_toggled(bool value)
{
    QWidget* widgetList[] =
    {
        // dpad
        this->dpadUpButton,
        this->dpadDownButton,
        this->dpadLeftButton,
        this->dpadRightButton,
        // analog stick
        this->analogStickUpButton,
        this->analogStickDownButton,
        this->analogStickLeftButton,
        this->analogStickRightButton,
        this->analogStickRangeSlider,
        // cbuttons
        this->cbuttonUpButton,
        this->cbuttonDownButton,
        this->cbuttonLeftButton,
        this->cbuttonRightButton,
        // triggers
        this->leftTriggerButton,
        this->rightTriggerButton,
        this->zTriggerButton,
        // buttons
        this->aButton,
        this->bButton,
        this->startButton,
        // misc UI elements
        this->deadZoneSlider,
        this->setupButton,
        this->pushButton_16,
        this->inputDeviceComboBox,
        this->inputDeviceRefreshButton
    };

    for (auto& widget : widgetList)
    {
        widget->setEnabled(value);
    }

    this->ClearControllerImage();
}

void ControllerWidget::on_setupButton_clicked()
{
    CustomButton* buttonList[] =
    {
        this->startButton,
    };

    for (auto& button : buttonList)
    {
        button->setFocus(Qt::OtherFocusReason);
        //button->activateWindow();
        button->click();
    }
}

void ControllerWidget::on_CustomButton_released(CustomButton* button)
{
    if (this->currentButton != nullptr)
    {
        this->currentButton->StopTimer();
        this->currentButton->RestoreState();
    }

    this->currentButton = button;
    button->SaveState();
    button->StartTimer();
}

void ControllerWidget::on_CustomButton_TimerFinished(CustomButton* button)
{
    if (this->currentButton == button)
    {
        this->currentButton = nullptr;
    }

    button->RestoreState();
}

void ControllerWidget::on_MainDialog_SdlEvent(SDL_Event* event)
{
    switch (event->type)
    {
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
        { // gamepad button
            const SDL_GameControllerButton sdlButton = (SDL_GameControllerButton)event->cbutton.button;
            const bool sdlButtonPressed = (event->type == SDL_CONTROLLERBUTTONDOWN);

            // handle button widget
            if (this->currentButton != nullptr)
            {
                if (sdlButtonPressed)
                {
                    this->currentButton->SetInputData(
                        InputType::GamepadButton, 
                        sdlButton,
                        SDL_GameControllerGetStringForButton(sdlButton)
                    );
                    this->currentButton = nullptr;
                }
                break;
            }

            // update controller button state
            for (auto& button : this->buttonWidgetMappings)
            {
                if (button.buttonWidget->GetInputType() == InputType::GamepadButton &&
                    button.buttonWidget->GetInputData() == sdlButton)
                {
                    this->controllerImageWidget->SetButtonState(button.button, sdlButtonPressed);
                }
            }

            // update controller analog stick state
            for (auto& joystick : this->joystickWidgetMappings)
            {
                if (joystick.buttonWidget->GetInputType() == InputType::GamepadButton &&
                    joystick.buttonWidget->GetInputData() == sdlButton)
                {
                    switch (joystick.direction)
                    {
                        case InputAxisDirection::Up:
                        case InputAxisDirection::Down:
                        {
                            const int value = (
                                joystick.direction == InputAxisDirection::Up ?
                                    100 :
                                    -100
                            );
                            this->controllerImageWidget->SetYAxisState(sdlButtonPressed ? value : 0);
                        } break;

                        case InputAxisDirection::Left:
                        case InputAxisDirection::Right:
                        {
                            const int value = (
                                joystick.direction == InputAxisDirection::Left ?
                                    100 :
                                    -100
                            );
                            this->controllerImageWidget->SetXAxisState(sdlButtonPressed ? value : 0);
                        } break;

                        default:
                            break;
                    }
                }
            }
        } break;

        case SDL_CONTROLLERAXISMOTION:
        { // gamepad axis
            const SDL_GameControllerAxis sdlAxis = (SDL_GameControllerAxis)event->caxis.axis;
            const int sdlAxisValue = event->caxis.value;
            // make sure the user presses the axis
            // more than 50%, otherwise we might detect
            // an accidental axis movement (due to i.e deadzone)
            const bool sdlAxisButtonPressed = (abs(sdlAxisValue) >= (SDL_AXIS_PEAK / 2));
            const int sdlAxisDirection = (sdlAxisValue > 0 ? 1 : 0);
            QString name = SDL_GameControllerGetStringForAxis(sdlAxis);
            name.append(sdlAxisValue > 0 ? "+" : "-");

            // handle button widget
            if (this->currentButton != nullptr)
            {
                if (sdlAxisButtonPressed)
                {
                    this->currentButton->SetInputData(
                        InputType::GamepadAxis, 
                        sdlAxis,
                        name
                    );
                    this->currentButton->SetExtraInputData(
                        sdlAxisDirection
                    );
                    this->currentButton = nullptr;
                }
                break;
            }

            // update controller button state
            for (auto& button : this->buttonWidgetMappings)
            {
                if (button.buttonWidget->GetInputType() == InputType::GamepadAxis &&
                    button.buttonWidget->GetInputData() == sdlAxis &&
                    button.buttonWidget->GetExtraInputData() == sdlAxisDirection)
                {
                    this->controllerImageWidget->SetButtonState(button.button, sdlAxisButtonPressed);
                }
            }

            // update controller analog stick state
            for (auto& joystick : this->joystickWidgetMappings)
            {
                if (joystick.buttonWidget->GetInputType() == InputType::GamepadAxis &&
                    joystick.buttonWidget->GetInputData() == sdlAxis &&
                    joystick.buttonWidget->GetExtraInputData() == sdlAxisDirection)
                {
                    const int value = -(double)((double)sdlAxisValue / SDL_AXIS_PEAK * 100);

                    switch (joystick.direction)
                    {
                        case InputAxisDirection::Up:
                        case InputAxisDirection::Down:
                        {
                            this->controllerImageWidget->SetYAxisState(value);
                        } break;

                        case InputAxisDirection::Left:
                        case InputAxisDirection::Right:
                        {
                            this->controllerImageWidget->SetXAxisState(value);
                        } break;

                        default:
                            break;
                    }
                }
            }
        } break;

        default:
            break;
    }
}

void ControllerWidget::on_MainDialog_SdlEventPollFinished()
{
    this->controllerImageWidget->UpdateImage();
}

/*
void ControllerWidget::SetButtonState(SDL_GameControllerButton sdlButton, int state)
{
    if (this->currentButton != nullptr)
    {
        if (state)
        {
            this->currentButton->StopTimer();
            this->currentButton->SetButton(sdlButton);
            this->currentButton->setText(SDL_GameControllerGetStringForButton(sdlButton));
            this->currentButton = nullptr;
        }
        return;
    }
}

void ControllerWidget::SetAxisState(SDL_GameControllerAxis axis, int16_t state)
{
    const double statePercentage = (double)state / SDL_AXIS_PEAK * 100;

    switch (axis)
    {
        case SDL_CONTROLLER_AXIS_LEFTX:
            this->controllerImageWidget->SetXAxisState(statePercentage);
            break;
        case SDL_CONTROLLER_AXIS_LEFTY:
            this->controllerImageWidget->SetYAxisState(statePercentage);
            break;
        default:
            break;
    }
}
*/
bool ControllerWidget::IsPluggedIn()
{
    return this->controllerPluggedCheckBox->isChecked();
}
