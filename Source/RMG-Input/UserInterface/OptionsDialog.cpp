/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "OptionsDialog.hpp"

#include <QFileDialog>
#include <RMG-Core/Core.hpp>

using namespace UserInterface;

OptionsDialog::OptionsDialog(QWidget* parent, OptionsDialogSettings settings) : QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    this->setupUi(this);

    this->controllerPakComboBox->setCurrentIndex(settings.ControllerPak);
    this->gameboyRomLineEdit->setText(QString::fromStdString(settings.GameboyRom));
    this->gameboySaveLineEdit->setText(QString::fromStdString(settings.GameboySave));
    this->removeDuplicateMappingsCheckbox->setChecked(settings.RemoveDuplicateMappings);
}

bool OptionsDialog::HasSaved()
{
    return this->hasSaved;
}

OptionsDialogSettings OptionsDialog::GetSettings()
{
    return this->settings;
}

void OptionsDialog::on_buttonBox_clicked(QAbstractButton *button)
{

    QPushButton *pushButton = (QPushButton *)button;
    QPushButton *okButton = this->buttonBox->button(QDialogButtonBox::Ok);

    if (pushButton == okButton)
    {
        this->hasSaved = true;
        this->settings.ControllerPak = this->controllerPakComboBox->currentIndex();
        this->settings.GameboyRom = this->gameboyRomLineEdit->text().toStdString();
        this->settings.GameboySave = this->gameboySaveLineEdit->text().toStdString();
        this->settings.RemoveDuplicateMappings = this->removeDuplicateMappingsCheckbox->isChecked();
    }
    
    this->accept();
}

void OptionsDialog::on_changeGameboyRomButton_clicked()
{
    QString gameBoyRom;
    gameBoyRom = QFileDialog::getOpenFileName(this, "", "", "Gameboy ROM (*.gb *.gbc)");

    if (!gameBoyRom.isEmpty())
    {
        this->gameboyRomLineEdit->setText(gameBoyRom);
    }
}

void OptionsDialog::on_changeGameboySaveButton_clicked()
{
    QString gameBoySave;
    gameBoySave = QFileDialog::getOpenFileName(this, "", "", "Gameboy SAVE (*.sav *.ram)");

    if (!gameBoySave.isEmpty())
    {
        this->gameboySaveLineEdit->setText(gameBoySave);
    }
}
