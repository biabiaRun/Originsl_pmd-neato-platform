SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
SET(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
add_subdirectory(gtest)
SET_TARGET_PROPERTIES(gtest
    PROPERTIES
    FOLDER contrib
    )
SET_TARGET_PROPERTIES(gtest_main
    PROPERTIES
    FOLDER contrib
    )
SET_TARGET_PROPERTIES(gmock
    PROPERTIES
    FOLDER contrib
    )
SET_TARGET_PROPERTIES(gmock_main
    PROPERTIES
    FOLDER contrib
    )

if (ROYALE_ENABLE_BDD)
    add_subdirectory(catch)
endif()

add_subdirectory(libusb)
add_subdirectory(numpy)

IF (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
ELSE()
    IF(NOT ROYALE_USE_SYSTEM_GLUT_LIBRARY)
      SET(FREEGLUT_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
      SET(FREEGLUT_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
      SET(FREEGLUT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
      SET(FREEGLUT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/freeglut-3.0.0-rc2/include" CACHE STRING "" FORCE)
      OPTION(INSTALL_PDB "Also install .pdb files" OFF)
      add_subdirectory(freeglut-3.0.0-rc2)
      SET_TARGET_PROPERTIES(freeglut_static
          PROPERTIES
          FOLDER contrib
        )
    ENDIF()
ENDIF()