; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Rosalie's Mupen GUI"
#define MyAppPublisher "Rosalie"
#define MyAppURL "https://github.com/Rosalie241/RMG"
#define MyAppExeName "RMG.exe"

; REQUIREMENTS:
; Specify MyAppDir, which should point to the binaries
; Specify MySrcDir, which should point to the source code
; Specify MyOutDir, which should point to the output directory
; Specify MyAppVer, which should contain the version
;
; EXAMPLE USAGE:
; "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" ".\Source\Installer\RMG.iss" /DMyAppDir=.\Bin\ /DMyAppVer="1.0" /DMyOutDir=.\Bin\ /DMySrcDir=.\


[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{AD31B3C7-8374-43D0-9C6C-81A01BE4822B}
AppName={#MyAppName}
AppVersion={#MyAppVer}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
LicenseFile={#MySrcDir}\LICENSE
; Remove the following line to run in administrative install mode (install for all users.)
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=commandline
OutputDir={#MyOutDir}
OutputBaseFilename=Setup RMG {#MyAppVer}
SetupIconFile={#MySrcDir}\Source\RMG\UserInterface\Resource\RMG.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#MyAppDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

