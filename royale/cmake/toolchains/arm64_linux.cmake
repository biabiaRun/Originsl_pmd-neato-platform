#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

# end configuration if cross compiling or in try_compile
# otherwise this will trigger the "No ARM64_COMPILER_TOOLS", because
# no variables are defined in this case.
if( DEFINED CMAKE_CROSSCOMPILING )
 # subsequent toolchain loading is not needed
 return()
endif()
get_property( _CMAKE_IN_TRY_COMPILE GLOBAL PROPERTY IN_TRY_COMPILE )
if( _CMAKE_IN_TRY_COMPILE )
 return()
endif()

# Check if necessary variables are defined
if(NOT DEFINED ARM64_COMPILER_TOOLS)
  if(DEFINED ENV{ARM64_COMPILER_TOOLS})
    set(ARM64_COMPILER_TOOLS $ENV{ARM64_COMPILER_TOOLS})
  else()
    message(FATAL_ERROR "There is no path to the compiler configured. Please
     define the environment variable ARM64_COMPILER_TOOLS or the cmake variable
     ARM64_COMPILER_TOOLS to configure the compiler path.")
  endif()
endif()
if(NOT DEFINED ARM64_ENVIRONMENT_PATH)
  if(DEFINED ENV{ARM64_ENVIRONMENT_PATH})
    set(ARM64_ENVIRONMENT_PATH $ENV{ARM64_ENVIRONMENT_PATH})
  else()
    message(FATAL_ERROR "ARM64_ENVIRONMENT_PATH not configured
     There is no path to compilation environment
     (CMAKE_FIND_ROOT_PATH) configured. Please define the environment variable
     ARM64_ENVIRONMENT_PATH or the cmake variable ARM64_ENVIRONMENT_PATH to
     configure the environment path.")
  endif()
endif()

if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL x86_64)
  set(COMPILER_DIR
    ${ARM64_COMPILER_TOOLS}/bin)
else(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL x86_64)
  message (ERROR "${CMAKE_HOST_SYSTEM_PROCESSOR} is not supported as host platform for Linux ARM 64")
endif(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL x86_64)

message(STATUS "Arm compiler directory is set to: ${COMPILER_DIR}")

# set the necessary parameters for cross compiling
SET(CMAKE_SYSTEM_NAME Linux)

# specify the processor type
# if you adapt this file for an other arm platform, set this variable to the
# output of 'uname -p on the target machine
SET(CMAKE_SYSTEM_PROCESSOR aarch64)

# specify the cross compiler
SET(CMAKE_C_COMPILER   ${COMPILER_DIR}/aarch64-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER ${COMPILER_DIR}/aarch64-linux-gnu-c++)

# if you want to set additional compiler flags for your platform,
# uncomment one or both of the following variable definitions and
# and set them as you need.
#SET(CMAKE_C_FLAGS "")
#SET(CMAKE_CXX_FLAGS "")

# where is the target environment.
# if you adapt this file for an other arm platform and you cross compilation
# environment contains a complete SYSROOT directory for this platform it is
# probably better to define CMAKE_SYSROOT instead of CMAKE_FIND_ROOT_PATH.
SET(CMAKE_FIND_ROOT_PATH  ${ARM64_ENVIRONMENT_PATH})
#SET(CMAKE_SYSROOT  ${ARM64_ENVIRONMENT_PATH})

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# additional library directory as the cross compilers search path is not 100%
# if you adapt this file for  an other arm platform, first try to disable the
# next setting completly. Only use them if really necessary.
SET(CMAKE_LIBRARY_PATH
  "${ARM64_ENVIRONMENT_PATH}/lib/aarch64-linux-gnu"
  "${ARM64_ENVIRONMENT_PATH}/usr/lib/aarch64-linux-gnu"
)
link_directories(
  "${ARM64_ENVIRONMENT_PATH}/lib/aarch64-linux-gnu"
  "${ARM64_ENVIRONMENT_PATH}/usr/lib/aarch64-linux-gnu"
)

# disable royale parts that can or should not be build on raspberry
OPTION(ROYALE_ENABLE_TOOLS "Build all internal tools" OFF)
OPTION(ROYALE_ENABLE_PANDOC
  "Use Pandoc for md documents (Pandoc has to be installed and in the path)"
  OFF
)
