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
#include "UserInterface/OptionsDialog.hpp"

#include "common.hpp"

#include <RMG-Core/Core.hpp>

#include <QPixmap>
#include <QResizeEvent>
#include <QPainter>
#include <SDL.h>
#include <QSvgRenderer>
#include <iostream>

using namespace UserInterface::Widget;

ControllerWidget::ControllerWidget(QWidget* parent, EventFilter* eventFilter) : QWidget(parent)
{
    this->setupUi(this);

    // install eventFilter to all children and parents
    this->installEventFilter(eventFilter);
    for (auto& widget : this->children())
    {
        widget->installEventFilter(eventFilter);
    }
    QObject* parentObject = this->parent();
    while (parentObject != nullptr)
    {
        parentObject->installEventFilter(eventFilter);
        parentObject = parentObject->parent();
    }

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

    this->buttonSettingMappings.append(
    {
        { this->aButton, SettingsID::Input_A_InputType, SettingsID::Input_A_Name, SettingsID::Input_A_Data, SettingsID::Input_A_ExtraData },
        { this->bButton, SettingsID::Input_B_InputType, SettingsID::Input_B_Name, SettingsID::Input_B_Data, SettingsID::Input_B_ExtraData },
        { this->startButton, SettingsID::Input_Start_InputType, SettingsID::Input_Start_Name, SettingsID::Input_Start_Data, SettingsID::Input_Start_ExtraData },
        { this->dpadUpButton, SettingsID::Input_DpadUp_InputType, SettingsID::Input_DpadUp_Name, SettingsID::Input_DpadUp_Data, SettingsID::Input_DpadUp_ExtraData },
        { this->dpadDownButton, SettingsID::Input_DpadDown_InputType, SettingsID::Input_DpadDown_Name, SettingsID::Input_DpadDown_Data, SettingsID::Input_DpadDown_ExtraData },
        { this->dpadLeftButton, SettingsID::Input_DpadLeft_InputType, SettingsID::Input_DpadLeft_Name, SettingsID::Input_DpadLeft_Data, SettingsID::Input_DpadLeft_ExtraData },
        { this->dpadRightButton, SettingsID::Input_DpadRight_InputType, SettingsID::Input_DpadRight_Name, SettingsID::Input_DpadRight_Data, SettingsID::Input_DpadRight_ExtraData },
        { this->cbuttonUpButton, SettingsID::Input_CButtonUp_InputType, SettingsID::Input_CButtonUp_Name, SettingsID::Input_CButtonUp_Data, SettingsID::Input_CButtonUp_ExtraData },
        { this->cbuttonDownButton, SettingsID::Input_CButtonDown_InputType, SettingsID::Input_CButtonDown_Name, SettingsID::Input_CButtonDown_Data, SettingsID::Input_CButtonDown_ExtraData },
        { this->cbuttonLeftButton, SettingsID::Input_CButtonLeft_InputType, SettingsID::Input_CButtonLeft_Name, SettingsID::Input_CButtonLeft_Data, SettingsID::Input_CButtonLeft_ExtraData },
        { this->cbuttonRightButton, SettingsID::Input_CButtonRight_InputType, SettingsID::Input_CButtonRight_Name, SettingsID::Input_CButtonRight_Data, SettingsID::Input_CButtonRight_ExtraData },
        { this->leftTriggerButton, SettingsID::Input_LeftTrigger_InputType, SettingsID::Input_LeftTrigger_Name, SettingsID::Input_LeftTrigger_Data, SettingsID::Input_LeftTrigger_ExtraData },
        { this->rightTriggerButton, SettingsID::Input_RightTrigger_InputType, SettingsID::Input_RightTrigger_Name, SettingsID::Input_RightTrigger_Data, SettingsID::Input_RightTrigger_ExtraData },
        { this->zTriggerButton, SettingsID::Input_ZTrigger_InputType, SettingsID::Input_ZTrigger_Name, SettingsID::Input_ZTrigger_Data, SettingsID::Input_ZTrigger_ExtraData },
        { this->analogStickUpButton, SettingsID::Input_AnalogStickUp_InputType, SettingsID::Input_AnalogStickUp_Name, SettingsID::Input_AnalogStickUp_Data, SettingsID::Input_AnalogStickUp_ExtraData },
        { this->analogStickDownButton, SettingsID::Input_AnalogStickDown_InputType, SettingsID::Input_AnalogStickDown_Name, SettingsID::Input_AnalogStickDown_Data, SettingsID::Input_AnalogStickDown_ExtraData },
        { this->analogStickLeftButton, SettingsID::Input_AnalogStickLeft_InputType, SettingsID::Input_AnalogStickLeft_Name, SettingsID::Input_AnalogStickLeft_Data, SettingsID::Input_AnalogStickLeft_ExtraData },
        { this->analogStickRightButton, SettingsID::Input_AnalogStickRight_InputType, SettingsID::Input_AnalogStickRight_Name, SettingsID::Input_AnalogStickRight_Data, SettingsID::Input_AnalogStickRight_ExtraData },
    });

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
        this->setupButtonWidgets.append(button);
    }


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

bool ControllerWidget::isCurrentDeviceKeyboard()
{
    return this->inputDeviceComboBox->currentIndex() == 0;
}

bool ControllerWidget::isCurrentDeviceNotFound()
{
    QString title = this->inputDeviceComboBox->currentText();
    return title.endsWith("(not found)");
}

void ControllerWidget::disableAllChildren()
{
    for (auto& object : this->children())
    {
        if (!object->inherits("QWidget"))
        {
            continue;
        }

        QWidget* widget = (QWidget*)object;
        if (widget->isEnabled())
        {
            widget->setEnabled(false);
        }
    }
}

void ControllerWidget::enableAllChildren()
{
    for (auto& object : this->children())
    {
        if (!object->inherits("QWidget"))
        {
            continue;
        }

        QWidget* widget = (QWidget*)object;
        if (!widget->isEnabled())
        {
            widget->setEnabled(true);
        }
    }
}

void ControllerWidget::removeDuplicates(CustomButton* button)
{
    std::string section = this->settingsSection.toStdString();

    if (!CoreSettingsGetBoolValue(SettingsID::Input_RemoveDuplicateMappings, section))
    {
        return;
    }

    for (auto& buttonWidget : this->setupButtonWidgets)
    {
        // skip ourselfes
        if (buttonWidget == button)
        {
            continue;
        }

        if ((buttonWidget->GetInputType() == button->GetInputType()) &&
            (buttonWidget->GetInputData() == button->GetInputData()) &&
            (buttonWidget->GetExtraInputData() == button->GetExtraInputData()))
        {
            buttonWidget->Clear();
        }
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

void ControllerWidget::CheckInputDeviceSettings()
{
    std::string section = this->settingsSection.toStdString();

    if (!CoreSettingsSectionExists(section))
    {
        return;
    }

    std::string deviceName = CoreSettingsGetStringValue(SettingsID::Input_DeviceName, section);
    int deviceNum = CoreSettingsGetIntValue(SettingsID::Input_DeviceNum, section);

    // clear (not found) devices first
    int notFoundIndex = this->inputDeviceComboBox->findText("(not found)", Qt::MatchFlag::MatchEndsWith);
    if (notFoundIndex != -1)
    {
        this->inputDeviceNameList.removeAt(notFoundIndex);
        this->inputDeviceComboBox->removeItem(notFoundIndex);
    }

    int deviceNameIndex = this->inputDeviceComboBox->findText(QString::fromStdString(deviceName), Qt::MatchFlag::MatchStartsWith);
    int deviceNumIndex = this->inputDeviceComboBox->findData(deviceNum);
    
    if ((deviceNameIndex == deviceNumIndex) &&
        (deviceNameIndex != -1))
    { // full match
        this->inputDeviceComboBox->setCurrentIndex(deviceNameIndex);
    }
    else if (deviceNameIndex != -1)
    { // name only match
        this->inputDeviceComboBox->setCurrentIndex(deviceNameIndex);
    }
    else
    { // no match
        QString title = QString::fromStdString(deviceName);
        title += " (not found)";
        this->inputDeviceNameList.append(QString::fromStdString(deviceName));
        this->inputDeviceComboBox->addItem(title, deviceNum);
        this->inputDeviceComboBox->setCurrentIndex(this->inputDeviceNameList.count() - 1);
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

void ControllerWidget::GetCurrentInputDevice(QString& deviceName, int& deviceNum, bool ignoreDeviceNotFound)
{
    int currentIndex = this->inputDeviceComboBox->currentIndex();

    if (this->isCurrentDeviceNotFound() && !ignoreDeviceNotFound)
    {
        deviceName = "";
        deviceNum = -1;
    }
    else
    {
        deviceName = this->inputDeviceNameList.at(currentIndex);
        deviceNum = this->inputDeviceComboBox->itemData(currentIndex).toInt();
    }
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
    this->controllerImageWidget->SetRange(value);
}

void ControllerWidget::on_inputDeviceComboBox_currentIndexChanged(int value)
{
    QString deviceName = this->inputDeviceNameList.at(value);
    int deviceNum = this->inputDeviceComboBox->itemData(value).toInt();

    this->ClearControllerImage();

    if (this->isCurrentDeviceNotFound())
    {
        deviceName = "";
        deviceNum = -1;
    }

    emit this->CurrentInputDeviceChanged(this, deviceName, deviceNum);
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
        this->optionsButton,
        this->setupButton,
        this->resetButton,
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
    this->currentInSetup = true;
    this->currentSetupButtonWidgetIndex = 0;

    this->setupButtonWidgets.at(0)->setFocus(Qt::OtherFocusReason);
    this->setupButtonWidgets.at(0)->click();
}

void ControllerWidget::on_resetButton_clicked()
{
    this->LoadSettings();
}

void ControllerWidget::on_optionsButton_clicked()
{
    OptionsDialog dialog(this, this->settingsSection);
    dialog.exec();
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
    
    this->disableAllChildren();
}

void ControllerWidget::on_CustomButton_TimerFinished(CustomButton* button)
{
    if (this->currentButton == button)
    {
        this->currentButton = nullptr;
    }

    button->RestoreState();

    this->enableAllChildren();
}

void ControllerWidget::on_CustomButton_DataSet(CustomButton* button)
{
    this->enableAllChildren();
    this->removeDuplicates(button);
    this->currentButton = nullptr;


/*
    if (this->currentInSetup)
    {
        if (this->currentSetupButtonWidgetIndex + 1 >= this->setupButtonWidgets.count())
        {
            this->currentInSetup = false;
            return;
        }

        this->currentSetupButtonWidgetIndex++;
        this->setupButtonWidgets.at(currentSetupButtonWidgetIndex)->setFocus(Qt::OtherFocusReason);
        this->setupButtonWidgets.at(currentSetupButtonWidgetIndex)->click();
    }*/
}

void ControllerWidget::on_MainDialog_SdlEvent(SDL_Event* event)
{
    switch (event->type)
    {
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
        { // gamepad & joystick button
            SDL_JoystickID joystickId = -1;
            InputType inputType = InputType::Invalid;
            int sdlButton = 0;
            bool sdlButtonPressed = false;
            QString sdlButtonName;

            if ((event->type == SDL_CONTROLLERBUTTONDOWN) ||
                (event->type == SDL_CONTROLLERBUTTONUP))
            { // gamepad button
                joystickId = event->cbutton.which;
                inputType = InputType::GamepadButton;
                sdlButton = event->cbutton.button;
                sdlButtonPressed = (event->type == SDL_CONTROLLERBUTTONDOWN);
                sdlButtonName = SDL_GameControllerGetStringForButton((SDL_GameControllerButton)sdlButton);
            }
            else
            { // joystick button
                joystickId = event->jbutton.which;
                inputType = InputType::JoystickButton;
                sdlButton = event->jbutton.button;
                sdlButtonPressed = (event->type == SDL_JOYBUTTONDOWN);
                sdlButtonName = "button " + QString::number(sdlButton);
            }

            // make sure we have the right joystick
            if (joystickId != this->currentJoystickId)
            {
                break;
            }

            // handle button widget
            if (this->currentButton != nullptr)
            {
                if (sdlButtonPressed)
                {
                    this->currentButton->SetInputData(
                        inputType, 
                        sdlButton,
                        0,
                        sdlButtonName
                    );
                }
                break;
            }

            // update controller button state
            for (auto& button : this->buttonWidgetMappings)
            {
                if (button.buttonWidget->GetInputType() == inputType &&
                    button.buttonWidget->GetInputData() == sdlButton)
                {
                    this->controllerImageWidget->SetButtonState(button.button, sdlButtonPressed);
                }
            }

            // update controller analog stick state
            for (auto& joystick : this->joystickWidgetMappings)
            {
                if (joystick.buttonWidget->GetInputType() == inputType &&
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
        case SDL_JOYAXISMOTION:
        { // gamepad & joystick axis
            SDL_JoystickID joystickId = -1;
            InputType inputType = InputType::Invalid;
            int sdlAxis = 0;
            int sdlAxisValue = 0;
            QString sdlAxisName;

            if (event->type == SDL_CONTROLLERAXISMOTION)
            { // gamepad axis
                joystickId = event->caxis.which;
                inputType = InputType::GamepadAxis;
                sdlAxis = event->caxis.axis;
                sdlAxisValue = event->caxis.value;
                sdlAxisName = SDL_GameControllerGetStringForAxis((SDL_GameControllerAxis)sdlAxis);
                sdlAxisName += sdlAxisValue > 0 ? "+" : "-";
            }
            else
            { // joystick axis
                joystickId = event->jaxis.which;
                inputType = InputType::JoystickAxis;
                sdlAxis = event->jaxis.axis;
                sdlAxisValue = event->jaxis.value;
                sdlAxisName = "axis " + QString::number(sdlAxis);
                sdlAxisName += sdlAxisValue > 0 ? "+" : "-";
            }

            // make sure the user presses the axis
            // more than 50%, otherwise we might detect
            // an accidental axis movement (due to i.e deadzone)
            const bool sdlAxisButtonPressed = (abs(sdlAxisValue) >= (SDL_AXIS_PEAK / 2));
            const int sdlAxisDirection = (sdlAxisValue > 0 ? 1 : 0);

            // make sure we have the right joystick
            if (joystickId != this->currentJoystickId)
            {
                break;
            }

            // handle button widget
            if (this->currentButton != nullptr)
            {
                if (sdlAxisButtonPressed)
                {
                    this->currentButton->SetInputData(
                        inputType, 
                        sdlAxis,
                        sdlAxisDirection,
                        sdlAxisName
                    );
                }
                break;
            }

            // update controller button state
            for (auto& button : this->buttonWidgetMappings)
            {
                if (button.buttonWidget->GetInputType() == inputType &&
                    button.buttonWidget->GetInputData() == sdlAxis &&
                    button.buttonWidget->GetExtraInputData() == sdlAxisDirection)
                {
                    this->controllerImageWidget->SetButtonState(button.button, sdlAxisButtonPressed);
                }
            }

            // update controller analog stick state
            for (auto& joystick : this->joystickWidgetMappings)
            {
                if (joystick.buttonWidget->GetInputType() == inputType &&
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

        case SDL_KEYDOWN:
        case SDL_KEYUP:
        { // keyboard button

            // make sure a keyboard is selected
            if (!this->isCurrentDeviceKeyboard())
            {
                break;
            }

            const SDL_Scancode sdlButton = (SDL_Scancode)event->key.keysym.scancode;
            const bool sdlButtonPressed = (event->type == SDL_KEYDOWN);

            // handle button widget
            if (this->currentButton != nullptr)
            {
                if (sdlButtonPressed)
                {
                    this->currentButton->SetInputData(
                        InputType::Keyboard, 
                        sdlButton,
                        0,
                        SDL_GetScancodeName(sdlButton)
                    );
                }
                break;
            }

            // update controller button state
            for (auto& button : this->buttonWidgetMappings)
            {
                if (button.buttonWidget->GetInputType() == InputType::Keyboard &&
                    button.buttonWidget->GetInputData() == sdlButton)
                {
                    this->controllerImageWidget->SetButtonState(button.button, sdlButtonPressed);
                }
            }

            // update controller analog stick state
            for (auto& joystick : this->joystickWidgetMappings)
            {
                if (joystick.buttonWidget->GetInputType() == InputType::Keyboard &&
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

            break;
        }

        default:
            break;
    }
}

void ControllerWidget::on_MainDialog_SdlEventPollFinished()
{
    this->controllerImageWidget->UpdateImage();
}

bool ControllerWidget::IsPluggedIn()
{
    return this->controllerPluggedCheckBox->isChecked();
}

void ControllerWidget::SetSettingsSection(QString section)
{
    this->settingsSection = section;
}

void ControllerWidget::LoadSettings()
{
    if (this->settingsSection.isEmpty())
    {
        return;
    }

    std::string section = this->settingsSection.toStdString();

    if (!CoreSettingsSectionExists(section))
    {
        this->SaveDefaultSettings();
    }

    this->controllerPluggedCheckBox->setChecked(CoreSettingsGetBoolValue(SettingsID::Input_PluggedIn, section));
    this->analogStickRangeSlider->setValue(CoreSettingsGetIntValue(SettingsID::Input_Range, section));
    this->deadZoneSlider->setValue(CoreSettingsGetIntValue(SettingsID::Input_Deadzone, section));

    for (auto& buttonSetting : this->buttonSettingMappings)
    {
        enum InputType type = (InputType)CoreSettingsGetIntValue(buttonSetting.inputTypeSettingsId, section);
        std::string    name = CoreSettingsGetStringValue(buttonSetting.nameSettingsId, section);
        int            data = CoreSettingsGetIntValue(buttonSetting.dataSettingsId, section);
        int            extraData = CoreSettingsGetIntValue(buttonSetting.extraDataSettingsId, section);

        buttonSetting.button->SetInputData(type, data, extraData, QString::fromStdString(name));
    }

}

void ControllerWidget::SaveDefaultSettings()
{
    if (this->settingsSection.isEmpty())
    {
        return;
    }

    std::string section = this->settingsSection.toStdString();

    CoreSettingsSetValue(SettingsID::Input_PluggedIn, section, false);
    CoreSettingsSetValue(SettingsID::Input_DeviceName, section, std::string("Keyboard"));
    CoreSettingsSetValue(SettingsID::Input_DeviceNum, section, -1);
    CoreSettingsSetValue(SettingsID::Input_Range, section, 100);
    CoreSettingsSetValue(SettingsID::Input_Deadzone, section, 15);
    CoreSettingsSetValue(SettingsID::Input_Pak, section, 0);
    CoreSettingsSetValue(SettingsID::Input_RemoveDuplicateMappings, section, true);

    for (auto& buttonSetting : this->buttonSettingMappings)
    {
        CoreSettingsSetValue(buttonSetting.inputTypeSettingsId, section, (int)InputType::Invalid);
        CoreSettingsSetValue(buttonSetting.nameSettingsId, section, std::string(" "));
        CoreSettingsSetValue(buttonSetting.dataSettingsId, section, 0);
        CoreSettingsSetValue(buttonSetting.extraDataSettingsId, section, 0);
    }   
}

void ControllerWidget::SaveSettings()
{
    if (this->settingsSection.isEmpty())
    {
        return;
    }
    
    QString deviceName;
    int deviceNum;
    std::string section = this->settingsSection.toStdString();

    this->GetCurrentInputDevice(deviceName, deviceNum, true);

    CoreSettingsSetValue(SettingsID::Input_PluggedIn, section, this->IsPluggedIn());
    CoreSettingsSetValue(SettingsID::Input_DeviceName, section, deviceName.toStdString());
    CoreSettingsSetValue(SettingsID::Input_DeviceNum, section, deviceNum);
    CoreSettingsSetValue(SettingsID::Input_Range, section, this->analogStickRangeSlider->value());
    CoreSettingsSetValue(SettingsID::Input_Deadzone, section, this->deadZoneSlider->value());

    for (auto& buttonSetting : this->buttonSettingMappings)
    {
        CoreSettingsSetValue(buttonSetting.inputTypeSettingsId, section, (int)buttonSetting.button->GetInputType());
        CoreSettingsSetValue(buttonSetting.nameSettingsId, section, buttonSetting.button->text().toStdString());
        CoreSettingsSetValue(buttonSetting.dataSettingsId, section, buttonSetting.button->GetInputData());
        CoreSettingsSetValue(buttonSetting.extraDataSettingsId, section, buttonSetting.button->GetExtraInputData());
    }
}

void ControllerWidget::SetCurrentJoystickID(SDL_JoystickID joystickId)
{
    this->currentJoystickId = joystickId;
}

