SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
if (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
  # workaround for https://github.com/google/googletest/issues/1111
  add_definitions ("-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
endif()
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

IF (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
ELSE()
    IF(NOT ROYALE_USE_SYSTEM_GLUT_LIBRARY)
      SET(FREEGLUT_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
      SET(FREEGLUT_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
      SET(FREEGLUT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
      SET(FREEGLUT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/freeglut-3.0.0-rc2/include" CACHE STRING "" FORCE)
      SET(INSTALL_PDB OFF)
      add_subdirectory(freeglut-3.0.0-rc2)
      SET_TARGET_PROPERTIES(freeglut_static
          PROPERTIES
          FOLDER contrib
        )
    ENDIF()
ENDIF()

if (NOT ${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
  if (ROYALE_ENABLE_RELEASEANALYZER)
    add_subdirectory(libziputils)
    SET_TARGET_PROPERTIES(zipUtils
    	PROPERTIES
    	FOLDER contrib/zip
    	)

    SET(ROYALE_LIBZIPUTILS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libziputils" CACHE STRING "" FORCE)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libziputils/libarchive)
  endif()
endif()
