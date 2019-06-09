#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

#----------------------------      GLOBAL C-flags         ----------------------------
SET(CMAKE_C_FLAGS                                    "${CMAKE_C_FLAGS} -Wno-deprecated -Wall -Wno-deprecated-declarations")
#SET(CMAKE_C_FLAGS                                   "${CMAKE_C_FLAGS}   -fPIC")
#SET(CMAKE_C_FLAGS_DEBUG                             "${CMAKE_C_FLAGS_DEBUG}")
#SET(CMAKE_C_FLAGS_RELEASE                           "${CMAKE_C_FLAGS_RELEASE}")
#SET(CMAKE_C_FLAGS_MINSIZEREL                        "${CMAKE_C_FLAGS_MINSIZEREL}")
#SET(CMAKE_C_FLAGS_RELWITHDEBINFO                    "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
#----------------------------      GLOBAL CXX-flags       ----------------------------
SET(CMAKE_CXX_FLAGS                                 "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wall -Wsign-compare -Wuninitialized -Wunused -Wno-unused-local-typedef -Wno-inconsistent-missing-override -Wno-deprecated-declarations -std=c++11")
#SET(CMAKE_CXX_FLAGS_DEBUG                           "${CMAKE_CXX_FLAGS_DEBUG}")
#SET(CMAKE_CXX_FLAGS_RELEASE                         "-O2 -s -DNDEBUG")
#SET(CMAKE_CXX_FLAGS_MINSIZEREL                      "${CMAKE_CXX_FLAGS_MINSIZEREL}")
#SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO                  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
#SET(CMAKE_SHARED_LINKER_FLAGS                        "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
#SET(CMAKE_EXE_LINKER_FLAGS                           "${CMAKE_EXE_LINKER_FLAGS} --coverage")

if(ROYALE_ENABLE_COV)
  SET(CMAKE_CXX_FLAGS                                 "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -D_FILE_OFFSET_BITS=64")
  SET(CMAKE_C_FLAGS                                   "${CMAKE_C_FLAGS}   -fprofile-arcs -ftest-coverage -D_FILE_OFFSET_BITS=64")
endif()

SET(CMAKE_MACOSX_RPATH TRUE CACHE BOOL "")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH OFF CACHE BOOL "")
SET(CMAKE_INSTALL_RPATH "@loader_path" CACHE STRING "")
