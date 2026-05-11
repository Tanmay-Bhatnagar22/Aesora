[Setup]
; Aesora Installer Script

AppId={{1A2B3C4D-5E6F-7G8H-9I0J-K1L2M3N4O5P6}
AppName=Aesora
AppVersion=1.0.0
AppPublisher=Tanmay Bhatnagar
AppPublisherURL=https://github.com/Tanmay-Bhatnagar22/Aesora
AppSupportURL=https://github.com/Tanmay-Bhatnagar22/Aesora/issues
AppUpdatesURL=https://github.com/Tanmay-Bhatnagar22/Aesora/releases

DefaultDirName={autopf}\Aesora
DefaultGroupName=Aesora
AllowNoIcons=yes

LicenseFile=C:\Users\HP\.vscode\Tanmay-c-cpp\Aesora\LICENSE

OutputDir=installer_output
OutputBaseFilename=Aesora-1.0.0-Setup

Compression=lzma
SolidCompression=yes

PrivilegesRequired=lowest

ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible

WizardStyle=modern

; Installer icon
SetupIconFile=C:\Users\HP\.vscode\Tanmay-c-cpp\Aesora\assests\Aesora.ico
UninstallDisplayIcon={app}\Aesora.exe

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]

; Main executable
Source: "C:\Users\HP\.vscode\Tanmay-c-cpp\Aesora\build\Aesora.exe"; DestDir: "{app}"; Flags: ignoreversion

; Documentation
Source: "C:\Users\HP\.vscode\Tanmay-c-cpp\Aesora\README.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\HP\.vscode\Tanmay-c-cpp\Aesora\LICENSE"; DestDir: "{app}"; Flags: ignoreversion

; Application icon
Source: "C:\Users\HP\.vscode\Tanmay-c-cpp\Aesora\assests\Aesora.ico"; DestDir: "{app}"; Flags: ignoreversion

[Icons]

; Start Menu shortcut
Name: "{group}\Aesora"; Filename: "{app}\Aesora.exe"; IconFilename: "{app}\aesora.ico"

; Desktop shortcut
Name: "{autodesktop}\Aesora"; Filename: "{app}\Aesora.exe"; Tasks: desktopicon; IconFilename: "{app}\aesora.ico"

; README shortcut
Name: "{group}\README"; Filename: "{app}\README.md"

; Uninstall shortcut
Name: "{group}\Uninstall Aesora"; Filename: "{uninstallexe}"

[Run]

; Launch app after install
Filename: "{app}\Aesora.exe"; Description: "Launch Aesora"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: "{app}\aesora.ico"