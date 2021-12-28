/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef COMMON_HPP
#define COMMON_HPP

enum class N64ControllerButton
{
    A = 0,
    B,
    Start,
    DpadUp,
    DpadDown,
    DpadLeft,
    DpadRight,
    CButtonUp,
    CButtonDown,
    CButtonLeft,
    CButtonRight,
    LeftTrigger,
    RightTrigger,
    ZTrigger,
    Invalid
};

enum class InputType
{
    Keyboard = -1,
    GamepadButton,
    GamepadAxis,
    Invalid
};

enum class InputAxisDirection
{
    Up = 0,
    Down,
    Left,
    Right
};

#endif // COMMON_HPP
