# Generic
set (FACTORY_SOURCES
    "src/CameraCoreBuilderFactory.cpp"
    "src/CameraCoreBuilderImpl.cpp"
    "src/CoreConfigFactory.cpp"
    "src/FlashDefinedImagerInterfaceAdapter.cpp"
    "src/SoftwareDefinedImagerInterfaceAdapter.cpp"
    "src/CameraCoreBuilderMira.cpp"
    "src/ImagerFactory.cpp"
    "src/ModuleConfigFactoryByStorageId.cpp"
    "src/ModuleConfigFactoryByStorageIdBase.cpp"
    "src/ModuleConfigFactoryFixed.cpp"
    "src/ModuleConfigFactoryZwetschge.cpp"
    "src/PsdTemperatureSensorFactory.cpp"
    "src/ProcessingParameterMapFactory.cpp"
    "src/TemperatureSensorFactory.cpp"
    "src/NonVolatileStorageFactory.cpp"
    "src/FlowControlStrategyFactory.cpp"
    )

set (FACTORY_HEADERS
    "inc/factory/CameraCoreBuilderFactory.hpp"
    "inc/factory/CameraCoreBuilderImpl.hpp"
    "inc/factory/CameraCoreBuilderMira.hpp"
    "inc/factory/ImagerFactory.hpp"
    "inc/factory/FlashDefinedImagerInterfaceAdapter.hpp"
    "inc/factory/SoftwareDefinedImagerInterfaceAdapter.hpp"
    "inc/factory/ImagerUseCaseDefinitionAdapter.hpp"
    "inc/factory/ImagerConfigAdapter.hpp"
    "inc/factory/CoreConfigFactory.hpp"
    "inc/factory/ICameraCoreBuilder.hpp"
    "inc/factory/IModuleConfigFactory.hpp"
    "inc/factory/IProcessingParameterMapFactory.hpp"
    "inc/factory/ModuleConfigFactoryByStorageId.hpp"
    "inc/factory/ModuleConfigFactoryByStorageIdBase.hpp"
    "inc/factory/ModuleConfigFactoryFixed.hpp"
    "inc/factory/ModuleConfigFactoryZwetschge.hpp"
    "inc/factory/PsdTemperatureSensorFactory.hpp"
    "inc/factory/ProcessingParameterMapFactory.hpp"
    "inc/factory/TemperatureSensorFactory.hpp"
    "inc/factory/NonVolatileStorageFactory.hpp"
    "inc/factory/FlowControlStrategyFactory.hpp"
    )

add_library(component_factory      OBJECT ${FACTORY_SOURCES} ${FACTORY_HEADERS} )
set_target_properties(component_factory
    PROPERTIES
    FOLDER components
    EXCLUDE_FROM_ALL true
    )

add_subdirectory(test)
