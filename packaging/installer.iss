#define Version "0.0.4"
#define ProjectName GetEnv('PROJECT_NAME')
#define ProductName GetEnv('PRODUCT_NAME')
#define Publisher GetEnv('COMPANY_NAME')
#define Year GetDateTimeString("yyyy","","")

[Setup]
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
AppName={#ProductName}
OutputBaseFilename={#ProductName}-{#Version}-Windows
AppCopyright=Copyright (C) {#Year} {#Publisher}
AppPublisher={#Publisher}
AppVersion={#Version}
DefaultDirName="{commonpf64}\{#Publisher}\{#ProductName}"
DisableDirPage=no

; MAKE SURE YOU READ/MODIFY THE EULA BEFORE USING IT
LicenseFile="resources\EULA"
UninstallFilesDir="{commonappdata}\{#ProductName}\uninstall"

[Files]
Source: "..\Builds\{#ProjectName}_artefacts\Release\{#ProductName}.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Builds\Release\SDL3.dll"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#ProductName}"; Filename: "{app}\{#ProductName}.exe"
Name: "{autoprograms}\Uninstall {#ProductName}"; Filename: "{uninstallexe}"
