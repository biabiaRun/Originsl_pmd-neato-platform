set (TEMPERATURE_SOURCES
    "src/PsdTemperatureSensorNtc.cpp"
    "src/TemperatureSensorADS1013_NTC.cpp"
    "src/TemperatureSensorMCP98x43.cpp"
    "src/TemperatureSensorTMP102.cpp"
    )

set (TEMPERATURE_HEADERS
    "inc/temperature/PsdTemperatureSensorNtc.hpp"
    "inc/temperature/TemperatureSensorADS1013_NTC.hpp"
    "inc/temperature/TemperatureSensorMCP98x43.hpp"
    "inc/temperature/TemperatureSensorTMP102.hpp"
    )

set (TEMPERATURE_TESTS
    "test/TestTemperatureSensors.cpp"
    )

add_library(component_temperature OBJECT ${TEMPERATURE_SOURCES} ${TEMPERATURE_HEADERS} )
set_target_properties(component_temperature
    PROPERTIES
    FOLDER components
    EXCLUDE_FROM_ALL true
    )

include_directories( ${gtest_SOURCE_DIR}/include test/inc )
add_library(comptests_temperature OBJECT ${TEMPERATURE_TESTS} )
set_target_properties(comptests_temperature
    PROPERTIES
    FOLDER components/tests
    EXCLUDE_FROM_ALL true
    )
