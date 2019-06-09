#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

if(ARCHITECTURE STREQUAL "arm")
  IF(${ARCHITECTURE_BITNESS} STREQUAL 64)
  set(ARCH_C_FLAGS "-ftree-vectorize -ffast-math -fno-short-enums")
  set(ARCH_CXX_FLAGS "-ftree-vectorize -ffast-math -fno-short-enums")
  ELSEIF(${ARCHITECTURE_BITNESS} STREQUAL 32)
    set(ARCH_C_FLAGS "-ftree-vectorize -ffast-math -fno-short-enums -mfpu=neon-vfpv4")
    set(ARCH_CXX_FLAGS "-ftree-vectorize -ffast-math -fno-short-enums -mfpu=neon-vfpv4")
  ENDIF(${ARCHITECTURE_BITNESS} STREQUAL 64)
else()
  set(ARCH_C_FLAGS "")
  set(ARCH_CXX_FLAGS "")
endif()

#----------------------------      GLOBAL C-flags         ----------------------------
SET(CMAKE_C_FLAGS                                   "${CMAKE_C_FLAGS} -fPIC -Wall -Wconversion -D_FILE_OFFSET_BITS=64 ${ARCH_C_FLAGS}")
SET(CMAKE_C_FLAGS_DEBUG                             "${CMAKE_C_FLAGS_DEBUG} -ggdb")
#SET(CMAKE_C_FLAGS_RELEASE                           "")
#SET(CMAKE_C_FLAGS_MINSIZEREL                        "")
#SET(CMAKE_C_FLAGS_RELWITHDEBINFO                    "")
#----------------------------      GLOBAL CXX-flags       ----------------------------
SET(CMAKE_CXX_FLAGS                                 "${CMAKE_CXX_FLAGS} -fPIC -std=c++0x -Wall -Wconversion -D_FILE_OFFSET_BITS=64 ${ARCH_CXX_FLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG                           "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE                         "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG")
#SET(CMAKE_CXX_FLAGS_MINSIZEREL                      "")
#SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO                  "")

if(ROYALE_ENABLE_COV)
  SET(CMAKE_CXX_FLAGS_DEBUG                         "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS_DEBUG                           "${CMAKE_C_FLAGS_DEBUG}   -fprofile-arcs -ftest-coverage")
endif()

SET (CMAKE_BUILD_WITH_INSTALL_RPATH OFF CACHE BOOL "")
SET (CMAKE_INSTALL_RPATH "\$ORIGIN/" CACHE STRING "")
