cmake_minimum_required(VERSION 3.1)

set(CMAKE_PREFIX_PATH "../../../share")

find_package(royale REQUIRED)

link_directories(${royale_LIB_DIR})

project (sampleCAPI)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../inc
)

add_executable(sampleCAPI
   sampleCAPI.c
   )
set_target_properties( sampleCAPI
	PROPERTIES
	C_STANDARD 11
	C_STANDARD_REQUIRED true
)

if (WIN32)
    add_custom_command(
        TARGET sampleCAPI
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${royale_INSTALL_PATH}/bin/royale.dll"  $<TARGET_FILE_DIR:sampleCAPI>)

    add_custom_command(
        TARGET sampleCAPI
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${royale_INSTALL_PATH}/bin/spectre.dll"  $<TARGET_FILE_DIR:sampleCAPI>)
        
    add_custom_command(
        TARGET sampleCAPI
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${royale_INSTALL_PATH}/bin/royaleCAPI.dll"  $<TARGET_FILE_DIR:sampleCAPI>)
endif (WIN32)

if (APPLE)
    add_custom_command(
        TARGET sampleCAPI
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${royale_INSTALL_PATH}/bin/libroyale.dylib" $<TARGET_FILE_DIR:sampleCAPI>)

    add_custom_command(
        TARGET sampleCAPI
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${royale_INSTALL_PATH}/bin/libroyale.${royale_VERSION}.dylib" $<TARGET_FILE_DIR:sampleCAPI>)

    add_custom_command(
        TARGET sampleCAPI
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${royale_INSTALL_PATH}/bin/libspectre.dylib" $<TARGET_FILE_DIR:sampleCAPI>)
        
    add_custom_command(
        TARGET sampleCAPI
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${royale_INSTALL_PATH}/bin/libroyaleCAPI.dylib" $<TARGET_FILE_DIR:sampleCAPI>)

    add_custom_command(
        TARGET sampleCAPI
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${royale_INSTALL_PATH}/bin/libroyaleCAPI.${royale_VERSION}.dylib" $<TARGET_FILE_DIR:sampleCAPI>)
endif (APPLE)

target_link_libraries(sampleCAPI royaleCAPI)
