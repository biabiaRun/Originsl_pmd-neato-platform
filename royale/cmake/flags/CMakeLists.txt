#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

ROYALE_INCLUDE_TARGET_FILE("${ROYALE_SOURCE_DIR}/cmake/flags")

if (ROYALE_LOGGING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DROYALE_USE_LOGGING")
endif ()

if (ROYALE_LOGGING_FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DROYALE_FORCE_LOGGING -DROYALE_USE_LOGGING")
endif ()
