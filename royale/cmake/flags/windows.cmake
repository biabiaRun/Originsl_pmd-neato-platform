#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

MESSAGE ("Windows flags")

IF(${ARCHITECTURE_BITNESS} STREQUAL 32)
    SET(MACHINE X86)
ELSE(${ARCHITECTURE_BITNESS} STREQUAL 32)
    SET(MACHINE X64)
ENDIF(${ARCHITECTURE_BITNESS} STREQUAL 32)

#----------------------------      GLOBAL C-flags         ----------------------------
SET(CMAKE_C_FLAGS                                   "${CMAKE_C_FLAGS} /DWIN32 /D_WINDOWS /W3 ")
SET(CMAKE_C_FLAGS_DEBUG                             "${CMAKE_C_FLAGS_DEBUG} /D_DEBUG /Zi /Ob0 /Od /RTC1")
SET(CMAKE_C_FLAGS_RELEASE                           "${CMAKE_C_FLAGS_RELEASE} /O2 /Ob2 /D NDEBUG")
SET(CMAKE_C_FLAGS_MINSIZEREL                        "${CMAKE_C_FLAGS_MINSIZEREL} /O1 /Ob1 /D NDEBUG")
SET(CMAKE_C_FLAGS_RELWITHDEBINFO                    "${CMAKE_C_FLAGS_RELWITHDEBINFO} /Zi /O2 /Ob1 /D NDEBUG")
#----------------------------      GLOBAL CXX-flags       ----------------------------
SET(CMAKE_CXX_FLAGS                                 "${CMAKE_CXX_FLAGS} /DWIN32 /D_WINDOWS /W3 /GR /EHsc /fp:fast /arch:SSE2 /Oy /Ot /D_VARIADIC_MAX=10 /DNOMINMAX")
SET(CMAKE_CXX_FLAGS_DEBUG                           "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG /Zi /Ob0 /Od /RTC1 /MDd")
SET(CMAKE_CXX_FLAGS_RELEASE                         "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /D NDEBUG /MD")
SET(CMAKE_CXX_FLAGS_MINSIZEREL                      "${CMAKE_CXX_FLAGS_MINSIZEREL} /O1 /Ob1 /D NDEBUG")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO                  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Zi /O2 /Ob1 /D NDEBUG")

IF(MACHINE STREQUAL X64)
    ROYALE_REMOVE_FROM_STRING(CMAKE_CXX_FLAGS " /arch:SSE2")
ENDIF(MACHINE STREQUAL X64)
