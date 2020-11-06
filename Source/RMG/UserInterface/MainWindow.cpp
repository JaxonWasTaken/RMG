/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include "MainWindow.hpp"
#include "Config.hpp"
#include "../Utilities/QtKeyToSdl2Key.hpp"

#include <QMenuBar>
#include <QStatusBar>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QString>
#include <QSettings>
#include <QFileDialog>

using namespace UserInterface;
using namespace M64P::Wrapper;

MainWindow::MainWindow() : QMainWindow(nullptr)
{
}

MainWindow::~MainWindow()
{
}

bool MainWindow::Init(void)
{
    if (!g_Logger.Init())
    {
        this->ui_MessageBox("Error", "Logger::Init Failed", g_Logger.GetLastError());
        return false;
    }

    if (!g_MupenApi.Init(MUPEN_CORE_FILE))
    {
        this->ui_MessageBox("Error", "Api::Init Failed", g_MupenApi.GetLastError());
        return false;
    }

    // TODO, move this crap to a plugin loader
    QList<Plugin_t> pluginList;
    QString settingsArray[4] = {SETTINGS_PLUGIN_GFX, SETTINGS_PLUGIN_AUDIO, SETTINGS_PLUGIN_INPUT, SETTINGS_PLUGIN_RSP};
    QString settingsValue;
    PluginType type;
    QComboBox *comboBox;
    bool found;

    for (int i = 0; i < 4; i++)
    {
        found = false;
        type = (PluginType)i;

        pluginList.clear();
        pluginList = g_MupenApi.Core.GetPlugins((PluginType)i);

        g_MupenApi.Config.GetOption(SETTINGS_SECTION, settingsArray[i], &settingsValue);

        if (!settingsValue.isEmpty())
        {
            for (Plugin_t &p : pluginList)
            {
                if (p.FileName == settingsValue)
                {
                    g_MupenApi.Core.SetPlugin(p);
                    found = true;
                    break;
                }
            }
        }

        if (!found)
            g_MupenApi.Core.SetPlugin(pluginList.first());
    }

    g_MupenApi.Config.SetOption("Core", "ScreenshotPath", "Screenshots");
    g_MupenApi.Config.SetOption("Core", "SaveStatePath", "Save/State");
    g_MupenApi.Config.SetOption("Core", "SaveSRAMPath", "Save/Game");
    g_MupenApi.Config.SetOption("Core", "SharedDataPath", "Data");

    g_MupenApi.Config.OverrideUserPaths("Data", "Cache");

    this->ui_Init();
    this->ui_Setup();

    this->menuBar_Init();
    this->menuBar_Setup(false, false);

    this->emulationThread_Init();
    this->emulationThread_Connect();

    g_EmuThread = this->emulationThread;

    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    g_MupenApi.Config.SetOption(SETTINGS_SECTION, SETTINGS_GEOMETRY, QString(this->saveGeometry().toBase64().toStdString().c_str()));

    this->on_Action_File_EndEmulation();

    while (g_EmuThread->isRunning())
        QCoreApplication::processEvents();

    QMainWindow::closeEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!g_MupenApi.Core.IsEmulationRunning())
    {
        QMainWindow::keyPressEvent(event);
        return;
    }

    int key = Utilities::QtKeyToSdl2Key(event->key());
    int mod = Utilities::QtModKeyToSdl2ModKey(event->modifiers());

    g_MupenApi.Core.SetKeyDown(key, mod);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (!g_MupenApi.Core.IsEmulationRunning())
    {
        QMainWindow::keyReleaseEvent(event);
        return;
    }

    int key = Utilities::QtKeyToSdl2Key(event->key());
    int mod = Utilities::QtModKeyToSdl2ModKey(event->modifiers());

    g_MupenApi.Core.SetKeyUp(key, mod);
}

void MainWindow::ui_Init(void)
{
    this->ui_Icon = QIcon(":Resource/RMG.png");

    this->ui_Widgets = new QStackedWidget();
    this->ui_Widget_RomBrowser = new Widget::RomBrowserWidget();
    this->ui_Widget_OpenGL = new Widget::OGLWidget();

    QString dir;
    g_MupenApi.Config.GetOption(SETTINGS_SECTION, SETTINGS_DIRECTORY, &dir);

    this->ui_Widget_RomBrowser->SetDirectory(dir);
    this->ui_Widget_RomBrowser->RefreshRomList();

    connect(this->ui_Widget_RomBrowser, &Widget::RomBrowserWidget::on_RomBrowser_Select, this, &MainWindow::on_RomBrowser_Selected);
}

void MainWindow::ui_Setup(void)
{
    this->ui_Stylesheet_Setup();

    this->setWindowIcon(this->ui_Icon);
    this->setWindowTitle(WINDOW_TITLE);
    this->setCentralWidget(this->ui_Widgets);

    QString geometry;
    g_MupenApi.Config.GetOption(SETTINGS_SECTION, SETTINGS_GEOMETRY, &geometry);
    this->restoreGeometry(QByteArray::fromBase64(geometry.toLocal8Bit()));

    this->statusBar()->showMessage("Core Library Hooked");

    this->ui_Widgets->addWidget(this->ui_Widget_RomBrowser);
    this->ui_Widgets->addWidget(this->ui_Widget_OpenGL->GetWidget());

    this->ui_Widgets->setCurrentIndex(0);
}

void MainWindow::ui_Stylesheet_Setup(void)
{
    QFile stylesheet(APP_STYLESHEET_FILE);

    if (!stylesheet.open(QIODevice::ReadOnly))
        return;

    this->setStyleSheet(stylesheet.readAll());
}

void MainWindow::ui_MessageBox(QString title, QString text, QString details = "")
{
    g_Logger.AddText("MainWindow::ui_MessageBox: " + title + ", " + text + ", " + details);

    QMessageBox msgBox;
    msgBox.setWindowIcon(this->ui_Icon);
    msgBox.setIcon(QMessageBox::Icon::Critical);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setDetailedText(details);
    msgBox.addButton(QMessageBox::Ok);
    msgBox.show();
    msgBox.exec();
}

void MainWindow::ui_InEmulation(bool inEmulation, bool isPaused)
{
    this->menuBar_Setup(inEmulation, isPaused);

    if (inEmulation)
    {
        RomInfo_t info = { 0 };
        g_MupenApi.Core.GetRomInfo(&info);

        this->setWindowTitle(info.Settings.goodname + QString(" - ") + QString(WINDOW_TITLE));

        this->ui_Widgets->setCurrentIndex(1);
    }
    else
    {
        this->setWindowTitle(QString(WINDOW_TITLE));
        this->ui_Widgets->setCurrentIndex(0);
    }
}

void MainWindow::ui_SaveGeometry(void)
{
    if (this->ui_Geometry_Saved)
        return;

    this->ui_MinSize = this->minimumSize();
    this->ui_MaxSize = this->maximumSize();
    this->ui_Geometry = this->saveGeometry();
    this->ui_Geometry_Saved = true;
}

void MainWindow::ui_LoadGeometry(void)
{
    if (!this->ui_Geometry_Saved)
        return;

    this->setMinimumSize(0, 0);
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    // this->setMinimumSize(this->ui_MinSize);
    // this->setMaximumSize(this->ui_MaxSize);
    this->restoreGeometry(this->ui_Geometry);
    this->ui_Geometry_Saved = false;
}

void MainWindow::menuBar_Init(void)
{
    this->menuBar = new QMenuBar();
    this->menuBar_Actions_Init();
    this->menuBar_Actions_Connect();
}

void MainWindow::menuBar_Setup(bool inEmulation, bool isPaused)
{
    this->menuBar_Actions_Setup(inEmulation, isPaused);

    this->menuBar->clear();

    this->menuBar_Menu = this->menuBar->addMenu("File");
    this->menuBar_Menu->addAction(this->action_File_OpenRom);
    this->menuBar_Menu->addAction(this->action_File_OpenCombo);
    this->menuBar_Menu->addAction(this->action_File_RomInfo);
    this->menuBar_Menu->addSeparator();
    this->menuBar_Menu->addAction(this->action_File_StartEmulation);
    this->menuBar_Menu->addAction(this->action_File_EndEmulation);
    this->menuBar_Menu->addSeparator();

    // TODO: Add language support
    QMenu *langMenu = new QMenu("Language", this);
    langMenu->addAction("English");
    this->menuBar_Menu->addMenu(langMenu);

    if (!inEmulation)
    {
        this->menuBar_Menu->addSeparator();
        this->menuBar_Menu->addAction(this->action_File_ChooseDirectory);
        this->menuBar_Menu->addAction(this->action_File_RefreshRomList);
        this->menuBar_Menu->addSeparator();

        // TODO: Actually implement this
        /*
        this->menuBar_Menu->addAction(this->action_File_RecentRom);
        this->menuBar_Menu->addAction(this->action_File_RecentRomDirectories);
        */
    }
    this->menuBar_Menu->addSeparator();
    this->menuBar_Menu->addAction(this->action_File_Exit);

    if (inEmulation)
    {
        QMenu *resetMenu = new QMenu("Reset", this);
        resetMenu->addAction(action_System_SoftReset);
        resetMenu->addAction(action_System_HardReset);

        this->menuBar_Menu = this->menuBar->addMenu("System");
        this->menuBar_Menu->addMenu(resetMenu);
        this->menuBar_Menu->addAction(action_System_Pause);
        this->menuBar_Menu->addAction(this->action_System_GenerateBitmap);
        this->menuBar_Menu->addSeparator();
        this->menuBar_Menu->addAction(this->action_System_LimitFPS);
        this->menuBar_Menu->addSeparator();
        this->menuBar_Menu->addAction(this->action_System_SwapDisk);
        this->menuBar_Menu->addSeparator();
        this->menuBar_Menu->addAction(this->action_System_SaveState);
        this->menuBar_Menu->addAction(this->action_System_SaveAs);
        this->menuBar_Menu->addAction(this->action_System_LoadState);
        this->menuBar_Menu->addAction(this->action_System_Load);
        this->menuBar_Menu->addSeparator();
        this->menuBar_Menu->addMenu(this->menu_System_CurrentSaveState);

        this->menu_System_CurrentSaveState->clear();

        QActionGroup *slotActionGroup = new QActionGroup(this);
        QList<QAction *> slotActions;
        QAction *slotAction;
        for (int i = 0; i < 10; i++)
        {
            slotActions.append(new QAction(this));
            slotAction = slotActions.at(i);

            slotAction->setText("Slot " + QString::number(i + 1));
            slotAction->setCheckable(true);
            slotAction->setChecked(i == 0);
            slotAction->setActionGroup(slotActionGroup);

            connect(slotAction, &QAction::triggered, [=](bool checked) {
                if (checked)
                {
                    int slot = slotAction->text().split(" ").last().toInt() - 1;
                    this->on_Action_System_CurrentSaveState(slot);
                }
            });

            this->menu_System_CurrentSaveState->addAction(slotAction);
        }

        this->menuBar_Menu->addSeparator();
        this->menuBar_Menu->addAction(this->action_System_Cheats);
        this->menuBar_Menu->addAction(this->action_System_GSButton);
    }

    this->menuBar_Menu = this->menuBar->addMenu("Options");
    this->menuBar_Menu->addAction(this->action_Options_FullScreen);
    this->menuBar_Menu->addSeparator();
    this->menuBar_Menu->addAction(this->action_Options_ConfigGfx);
    this->menuBar_Menu->addAction(this->action_Options_ConfigAudio);
    this->menuBar_Menu->addAction(this->action_Options_ConfigRsp);
    this->menuBar_Menu->addAction(this->action_Options_ConfigControl);
    this->menuBar_Menu->addSeparator();
    this->menuBar_Menu->addAction(this->action_Options_Settings);

    this->menuBar_Menu = this->menuBar->addMenu("Help");
    this->menuBar_Menu->addAction(this->action_Help_Support);
    this->menuBar_Menu->addAction(this->action_Help_HomePage);
    this->menuBar_Menu->addSeparator();
    this->menuBar_Menu->addAction(this->action_Help_About);

    this->setMenuBar(menuBar);
}

void MainWindow::emulationThread_Init(void)
{
    this->emulationThread = new Thread::EmulationThread();
}

void MainWindow::emulationThread_Connect(void)
{
    connect(this->emulationThread, &Thread::EmulationThread::on_Emulation_Finished, this, &MainWindow::on_Emulation_Finished);
    connect(this->emulationThread, &Thread::EmulationThread::on_Emulation_Started, this, &MainWindow::on_Emulation_Started);

    connect(this->emulationThread, &Thread::EmulationThread::on_VidExt_Init, this, &MainWindow::on_VidExt_Init, Qt::BlockingQueuedConnection);
    connect(this->emulationThread, &Thread::EmulationThread::on_VidExt_SetupOGL, this, &MainWindow::on_VidExt_SetupOGL, Qt::BlockingQueuedConnection);
    connect(this->emulationThread, &Thread::EmulationThread::on_VidExt_SetMode, this, &MainWindow::on_VidExt_SetMode, Qt::BlockingQueuedConnection);
    connect(this->emulationThread, &Thread::EmulationThread::on_VidExt_SetModeWithRate, this, &MainWindow::on_VidExt_SetModeWithRate, Qt::BlockingQueuedConnection);
    connect(this->emulationThread, &Thread::EmulationThread::on_VidExt_ResizeWindow, this, &MainWindow::on_VidExt_ResizeWindow, Qt::BlockingQueuedConnection);
    connect(this->emulationThread, &Thread::EmulationThread::on_VidExt_SetCaption, this, &MainWindow::on_VidExt_SetCaption, Qt::BlockingQueuedConnection);
    connect(this->emulationThread, &Thread::EmulationThread::on_VidExt_ToggleFS, this, &MainWindow::on_VidExt_ToggleFS, Qt::BlockingQueuedConnection);
    connect(this->emulationThread, &Thread::EmulationThread::on_VidExt_Quit, this, &MainWindow::on_VidExt_Quit, Qt::BlockingQueuedConnection);
}

void MainWindow::emulationThread_Launch(QString file)
{
    g_MupenApi.Config.Save();

    if (this->emulationThread->isRunning())
    {
        this->on_Action_File_EndEmulation();

        while (this->emulationThread->isRunning())
            QCoreApplication::processEvents();
    }

    this->emulationThread->SetRomFile(file);
    this->emulationThread->start();
}

void MainWindow::menuBar_Actions_Init(void)
{
    this->action_File_OpenRom = new QAction(this);
    this->action_File_OpenCombo = new QAction(this);
    this->action_File_RomInfo = new QAction(this);
    this->action_File_StartEmulation = new QAction(this);
    this->action_File_EndEmulation = new QAction(this);
    this->action_File_Language = new QAction(this);
    this->action_File_ChooseDirectory = new QAction(this);
    this->action_File_RefreshRomList = new QAction(this);
    this->action_File_RecentRom = new QAction(this);
    this->action_File_RecentRomDirectories = new QAction(this);
    this->action_File_Exit = new QAction(this);

    this->action_System_SoftReset = new QAction(this);
    this->action_System_HardReset = new QAction(this);
    this->action_System_Pause = new QAction(this);
    this->action_System_GenerateBitmap = new QAction(this);
    this->action_System_LimitFPS = new QAction(this);
    this->action_System_SwapDisk = new QAction(this);
    this->action_System_SaveState = new QAction(this);
    this->action_System_SaveAs = new QAction(this);
    this->action_System_LoadState = new QAction(this);
    this->action_System_Load = new QAction(this);
    this->menu_System_CurrentSaveState = new QMenu(this);
    this->action_System_Cheats = new QAction(this);
    this->action_System_GSButton = new QAction(this);

    this->action_Options_FullScreen = new QAction(this);
    this->action_Options_ConfigGfx = new QAction(this);
    this->action_Options_ConfigAudio = new QAction(this);
    this->action_Options_ConfigRsp = new QAction(this);
    this->action_Options_ConfigControl = new QAction(this);
    this->action_Options_Settings = new QAction(this);

    this->action_Help_Support = new QAction(this);
    this->action_Help_HomePage = new QAction(this);
    this->action_Help_About = new QAction(this);
}

void MainWindow::menuBar_Actions_Setup(bool inEmulation, bool isPaused)
{
    this->action_File_OpenRom->setText("Open ROM");
    this->action_File_OpenRom->setShortcut(QKeySequence("Ctrl+O"));
    this->action_File_OpenCombo->setText("Open Combo");
    this->action_File_OpenCombo->setShortcut(QKeySequence("Ctrl+Shift+O"));
    this->action_File_RomInfo->setText("Rom Info...");
    this->action_File_StartEmulation->setText("Start Emulation");
    this->action_File_StartEmulation->setShortcut(QKeySequence("F11"));
    this->action_File_StartEmulation->setEnabled(!inEmulation);
    this->action_File_EndEmulation->setText("End Emulation");
    this->action_File_EndEmulation->setShortcut(QKeySequence("F12"));
    this->action_File_EndEmulation->setEnabled(inEmulation);
    this->action_File_Language->setText("Language");
    this->action_File_ChooseDirectory->setText("Choose ROM Directory...");
    this->action_File_RefreshRomList->setText("Refresh ROM List");
    this->action_File_RefreshRomList->setShortcut(QKeySequence("F5"));
    this->action_File_RecentRom->setText("Recent ROM");
    this->action_File_RecentRomDirectories->setText("Recent ROM Directories");
    this->action_File_Exit->setText("Exit");
    this->action_File_Exit->setShortcut(QKeySequence("Alt+F4"));

    this->action_System_SoftReset->setText("Soft Reset");
    this->action_System_SoftReset->setShortcut(QKeySequence("F1"));
    this->action_System_HardReset->setText("Hard Reset");
    this->action_System_HardReset->setShortcut(QKeySequence("Shift+F1"));
    this->action_System_Pause->setText(isPaused ? "Resume" : "Pause");
    this->action_System_Pause->setShortcut(QKeySequence("F2"));
    this->action_System_GenerateBitmap->setText("Generate Bitmap");
    this->action_System_GenerateBitmap->setShortcut(QKeySequence("F3"));
    this->action_System_LimitFPS->setText("Limit FPS");
    this->action_System_LimitFPS->setShortcut(QKeySequence("F4"));
    this->action_System_LimitFPS->setCheckable(true);
    this->action_System_LimitFPS->setChecked(true);
    this->action_System_SwapDisk->setText("Swap Disk");
    this->action_System_SwapDisk->setShortcut(QKeySequence("Ctrl+D"));
    this->action_System_SwapDisk->setEnabled(false);
    this->action_System_SaveState->setText("Save State");
    this->action_System_SaveState->setShortcut(QKeySequence("F5"));
    this->action_System_SaveAs->setText("Save As...");
    this->action_System_SaveAs->setShortcut(QKeySequence("Ctrl+S"));
    this->action_System_LoadState->setText("Load State");
    this->action_System_LoadState->setShortcut(QKeySequence("F7"));
    this->action_System_Load->setText("Load...");
    this->action_System_Load->setShortcut(QKeySequence("Ctrl+L"));
    this->menu_System_CurrentSaveState->setTitle("Current Save State");
    this->action_System_Cheats->setText("Cheats...");
    this->action_System_Cheats->setShortcut(QKeySequence("Ctrl+C"));
    this->action_System_GSButton->setText("GS Button");
    this->action_System_GSButton->setShortcut(QKeySequence("F9"));

    this->action_Options_FullScreen->setText("Full Screen");
    this->action_Options_FullScreen->setEnabled(inEmulation);
    this->action_Options_FullScreen->setShortcut(QKeySequence("Alt+Return"));
    this->action_Options_ConfigGfx->setText("Configure Graphics Plugin...");
    this->action_Options_ConfigGfx->setEnabled(g_MupenApi.Core.HasPluginConfig(M64P::Wrapper::PluginType::Gfx));
    this->action_Options_ConfigAudio->setText("Configure Audio Plugin...");
    this->action_Options_ConfigAudio->setEnabled(g_MupenApi.Core.HasPluginConfig(M64P::Wrapper::PluginType::Audio));
    this->action_Options_ConfigRsp->setText("Configure RSP Plugin...");
    this->action_Options_ConfigRsp->setEnabled(g_MupenApi.Core.HasPluginConfig(M64P::Wrapper::PluginType::Rsp));
    this->action_Options_ConfigControl->setText("Configure Controller Plugin...");
    this->action_Options_ConfigControl->setEnabled(g_MupenApi.Core.HasPluginConfig(M64P::Wrapper::PluginType::Input));
    this->action_Options_Settings->setText("Settings...");
    this->action_Options_Settings->setShortcut(QKeySequence("Ctrl+T"));

    this->action_Help_Support->setText("Support Discord");
    this->action_Help_HomePage->setText("Homepage");
    this->action_Help_About->setText("About");
}

void MainWindow::menuBar_Actions_Connect(void)
{
    connect(this->action_File_OpenRom, &QAction::triggered, this, &MainWindow::on_Action_File_OpenRom);
    connect(this->action_File_OpenCombo, &QAction::triggered, this, &MainWindow::on_Action_File_OpenCombo);
    connect(this->action_File_EndEmulation, &QAction::triggered, this, &MainWindow::on_Action_File_EndEmulation);
    connect(this->action_File_ChooseDirectory, &QAction::triggered, this, &MainWindow::on_Action_File_ChooseDirectory);
    connect(this->action_File_RefreshRomList, &QAction::triggered, this, &MainWindow::on_Action_File_RefreshRomList);
    connect(this->action_File_Exit, &QAction::triggered, this, &MainWindow::on_Action_File_Exit);

    connect(this->action_System_SoftReset, &QAction::triggered, this, &MainWindow::on_Action_System_SoftReset);
    connect(this->action_System_HardReset, &QAction::triggered, this, &MainWindow::on_Action_System_HardReset);
    connect(this->action_System_Pause, &QAction::triggered, this, &MainWindow::on_Action_System_Pause);
    connect(this->action_System_GenerateBitmap, &QAction::triggered, this, &MainWindow::on_Action_System_GenerateBitmap);
    connect(this->action_System_LimitFPS, &QAction::triggered, this, &MainWindow::on_Action_System_LimitFPS);
    connect(this->action_System_SwapDisk, &QAction::triggered, this, &MainWindow::on_Action_System_SwapDisk);
    connect(this->action_System_SaveState, &QAction::triggered, this, &MainWindow::on_Action_System_SaveState);
    connect(this->action_System_SaveAs, &QAction::triggered, this, &MainWindow::on_Action_System_SaveAs);
    connect(this->action_System_LoadState, &QAction::triggered, this, &MainWindow::on_Action_System_LoadState);
    connect(this->action_System_Load, &QAction::triggered, this, &MainWindow::on_Action_System_Load);
    connect(this->action_System_Cheats, &QAction::triggered, this, &MainWindow::on_Action_System_Cheats);
    connect(this->action_System_GSButton, &QAction::triggered, this, &MainWindow::on_Action_System_GSButton);

    connect(this->action_Options_ConfigGfx, &QAction::triggered, this, &MainWindow::on_Action_Options_ConfigGfx);
    connect(this->action_Options_ConfigAudio, &QAction::triggered, this, &MainWindow::on_Action_Options_ConfigAudio);
    connect(this->action_Options_ConfigRsp, &QAction::triggered, this, &MainWindow::on_Action_Options_ConfigRsp);
    connect(this->action_Options_ConfigControl, &QAction::triggered, this, &MainWindow::on_Action_Options_ConfigControl);
    connect(this->action_Options_Settings, &QAction::triggered, this, &MainWindow::on_Action_Options_Settings);

    connect(this->action_Help_Support, &QAction::triggered, this, &MainWindow::on_Action_Help_Support);
    connect(this->action_Help_HomePage, &QAction::triggered, this, &MainWindow::on_Action_Help_HomePage);
    connect(this->action_Help_About, &QAction::triggered, this, &MainWindow::on_Action_Help_About);
}

void MainWindow::on_Action_File_OpenRom(void)
{
    bool isRunning = g_MupenApi.Core.IsEmulationRunning();
    bool isPaused = g_MupenApi.Core.isEmulationPaused();

    if (isRunning && !isPaused)
        this->on_Action_System_Pause();

    QFileDialog dialog;
    int ret;
    QString dir;

    dialog.setFileMode(QFileDialog::FileMode::ExistingFile);
    dialog.setNameFilter("N64 Roms (*.n64 *.z64 *.v64)");
    dialog.setWindowIcon(this->ui_Icon);

    ret = dialog.exec();
    if (!ret)
    {
        if (isRunning && !isPaused)
            this->on_Action_System_Pause();
        return;
    }

    this->emulationThread_Launch(dialog.selectedFiles().first());
}

void MainWindow::on_Action_File_OpenCombo(void)
{
}

void MainWindow::on_Action_File_EndEmulation(void)
{
    if (g_MupenApi.Core.isEmulationPaused())
    {
        this->on_Action_System_Pause();
    }

    if (!g_MupenApi.Core.StopEmulation())
    {
        this->ui_MessageBox("Error", "Api::Core::StopEmulation Failed!", g_MupenApi.Core.GetLastError());
    }
}

void MainWindow::on_Action_File_ChooseDirectory(void)
{
    QFileDialog dialog;
    QString dir;
    int ret;

    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    dialog.setWindowIcon(this->ui_Icon);

    ret = dialog.exec();
    if (ret)
    {
        dir = dialog.selectedFiles().first();
        g_MupenApi.Config.SetOption(SETTINGS_SECTION, SETTINGS_DIRECTORY, dir);
        this->ui_Widget_RomBrowser->SetDirectory(dir);
        this->ui_Widget_RomBrowser->RefreshRomList();
    }
}

void MainWindow::on_Action_File_RefreshRomList(void)
{
    this->ui_Widget_RomBrowser->RefreshRomList();
}

void MainWindow::on_Action_File_Exit(void)
{
    this->close();
}

void MainWindow::on_Action_System_SoftReset(void)
{
    if (!g_MupenApi.Core.ResetEmulation(false))
    {
        this->ui_MessageBox("Error", "Api::Core::ResetEmulation Failed!", g_MupenApi.Core.GetLastError());
    }
}

void MainWindow::on_Action_System_HardReset(void)
{
    if (!g_MupenApi.Core.ResetEmulation(true))
    {
        this->ui_MessageBox("Error", "Api::Core::ResetEmulation Failed!", g_MupenApi.Core.GetLastError());
    }
}

void MainWindow::on_Action_System_Pause(void)
{
    static bool paused = false;

    bool ret;
    QString error = "Api::Core::";

    if (!paused)
    {
        ret = g_MupenApi.Core.PauseEmulation();
        error += "PauseEmulation";
    }
    else
    {
        ret = g_MupenApi.Core.ResumeEmulation();
        error += "ResumeEmulation";
    }

    error += " Failed!";

    paused = !paused;

    if (!ret)
    {
        this->ui_MessageBox("Error", error, g_MupenApi.Core.GetLastError());
        return;
    }

    this->menuBar_Setup(true, paused);
}

void MainWindow::on_Action_System_GenerateBitmap(void)
{
    if (!g_MupenApi.Core.TakeScreenshot())
    {
        this->ui_MessageBox("Error", "Api::Core::TakeScreenshot Failed!", g_MupenApi.Core.GetLastError());
    }
}

#include <iostream>
void MainWindow::on_Action_System_LimitFPS(void)
{
    bool enabled, ret;

    enabled = this->action_System_LimitFPS->isChecked();

    std::cout << "enabled: " << enabled << std::endl;

    if (enabled)
        ret = g_MupenApi.Core.EnableSpeedLimiter();
    else
        ret = g_MupenApi.Core.DisableSpeedLimiter();

    if (!ret)
    {
        this->ui_MessageBox("Error", "aaaa Failed", g_MupenApi.Core.GetLastError());
    }
}

void MainWindow::on_Action_System_SwapDisk(void)
{
}

void MainWindow::on_Action_System_SaveState(void)
{
    if (!g_MupenApi.Core.SaveState())
    {
        this->ui_MessageBox("Error", "Api::Core::SaveState Failed", g_MupenApi.Core.GetLastError());
    }
}

void MainWindow::on_Action_System_SaveAs(void)
{
    bool isPaused = g_MupenApi.Core.isEmulationPaused();

    if (!isPaused)
        this->on_Action_System_Pause();

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save State"), "",
                                                    tr("SaveState (*.dat);;All Files (*)"));

    if (!g_MupenApi.Core.SaveStateAsFile(fileName))
    {
        this->ui_MessageBox("Error", "Api::Core::SaveStateAsFile Failed", g_MupenApi.Core.GetLastError());
    }

    if (!isPaused)
        this->on_Action_System_Pause();
}

void MainWindow::on_Action_System_LoadState(void)
{
    if (!g_MupenApi.Core.LoadState())
    {
        this->ui_MessageBox("Error", "Api::Core::LoadState Failed", g_MupenApi.Core.GetLastError());
    }
}

void MainWindow::on_Action_System_Load(void)
{
    bool isPaused = g_MupenApi.Core.isEmulationPaused();

    if (!isPaused)
        this->on_Action_System_Pause();

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Save State"), "",
                                                    tr("SaveState (*.dat);;All Files (*)"));

    if (!g_MupenApi.Core.LoadStateFromFile(fileName))
    {
        this->ui_MessageBox("Error", "Api::Core::LoadStateFromFile Failed", g_MupenApi.Core.GetLastError());
    }

    if (!isPaused)
        this->on_Action_System_Pause();
}

void MainWindow::on_Action_System_CurrentSaveState(int slot)
{
    if (!g_MupenApi.Core.SetSaveSlot(slot))
    {
        this->ui_MessageBox("Error", "Api::Core::SetSaveSlot Failed", g_MupenApi.Core.GetLastError());
    }
}

void MainWindow::on_Action_System_Cheats(void)
{
}

void MainWindow::on_Action_System_GSButton(void)
{
    if (!g_MupenApi.Core.PressGameSharkButton())
    {
        this->ui_MessageBox("Error", "Api::Core::PressGameSharkButton Failed", g_MupenApi.Core.GetLastError());
    }
}

void MainWindow::on_Action_Options_FullScreen(void)
{
}

void MainWindow::on_Action_Options_ConfigGfx(void)
{
    g_MupenApi.Core.OpenPluginConfig(M64P::Wrapper::PluginType::Gfx);
}

void MainWindow::on_Action_Options_ConfigAudio(void)
{
    g_MupenApi.Core.OpenPluginConfig(M64P::Wrapper::PluginType::Audio);
}

void MainWindow::on_Action_Options_ConfigRsp(void)
{
    g_MupenApi.Core.OpenPluginConfig(M64P::Wrapper::PluginType::Rsp);
}

void MainWindow::on_Action_Options_ConfigControl(void)
{
    g_MupenApi.Core.OpenPluginConfig(M64P::Wrapper::PluginType::Input);
}

void MainWindow::on_Action_Options_Settings(void)
{
    bool isRunning = g_MupenApi.Core.IsEmulationRunning();
    bool isPaused = g_MupenApi.Core.isEmulationPaused();

    if (isRunning && !isPaused)
        this->on_Action_System_Pause();

    Dialog::SettingsDialog dialog(this);
    dialog.exec();

    // reload UI,
    // because we need to keep Options -> Configure {type} Plugin...
    // up-to-date
    this->ui_InEmulation(emulationThread->isRunning(), isPaused);

    if (isRunning && !isPaused)
        this->on_Action_System_Pause();
}

void MainWindow::on_Action_Help_Support(void)
{
    QDesktopServices::openUrl(QUrl(APP_URL_SUPPORT));
}

void MainWindow::on_Action_Help_HomePage(void)
{
    QDesktopServices::openUrl(QUrl(APP_URL_HOMEPAGE));
}

void MainWindow::on_Action_Help_About(void)
{
}

void MainWindow::on_Emulation_Started(void)
{
    this->ui_InEmulation(true, false);
}

void MainWindow::on_Emulation_Finished(bool ret)
{
    if (!ret)
        this->ui_MessageBox("Error", "EmulationThread::run Failed", this->emulationThread->GetLastError());

    this->ui_InEmulation(false, false);
}

void MainWindow::on_RomBrowser_Selected(QString file)
{
    this->emulationThread_Launch(file);
}

void MainWindow::on_VidExt_Init(void)
{
    this->ui_InEmulation(true, false);
}

void MainWindow::on_VidExt_SetupOGL(QSurfaceFormat format, QThread *thread)
{
    this->ui_Widget_OpenGL->SetThread(thread);

    g_OGLWidget = this->ui_Widget_OpenGL;

    // this->ui_Widget_OpenGL->setCursor(Qt::BlankCursor);
    this->ui_Widget_OpenGL->setFormat(format);
}

void MainWindow::on_VidExt_SetMode(int width, int height, int bps, int mode, int flags)
{
    std::cout << "on_VidExt_SetMode" << std::endl;
    this->on_VidExt_ResizeWindow(width, height);
}

void MainWindow::on_VidExt_SetModeWithRate(int width, int height, int refresh, int bps, int mode, int flags)
{
    std::cout << "on_VidExt_SetModeWithRate" << std::endl;
    this->on_VidExt_ResizeWindow(width, height);
}

void MainWindow::on_VidExt_ResizeWindow(int width, int height)
{
    std::cout << "on_VidExt_ResizeWindow(" << width << "," << height << ");" << std::endl;

    if (!this->menuBar->isHidden())
        height += this->menuBar->height();

    if (!this->statusBar()->isHidden())
        height += this->statusBar()->height();

    this->ui_SaveGeometry();

    this->setMinimumSize(QSize(width, height));
    this->setMaximumSize(QSize(width, height));
}

void MainWindow::on_VidExt_SetCaption(QString title)
{
    std::cout << "on_VidExt_SetCaption" << std::endl;
    //this->setWindowTitle(QString(WINDOW_TITLE) + " - " + title);
}

void MainWindow::on_VidExt_ToggleFS(void)
{
    std::cout << "on_VidExt_ToggleFS" << std::endl;
}

void MainWindow::on_VidExt_Quit(void)
{
    std::cout << "on_VidExt_Quit" << std::endl;
    this->ui_InEmulation(false, false);
    this->ui_LoadGeometry();
}