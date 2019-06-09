cmake_minimum_required(VERSION 2.8)

# Please insert your PCL path
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} [path to the cmake folder of your PCL installation])

# Check for PCL >= 1.3
find_package(PCL 1.3 QUIET)

if(NOT PCL_FOUND)
  message("PCL example will not be build as no PCL was found!")
  return()
endif()

if (PCL_FOUND AND NOT Qt5Widgets_FOUND)
    # https://bugs.debian.org/867454 and https://bugs.debian.org/867312
    message ("The Point Cloud Library PCL has been found, but it hasn't added its Qt dependency. If you're running Debian or Ubuntu, please install the libvtk6-qt-dev and libfontconfig1-dev packages.")
    return()
endif ()

set(CMAKE_PREFIX_PATH "../../../share")

find_package(royale REQUIRED)

link_directories(${royale_LIB_DIR} ${PCL_LIBRARY_DIRS})

project (samplePCL)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../inc
  SYSTEM ${PCL_INCLUDE_DIRS}
)

add_definitions(${PCL_DEFINITIONS} 
                -D_SCL_SECURE_NO_WARNINGS
                -D_CRT_SECURE_NO_WARNINGS)

add_executable(samplePCL
   samplePCL.cpp
   )

COPY_ROYALE_LIBS(samplePCL)

target_link_libraries(samplePCL ${royale_LIBS} ${PCL_LIBRARIES})
