set (TEMPERATURE_SOURCES
    "src/PsdTemperatureSensorIc.cpp"
    "src/PsdTemperatureSensorNtc.cpp"
    "src/TemperatureSensorADS1013_NTC.cpp"
    "src/TemperatureSensorMCP98x43.cpp"
    "src/TemperatureSensorTMP102.cpp"
    "src/TemperatureSensorTMP103.cpp"
    )

set (TEMPERATURE_HEADERS
    "inc/temperature/PsdTemperatureSensorIc.hpp"
    "inc/temperature/PsdTemperatureSensorNtc.hpp"
    "inc/temperature/TemperatureSensorADS1013_NTC.hpp"
    "inc/temperature/TemperatureSensorMCP98x43.hpp"
    "inc/temperature/TemperatureSensorTMP102.hpp"
    "inc/temperature/TemperatureSensorTMP103.hpp"
    )

set (TEMPERATURE_TESTS
    "test/TestTemperatureSensors.cpp"
    "test/TestPsdTemperatureSensorIc.cpp"
    ${CMAKE_SOURCE_DIR}/source/core/test/inc/MockRawFrame.hpp
)

add_library(component_temperature OBJECT ${TEMPERATURE_SOURCES} ${TEMPERATURE_HEADERS} )
set_target_properties(component_temperature
    PROPERTIES
    FOLDER components
    EXCLUDE_FROM_ALL true
    )

include_directories(
    ${gtest_SOURCE_DIR}/include
    ${gmock_SOURCE_DIR}/include
    test/inc
    ${CMAKE_SOURCE_DIR}/source/core/test/inc
)
add_library(comptests_temperature OBJECT ${TEMPERATURE_TESTS} )
set_target_properties(comptests_temperature
    PROPERTIES
    FOLDER components/tests
    EXCLUDE_FROM_ALL true
    )
