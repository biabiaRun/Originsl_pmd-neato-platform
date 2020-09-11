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

find_package (PCL)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../royale/source/core/inc
    # ${PCL_INCLUDE_DIRS}
    )

set(HEADERS
  )

add_compile_options(-std=c++11)

set(SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/tof-daemon.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/pipeline_listener.cpp"
    )

link_directories(
    # ${PCL_LIBRARY_DIRS}
    )

# add_definitions(
#   ${PCL_DEFINITIONS}
#   )

add_executable(tof-daemon-prog
    ${HEADERS}
    ${SOURCES}
    )

install(TARGETS tof-daemon-prog DESTINATION ${ROYALE_INSTALL_BIN_DIR})

target_link_libraries(tof-daemon-prog platform)
#target_link_libraries(tof-daemon-prog platform ${PCL_LIBRARIES})
