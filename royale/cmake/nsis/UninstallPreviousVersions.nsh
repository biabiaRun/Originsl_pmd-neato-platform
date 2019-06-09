!macro UninstallPreviousVersions
    Var /GLOBAL index
    Var /GLOBAL packageName
    Var /GLOBAL regKey
    Var /GLOBAL regInstLocation
    Var /GLOBAL regUninstallStr
    Var /GLOBAL tempDir
    
    StrLen $packageName "${CPACK_PACKAGE_TOP_FOLDER}"
    StrCpy $index 0
    
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
    MessageBox MB_YESNO \
    "Old Royale Version $regInstLocation found. Would you like to uninstall it before installing a newer version?" \
    IDYES uninst IDNO loop
    
  uninst:
    GetTempFileName $tempDir
    CopyFiles /SILENT $regUninstallStr  $tempDir
    ExecWait '$tempDir _?=$regInstLocation'
    StrCpy $index 0
    Goto loop
  done:
!macroend
