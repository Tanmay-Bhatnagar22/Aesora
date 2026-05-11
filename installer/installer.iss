[Setup]
; Script generated for Aesora - Secure File Encryption Tool
; Created with Inno Setup 6.0 or later

AppId={{1A2B3C4D-5E6F-7G8H-9I0J-K1L2M3N4O5P6}
AppName=Aesora
AppVersion=1.0.0
AppPublisher=Aesora Contributors
AppPublisherURL=https://github.com/Tanmay-Bhatnagar22/Aesora
AppSupportURL=https://github.com/Tanmay-Bhatnagar22/aesora/issues
AppUpdatesURL=https://github.com/Tanmay-Bhatnagar22/aesora/releases
DefaultDirName={autopf}\Aesora
DefaultGroupName=Aesora
AllowNoIcons=yes
LicenseFile=LICENSE
OutputDir=installer_output
OutputBaseFilename=Aesora-1.0.0-Setup
Compression=lzma
SolidCompression=yes
PrivilegesRequired=lowest
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible

; Installer appearance
WizardStyle=modern
DisableWelcomePage=no
ShowLanguageDialog=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.1,10.0
Name: "addpath"; Description: "Add Aesora to PATH (allows 'aesora' command in terminal)"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Main executable (adjust path based on your build output location)
Source: "build\Release\aesora.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
; If you build in Debug mode, use: "build\Debug\aesora.exe" instead

; Documentation
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE"; DestDir: "{app}"; Flags: ignoreversion

; Optional: Include OpenSSL DLLs if not static linked
; Uncomment the following lines if you link against OpenSSL dynamically:
; Source: "C:\OpenSSL-Win64\bin\libssl-3-x64.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Source: "C:\OpenSSL-Win64\bin\libcrypto-3-x64.dll"; DestDir: "{app}\bin"; Flags: ignoreversion

[Icons]
Name: "{group}\Aesora"; Filename: "{cmd}"; Parameters: "/k title Aesora && ""{app}\bin\aesora.exe"" --help & pause"; IconIndex: 0; Comment: "Aesora - File Encryption Tool"
Name: "{autodesktop}\Aesora"; Filename: "{cmd}"; Parameters: "/k title Aesora && ""{app}\bin\aesora.exe"" --help & pause"; Tasks: desktopicon; IconIndex: 0; Comment: "Aesora - File Encryption Tool"
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Aesora"; Filename: "{cmd}"; Parameters: "/k title Aesora && ""{app}\bin\aesora.exe"" --help & pause"; Tasks: quicklaunchicon; IconIndex: 0; Comment: "Aesora - File Encryption Tool"

; Documentation shortcuts
Name: "{group}\README"; Filename: "{app}\README.md"; Flags: ignoreversion
Name: "{group}\LICENSE"; Filename: "{app}\LICENSE"; Flags: ignoreversion

; Uninstall shortcut
Name: "{group}\Uninstall Aesora"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\README.md"; Description: "View README"; Flags: shellexec postinstall skipifsilent

[Registry]
; Add to Windows registry for uninstall information
Root: HKCU; Subkey: "Environment"; ValueType: expandsz; ValueName: "PATH"; ValueData: "{olddata};{app}\bin"; Tasks: addpath; Check: NeedsAddPath(ExpandConstant('{app}\bin'))

[UninstallDelete]
Type: dirifempty; Name: "{app}\bin"
Type: dirifempty; Name: "{app}"

[Code]
function NeedsAddPath(Param: string): boolean;
var
  OrigPath: string;
begin
  if not RegQueryStringValue(HKEY_CURRENT_USER, 'Environment', 'PATH', OrigPath)
  then begin
    Result := True;
    exit;
  end;
  { look for the path with leading and trailing semicolon }
  { Pos() returns 0 if not found }
  Result := Pos(';' + Param + ';', ';' + OrigPath + ';') = 0;
  if not Result then
    Result := Pos(';' + Param + '\;', ';' + OrigPath + ';') = 0;
end;

// Notify Windows of environment change
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    if IsTaskSelected('addpath') then
    begin
      Shell('set PATH=%PATH%', ExpandConstant('{app}\bin'));
    end;
  end;
end;
