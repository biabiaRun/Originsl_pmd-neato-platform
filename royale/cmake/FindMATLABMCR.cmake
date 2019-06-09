#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

set(Matlab_FOUND 0)

find_package(Matlab COMPONENTS MX_LIBRARY ENG_LIBRARY)

IF (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    RETURN()
ENDIF()

if(Matlab_FOUND)
    RETURN()
endif()

set(_MATLAB_VERSIONS 9.1 9.0.1 9.0 8.5.1 8.5 8.4)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # Regular x86
    set(_REDIRECT_32BIT "WOW6432Node\\")
    set(_BITNESS_SUBFOLDER "win32")
else()
    # x64
    set(_REDIRECT_32BIT "")
    set(_BITNESS_SUBFOLDER "win64")
endif()

foreach(_VERSION ${_MATLAB_VERSIONS})
    get_filename_component(
      current_MATLAB_ROOT
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\${_REDIRECT_32BIT}MathWorks\\MATLAB Runtime\\${_VERSION};MATLABROOT]"
      ABSOLUTE)

    if (NOT(${current_MATLAB_ROOT} STREQUAL "/registry"))
        string(REPLACE "." "" currentVersion ${_VERSION})
        set(currentVersion "v${currentVersion}")
        set(current_MATLAB_ROOT "${current_MATLAB_ROOT}/${currentVersion}")

        if(EXISTS ${current_MATLAB_ROOT})
          list(APPEND _matlab_roots_list ${current_MATLAB_ROOT})
        endif()
     endif()
endforeach()

list(LENGTH _matlab_roots_list numRoots)

if(numRoots EQUAL 0)
    # No Matlab installations found
    RETURN()
endif()

list(GET _matlab_roots_list 0 first_Matlab_ROOT)
set(Matlab_ROOT ${first_Matlab_ROOT} CACHE STRING "Root folder of the Matlab Runtime")
set_property(CACHE Matlab_ROOT PROPERTY STRINGS ${_matlab_roots_list})

find_path(Matlab_INCLUDE_DIRS
  "mex.h" HINTS
  "${Matlab_ROOT}/extern/include"
)
find_library(Matlab_MEX_LIBRARY
  libmex HINTS
  "${Matlab_ROOT}/extern/lib/${_BITNESS_SUBFOLDER}/microsoft"
)
find_library(Matlab_MX_LIBRARY
  libmx HINTS
  "${Matlab_ROOT}/extern/lib/${_BITNESS_SUBFOLDER}/microsoft"
)
find_library(Matlab_ENG_LIBRARY
  libeng HINTS
  "${Matlab_ROOT}/extern/lib/${_BITNESS_SUBFOLDER}/microsoft"
)

set(Matlab_LIBRARIES
  ${Matlab_MEX_LIBRARY}
  ${Matlab_MX_LIBRARY}
  ${Matlab_ENG_LIBRARY}
)

# Change the dll extension to a mex extension.
IF(${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
    IF(ARCHITECTURE_BITNESS STREQUAL 64)
        SET(Matlab_MEX_EXTENSION "mexw64" CACHE STRING "" FORCE)
    ELSE()
        SET(Matlab_MEX_EXTENSION "mexw32" CACHE STRING "" FORCE)
    ENDIF()    
ELSEIF (${ROYALE_TARGET_PLATFORM} STREQUAL APPLE)
    IF(ARCHITECTURE_BITNESS STREQUAL 64)
        SET(Matlab_MEX_EXTENSION "mexmaci64" CACHE STRING "" FORCE)
    ELSE()
        SET(Matlab_MEX_EXTENSION "mexmaci32" CACHE STRING "" FORCE)
    ENDIF()    
ELSE () #LINUX
    IF(ARCHITECTURE_BITNESS STREQUAL 64)
        SET(Matlab_MEX_EXTENSION "mexa64" CACHE STRING "" FORCE)
    ELSE()
        SET(Matlab_MEX_EXTENSION "mexa32" CACHE STRING "" FORCE)
    ENDIF()    
ENDIF()

if(Matlab_INCLUDE_DIRS AND Matlab_LIBRARIES)
  set(Matlab_FOUND 1)
  message(STATUS "Matlab MCR : ${Matlab_ROOT}")
endif()

mark_as_advanced(
  Matlab_LIBRARIES
  Matlab_MEX_LIBRARY
  Matlab_MX_LIBRARY
  Matlab_ENG_LIBRARY
  Matlab_INCLUDE_DIRS
  Matlab_FOUND
  Matlab_ROOT
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Matlab
   FOUND_VAR Matlab_FOUND
   REQUIRED_VARS Matlab_LIBRARIES Matlab_INCLUDE_DIRS
   VERSION_VAR currentVersion
   HANDLE_COMPONENTS
   )
