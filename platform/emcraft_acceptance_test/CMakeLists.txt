cmake_minimum_required(VERSION 2.8)

# Create Project
project (acceptance_tests)
set(CMAKE_PREFIX_PATH "/opt/royale/share")

# Compile as C++11
add_compile_options(-std=c++11)

# Sources
set(TARGET_SOURCES
    main.cpp
    camera.cpp
)

# Find Packages
find_package(royale REQUIRED)

# Additional Include Directories
link_directories(${royale_LIB_DIR})
include_directories(
  /opt/royale/samples/inc
)

# Build
add_executable(${PROJECT_NAME} ${TARGET_SOURCES})

# Additional Dependencies
#COPY_ROYALE_LIBS(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} "${royale_LIBS}")
