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

# set(OpenCV_DIR "/opt/fsl-imx-xwayland/4.14-sumo/environment-setup-aarch64-poky-linux/usr/lib/cmake/opencv4")
set(OpenCV_DIR "/home/local/NEATO/mark.wilson/git/robot_lego/fsl-imx-xwayland-glibc-x86_64-neato-rootfs-debug-aarch64-toolchain-4.14-sumo/sysroots/aarch64-poky-linux/usr/lib/cmake/opencv4")
# message(STATUS "opencv path: ${OpenCV_DIR}")
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

