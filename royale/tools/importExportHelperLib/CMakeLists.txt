# Using Qt for image output
find_package(Qt5Widgets REQUIRED)

include_directories(
  ${Qt5Widgets_INCLUDE_DIRS}
  )

set (SOURCE
    "src/csvHelper.cpp"
    "src/plyHelper.cpp"
    "src/paramHelper.cpp"
    "src/imgHelper.cpp"
    "src/binHelper.cpp")

set (HEADER
    "inc/importExportHelperLib/csvHelper.hpp"
    "inc/importExportHelperLib/plyHelper.hpp"
    "inc/importExportHelperLib/paramHelper.hpp"
    "inc/importExportHelperLib/imgHelper.hpp"
    "inc/importExportHelperLib/binHelper.hpp")

add_library(importExportHelperLib STATIC ${SOURCE} ${HEADER})

target_link_libraries(importExportHelperLib PUBLIC royale)
target_link_libraries(importExportHelperLib PUBLIC Qt5::Widgets)

target_include_directories(importExportHelperLib
    PUBLIC
        "inc"
    PRIVATE
        "../../source/royale/inc"
        "../../source/core/inc")

set_target_properties(importExportHelperLib PROPERTIES
    LINKER_LANGUAGE CXX
    FOLDER tools)
