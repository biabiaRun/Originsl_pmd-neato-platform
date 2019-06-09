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
SET(CMAKE_C_FLAGS                                   "${CMAKE_C_FLAGS} -fPIC -D_FILE_OFFSET_BITS=64")
#SET(CMAKE_C_FLAGS_DEBUG                             "")
#SET(CMAKE_C_FLAGS_RELEASE                           "")
#SET(CMAKE_C_FLAGS_MINSIZEREL                        "")
#SET(CMAKE_C_FLAGS_RELWITHDEBINFO                    "" )
#----------------------------      GLOBAL CXX-flags       ----------------------------
SET(CMAKE_CXX_FLAGS                                 "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -ffast-math -D_FILE_OFFSET_BITS=64")
#SET(CMAKE_CXX_FLAGS_DEBUG                           "")
SET(CMAKE_CXX_FLAGS_RELEASE                         "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -s -DNDEBUG")
#SET(CMAKE_CXX_FLAGS_MINSIZEREL                      "")
#SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO                  "")
