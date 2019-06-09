; Copyright (C) 2017 pmdtechnologies ag
;
; THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
; KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
; PARTICULAR PURPOSE.
; 

!ifndef DriverInstallation_INCLUDED
!define DriverInstallation_INCLUDED

!include x64.nsh
!include WinVer.nsh

!macro DriverInstallation
    UserInfo::GetAccountType
    pop $0
    ${If} $0 == "admin"
        CopyFiles "$INSTDIR\driver\*.*" "$PLUGINSDIR"
        
        ${If} ${IsWin10}
            ${If} ${RunningX64}
                ExecWait '"$INSTDIR\driver\dpinst64.exe" /c /sa /sw /PATH $PLUGINSDIR\win10\x64'
            ${Else}
                ExecWait '"$INSTDIR\driver\dpinst32.exe" /c /sa /sw /PATH $PLUGINSDIR\win10\x86'
            ${EndIf}
        ${ElseIf} ${IsWin8}
        ${OrIf} ${IsWin8.1}
            ${If} ${RunningX64}
                ExecWait '"$INSTDIR\driver\dpinst64.exe" /c /sa /sw /PATH $PLUGINSDIR\win8\x64'
            ${Else}
                ExecWait '"$INSTDIR\driver\dpinst32.exe" /c /sa /sw /PATH $PLUGINSDIR\win8\x86'
            ${EndIf}
        ${ElseIf} ${IsWin7}
            ${If} ${RunningX64}
                ExecWait '"$INSTDIR\driver\dpinst64.exe" /c /sa /sw /PATH $PLUGINSDIR\win7\x64'
            ${Else}
                ExecWait '"$INSTDIR\driver\dpinst32.exe" /c /sa /sw /PATH $PLUGINSDIR\win7\x86'
            ${EndIf}
        ${Else}
            MessageBox mb_iconstop "Automatic driver installation Failed. This windows version is unknown to the installer!"
        ${EndIf}
        
    ${ELSE}
        MessageBox mb_iconstop "Driver installation Failed. Administrator rights required!"
        SetErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
    ${EndIf}
!macroend

!endif # !DriverInstallation_INCLUDED