# Generic
set (CONFIG_SOURCES
    "src/CoreConfigAdapter.cpp"
    "src/FlowControlStrategy100k.cpp"
    "src/FlowControlStrategyFixed.cpp"
    )

set (CONFIG_HEADERS
    "inc/config/CoreConfig.hpp"
    "inc/config/CoreConfigAdapter.hpp"
    "inc/config/ExternalConfig.hpp"
    "inc/config/ExternalConfigFileConfig.hpp"
    "inc/config/FlashMemoryConfig.hpp"
    "inc/config/FlowControlStrategy100k.hpp"
    "inc/config/FlowControlStrategyFixed.hpp"
    "inc/config/IImagerExternalConfig.hpp"
    "inc/config/IlluminationConfig.hpp"
    "inc/config/IlluminationPad.hpp"
    "inc/config/ImConnectedTemperatureSensor.hpp"
    "inc/config/ImageDataTransferType.hpp"
    "inc/config/ImagerConfig.hpp"
    "inc/config/ImagerType.hpp"
    "inc/config/ModuleConfig.hpp"
    "inc/config/SensorRoutingFilename.hpp"
    "inc/config/SensorRoutingImagerAsBridge.hpp"
    "inc/config/TemperatureSensorConfig.hpp"
    )

add_library(component_config      OBJECT ${CONFIG_SOURCES} ${CONFIG_HEADERS} )
set_target_properties(component_config
    PROPERTIES
    FOLDER components
    EXCLUDE_FROM_ALL true
    )

add_subdirectory(test)
