#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************


MESSAGE("Windows installer settings")

SET(CPACK_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/nsis")

SET(CPACK_RESOURCE_FILE_LICENSE "${ROYALE_LICENSE_PATH}")

SET(CPACK_GENERATOR "NSIS")
SET(CPACK_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}\\\\nsis\\\\rc")
SET(CPACK_PACKAGE_TOP_FOLDER "${ROYALE_NAME}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_TOP_FOLDER}\\\\${ROYALE_VERSION}")
SET(CPACK_MONOLITHIC_INSTALL OFF)
SET(CPACK_NSIS_MODIFY_PATH ON)
SET(CPACK_PACKAGE_ICON "${CPACK_BASE_DIR}\\\\package_icon.png")
SET(CPACK_PACKAGE_EXECUTABLES "royaleviewer" "royaleviewer")
SET(CPACK_NSIS_MUI_ICON "${CPACK_BASE_DIR}\\\\installer.ico")
SET(CPACK_NSIS_MUI_UNIICON "${CPACK_BASE_DIR}\\\\uninstaller.ico")
SET(CPACK_NSIS_EXECUTABLES_DIRECTORY "${INSTALL_FOLDER_RUNTIME_BASE}")
SET(CPACK_NSIS_INSTALLED_ICON_NAME "royaleviewer.exe")
SET(CPACK_NSIS_URL_INFO_ABOUT "http://www.pmdtec.com")
SET(CPACK_NSIS_HELP_LINK "http://www.pmdtec.com")
SET(CPACK_NSIS_CONTACT "info@pmdtec.com")
SET(CPACK_NSIS_INSTALLER_ARCHITECTURE_BITNESS 0)
IF(${ARCHITECTURE_BITNESS} STREQUAL 64)
    SET(CPACK_INSTALLER_ARCHITECTURE_BITNESS "64")
    SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    SET(CPACK_NSIS_PACKAGE_NAME "${ROYALE_VERSION_NAME} (Win64)")
ELSEIF(${ARCHITECTURE_BITNESS} STREQUAL 32)
    SET(CPACK_INSTALLER_ARCHITECTURE_BITNESS "32")
    SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
    SET(CPACK_NSIS_PACKAGE_NAME "${ROYALE_VERSION_NAME}")
ENDIF(${ARCHITECTURE_BITNESS} STREQUAL 64)
SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_NSIS_PACKAGE_NAME}")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_NSIS_PACKAGE_NAME}")
SET(CPACK_NSIS_DISPLAY_VERSION "${ROYALE_VERSION}")

set(CPACK_NSIS_CREATE_ICONS "
    SetOutPath \\\"$INSTDIR\\\\bin\\\"
    CreateDirectory \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\"
    CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\royaleviewer-${ROYALE_VERSION}.lnk\\\" \\\"$INSTDIR\\\\bin\\\\royaleviewer.exe\\\"
    CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\DoxygenHelp.lnk\\\" \\\"$INSTDIR\\\\doc\\\\html\\\\index.html\\\"
    CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\License.lnk\\\" \\\"$INSTDIR\\\\royale_license.txt\\\"
    CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Getting_Started_CamBoard_pico_flexx.lnk\\\" \\\"$INSTDIR\\\\Getting_Started_CamBoard_pico_flexx.pdf\\\"
    CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Getting_Started_CamBoard_pico_maxx.lnk\\\" \\\"$INSTDIR\\\\Getting_Started_CamBoard_pico_maxx.pdf\\\"
    CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Getting_Started_CamBoard_pico_monstar.lnk\\\" \\\"$INSTDIR\\\\Getting_Started_CamBoard_pico_monstar.pdf\\\"
    CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\RoyaleViewerDoc.lnk\\\" \\\"$INSTDIR\\\\RoyaleViewer.pdf\\\"
    CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\README.lnk\\\" \\\"$INSTDIR\\\\README.pdf\\\"
    CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Uninstall.lnk\\\" \\\"$INSTDIR\\\\Uninstall.exe\\\"
")
set(CPACK_NSIS_DELETE_ICONS "
    Delete \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\royaleviewer-${ROYALE_VERSION}.lnk\\\"
    Delete \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\DoxygenHelp.lnk\\\"
    Delete \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\License.lnk\\\"
    Delete \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Getting_Started_CamBoard_pico_flexx.lnk\\\"
    Delete \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Getting_Started_CamBoard_pico_maxx.lnk\\\"
    Delete \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Getting_Started_CamBoard_pico_monstar.lnk\\\"
    Delete \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\RoyaleViewerDoc.lnk\\\"
    Delete \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\README.lnk\\\"
    Delete \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Uninstall.lnk\\\"
    rmDir \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\"
")