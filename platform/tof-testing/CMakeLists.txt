#*********************************************************************************************************************
#  * Copyright (C) 2020, Neato Robotics, Inc.. All Rights Reserved.
#  *
#  * This file may contain contributions from others.
#  *
#  * This software is proprietary to Neato Robotics, Inc. and its transference and use is to be strictly controlled.
#  * Transference of this software to another party requires that all of the following conditions be met:
#  * 	A)	Neato has a copy of a signed NDA agreement with the receiving party
#  * 	B)	Neato Software Engineering has explicitly authorized the receiving party to have a copy of this software
#  * 	C)	When the work is completed or terminated by the receiving party, all copies of this software that the
#  *                             receiving party holds must be returned to Neato, or destroyed.
#  *  The receiving party is under legal obligation to not disclose or  transfer this software.
#  *  The receiving party may not appropriate, transform or re-use this software for any purpose other than a
#  *         Neato Robotics authorized purpose.
#  *
#*********************************************************************************************************************

if(NOT YOCTO)
  # Note: Setting CMAKE_SYSROOT violates documented use but doc also
  # says that it is the prefix the search paths for the find_* functions
  #
  set(CMAKE_SYSROOT $ENV{OECORE_TARGET_SYSROOT})
endif()

find_package(OpenCV REQUIRED)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../royale/source/core/inc
  ${OpenCV_INCLUDE_DIRS}
  )

set(HEADERS
  )

add_compile_options(-std=c++11)

set(SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/tof-testing.cpp"
  )

link_directories(
  )

add_executable(tof-testing-points
  ${HEADERS}
  ${SOURCES}
  )

install(TARGETS tof-testing-points DESTINATION ${ROYALE_INSTALL_BIN_DIR})

target_link_libraries(tof-testing-points platform ${OpenCV_LIBS})

