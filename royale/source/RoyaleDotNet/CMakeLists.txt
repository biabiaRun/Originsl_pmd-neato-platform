if (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)

#DOT_NET_WRAPPER_SRC_DIR is used in RoyaleDotNet.csproj.in project template
string(REGEX REPLACE "/" "\\\\" DOT_NET_WRAPPER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}) 

#DOT_NET_WRAPPER_VERSION_NUMBER is used in AssemblyInfo.cs.in template
set(DOT_NET_WRAPPER_VERSION_NUMBER "${ROYALE_VERSION_MAJOR}.${ROYALE_VERSION_MINOR}.${ROYALE_VERSION_PATCH}.${ROYALE_VERSION_BUILD}")

IF(${ARCHITECTURE_BITNESS} STREQUAL 32)
    SET(MACHINE x86)
    SET(DOTNETPLATFORM Win32)
ELSE(${ARCHITECTURE_BITNESS} STREQUAL 32)
    SET(MACHINE x64)
    SET(DOTNETPLATFORM x64)
ENDIF(${ARCHITECTURE_BITNESS} STREQUAL 32)

configure_file(
"RoyaleDotNet.csproj.in"
"RoyaleDotNet.csproj"
)

configure_file(
"Properties/AssemblyInfo.cs.in"
"Properties/AssemblyInfo.cs"
)

include_external_msproject(RoyaleDotNet 
   ${CMAKE_CURRENT_BINARY_DIR}/RoyaleDotNet.csproj
   TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC
   PLATFORM ${MACHINE}
   royaleCAPI)
   
set_target_properties(RoyaleDotNet
   PROPERTIES
   OUTPUT_NAME RoyaleDotNet
   FOLDER modules
   VERSION ${ROYALE_VERSION_MAJOR}.${ROYALE_VERSION_MINOR}.${ROYALE_VERSION_PATCH}.${ROYALE_VERSION_BUILD}
   )
   
install(FILES   ${CMAKE_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/RoyaleDotNet.dll
                ${CMAKE_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/RoyaleDotNet.XML
        DESTINATION ${ROYALE_INSTALL_DOT_NET_DIR}
        )

install(FILES ${CMAKE_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/RoyaleDotNet.dll
        DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetRetrieveData"
        )

install(FILES ${CMAKE_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/RoyaleDotNet.dll
        DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetCamInfo"
        )
        
install(FILES ${CMAKE_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/RoyaleDotNet.dll
        DESTINATION "${ROYALE_INSTALL_BIN_DIR}"
        )
        
add_subdirectory(test_RoyaleDotNet)
endif()
