if (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)

#TEST_DOT_NET_WRAPPER_SRC_DIR is used in RoyaleDotNet.csproj.in project template
string(REGEX REPLACE "/" "\\\\" TEST_DOT_NET_WRAPPER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}) 

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
"test_RoyaleDotNet.csproj.in"
"test_RoyaleDotNet.csproj"
)

configure_file(
"Properties/AssemblyInfo.cs.in"
"Properties/AssemblyInfo.cs"
)

configure_file(
"AccessCodes.cs.in"
"AccessCodes.cs"
)

include_external_msproject(test_RoyaleDotNet 
   ${CMAKE_CURRENT_BINARY_DIR}/test_RoyaleDotNet.csproj
   TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC
   PLATFORM ${MACHINE})
   
set_target_properties(test_RoyaleDotNet
   PROPERTIES
   OUTPUT_NAME test_RoyaleDotNet
   FOLDER tests/royale
   VERSION ${ROYALE_VERSION_MAJOR}.${ROYALE_VERSION_MINOR}.${ROYALE_VERSION_PATCH}.${ROYALE_VERSION_BUILD}
   )
   
endif()
