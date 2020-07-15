!macro UninstallPreviousVersions
    Var /GLOBAL index
    Var /GLOBAL packageName
    Var /GLOBAL regKey
    Var /GLOBAL regInstLocation
    Var /GLOBAL regUninstallStr
    Var /GLOBAL tempDir
    Var /GLOBAL msgboxCount
    
    StrLen $packageName "${CPACK_PACKAGE_TOP_FOLDER}"
    StrCpy $index 0
    StrCpy $msgboxCount 0
    
  loop:
    EnumRegKey $regKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall" $index
    StrCmp $regKey "" done
    IntOp $index $index + 1
    StrCpy $regUninstallStr $regKey $packageName
    StrCmp $regUninstallStr "${CPACK_PACKAGE_TOP_FOLDER}" 0 loop
    
    ReadRegStr $regUninstallStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\$regKey" "UninstallString"
    ReadRegStr $regInstLocation SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\$regKey" "InstallLocation"
    StrCmp $regUninstallStr  "" loop
    
    ReadRegStr $1 SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SW_APPLICATION_NAME}" "UninstallString"
    StrCmp $regUninstallStr $1 done 0

    StrCmp $msgboxCount 0 msgBox uninst

  msgBox:
    StrCpy $msgboxCount 1
    MessageBox MB_YESNO \
    "Would you like to uninstall all older versions before this installation?" \
    IDYES uninst IDNO done
    
  uninst:
    GetTempFileName $tempDir
    CopyFiles /SILENT $regUninstallStr  $tempDir
    ExecWait '$tempDir /S _?=$regInstLocation'
    StrCpy $index 0
    Goto loop
  done:
!macroend
