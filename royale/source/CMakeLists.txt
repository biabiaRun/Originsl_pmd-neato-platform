# Please be aware of the ordering of these folders.
# For the Matlab and Python wrapper paths are set that
# are used later on for the install step of the royale
# target.
# Due to a limitation of CMake we can only trigger the 
# installation of targets from the CMake file in the
# target folder. This means all the Royale library
# installations have to happen from the CMake file in the
# royale subdirectory.

add_subdirectory (matlab)
add_subdirectory (python)
add_subdirectory (core)
add_subdirectory (components)
add_subdirectory (royale)
add_subdirectory (royaleCAPI)
add_subdirectory (RoyaleDotNet)

if(NOT ${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    add_custom_target(hardware_test
        COMMAND $<TARGET_FILE:test_royale>
        COMMAND $<TARGET_FILE:test_royaleCAPI>
        COMMAND $<TARGET_FILE:test_royaleCAPI_v220>
        DEPENDS test_royale
        DEPENDS test_royaleCAPI
        DEPENDS test_royaleCAPI_v220
        )

    set_target_properties(hardware_test
       PROPERTIES
       FOLDER tests/royale
       )    

    if (TARGET hardware_test_roypy)
        add_dependencies (hardware_test
            hardware_test_roypy
            )
    endif()
endif()
