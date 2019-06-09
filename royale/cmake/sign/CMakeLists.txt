#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

if(${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
    list (APPEND SIGNPATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot]/bin/x86") # Win 8.0 SDK
    list (APPEND SIGNPATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot]/bin/x86") # Win 8.0 SDK 64 Bit System
    list (APPEND SIGNPATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot81]/bin/x86") # Win 8.1 SDK
    list (APPEND SIGNPATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot81]/bin/x86") # Win 8.1 SDK 64 Bit System
    list (APPEND SIGNPATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]/Tools/bin/i386") # Win 10 SDK
    list (APPEND SIGNPATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]/Tools/bin/i386") # Win 10 SDK 64 Bit System
    find_program(SIGNTOOL_EXECUTABLE signtool
        PATHS ${SIGNPATHS}
    )

    list (APPEND INFCATPATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;WdkBinRootVersioned]/x86")# Win 10 WDK
    list (APPEND INFCATPATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows Kits\\Installed Roots;WdkBinRootVersioned]/x86")# Win 10 WDK 64 Bit System
    find_program(INF2CAT_EXECUTABLE inf2cat
        PATHS ${INFCATPATHS}
    )

    list (APPEND INFVERIFPATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]/Tools/x86")# Win 10 WDK
    list (APPEND INFVERIFPATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]/Tools/x86")# Win 10 WDK 64 Bit System
    find_program(INFVERIF_EXECUTABLE infverif
        PATHS ${INFVERIFPATHS}
    )

    set(royaledir "${ROYALE_SOURCE_DIR}")
    string(REPLACE "/" "\\" royaledir ${royaledir})

    if(SIGNTOOL_EXECUTABLE)
        message(STATUS "Found signtool at \"${SIGNTOOL_EXECUTABLE}\", generate sign scripts for installers and drivers.")
        set(signtoolpath "\"${SIGNTOOL_EXECUTABLE}\"")
        string(REPLACE "/" "\\" signtoolpath ${signtoolpath})
        set(signtoolfile "${CPACK_OUTPUT_FILE_PREFIX}/${CPACK_PACKAGE_FILE_NAME}.exe")
        string(REPLACE "/" "\\" signtoolfile ${signtoolfile})

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/SignDriver.bat.template.nt" "${ROYALE_BINARY_DIR}/SignDriver.bat")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/signsetup.bat.template" "${ROYALE_BINARY_DIR}/SignSetup.bat")
    else()
        message(STATUS "signtool not found, can not generate sign scripts for installers and drivers.")
    endif()

    if(INF2CAT_EXECUTABLE)
        message(STATUS "Found inf2cat at \"${INF2CAT_EXECUTABLE}\", generate script CreateCatFiles.bat to generate cat files.")
    endif()
    if(INFVERIF_EXECUTABLE)
        message(STATUS "Found infverif at \"${INFVERIF_EXECUTABLE}\", generate script CreateCatFiles.bat to generate cat files.")
    endif()
    if(INF2CAT_EXECUTABLE AND INFVERIF_EXECUTABLE)
        set(INF2CATPATH "\"${INF2CAT_EXECUTABLE}\"")
        string(REPLACE "/" "\\" INF2CATPATH ${INF2CATPATH})
        set(INFVERIFPATH "\"${INFVERIF_EXECUTABLE}\"")
        string(REPLACE "/" "\\" INFVERIFPATH ${INFVERIFPATH})
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CreateCatFiles.bat.template.nt" "${ROYALE_BINARY_DIR}/CreateCatFiles.bat")
    else()
        message(STATUS "inf2cat or infverif not found, can not generate script to create cat files.")
    endif()
endif()
