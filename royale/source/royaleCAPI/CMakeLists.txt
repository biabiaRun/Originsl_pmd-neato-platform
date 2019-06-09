include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../royale/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../royale/inc/private
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/inc/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/inc
    )
    
link_directories(
    ${PROCESSING_SPECTRE_LIB_DIR}
    )

set(HEADERS_ROYALE_CAPI
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/CAPIMigration.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/CAPIVersion.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/CAPIVersion220.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/CAPIVersion300.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/CAPIVersion320.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/CAPIVersion330.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/CAPIVersion31000.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/CameraDeviceCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/CameraManagerCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/DataStructuresCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/DefinitionsCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/DepthDataCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/DepthImageCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/EventCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/ExposureCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/ExposureModeCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/ExtendedDataCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/IRImageCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/IntermediateDataCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/LensParametersCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/ProcessingParametersCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/RawDataCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/RecordCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/SparsePointCloudCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/StatusCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/TriggerModeCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/VariantCAPI.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royaleCAPI.h"
    )
    
set(HEADERS_ROYALE_CAPI_PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/CameraDeviceCreateCAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/DepthDataListenerCAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/DepthImageListenerCAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/EventListenerCAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/ExposureListenerCAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/ExtendedDataListenerCAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/HelperFunctionsCAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/IRImageListenerCAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/InstanceManagerCAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/ProcessingParametersConverterCAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/RecordStopListenerCAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/SparsePointCloudListenerCAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/VariantManagedCAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/CameraDeviceManagedCAPI.hpp"
    )
    
set(SOURCES_ROYALE_CAPI
    "${CMAKE_CURRENT_SOURCE_DIR}/src/CameraDeviceCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/CameraManagerCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/DataStructuresCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/DepthDataListenerCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/DepthImageListenerCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventListenerCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ExposureListenerCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ExtendedDataListenerCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/HelperFunctionsCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/IRImageListenerCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/LensParametersCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ProcessingParametersCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RecordStopListenerCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/SparsePointCloudListenerCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/StatusCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/VariantCAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/royaleCAPI.cpp"
    )
    
source_group(royaleCAPI FILES
    ${HEADERS_ROYALE_CAPI}
    )
    
add_library(royaleCAPI SHARED 
    ${HEADERS_ROYALE_CAPI_PRIVATE}
    ${HEADERS_ROYALE_CAPI}
    ${SOURCES_ROYALE_CAPI}
    )
    
add_definitions(-DROYALE_CAPI_EXPORTS)
    
set_target_properties(royaleCAPI
    PROPERTIES
    OUTPUT_NAME royaleCAPI
    FOLDER modules
    VERSION ${ROYALE_VERSION_MAJOR}.${ROYALE_VERSION_MINOR}.${ROYALE_VERSION_PATCH}
    )
    
target_link_libraries(royaleCAPI royale)
    
install(FILES ${HEADERS_ROYALE_CAPI} DESTINATION "${ROYALE_INSTALL_INCLUDE_DIR}/royaleCAPI")

IF(${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
    INSTALL(TARGETS royaleCAPI
        RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR}
        LIBRARY DESTINATION ${ROYALE_INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${ROYALE_INSTALL_LIB_DIR}
        )
        
    INSTALL(TARGETS royaleCAPI
        RUNTIME DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetRetrieveData"
        LIBRARY DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetRetrieveData"
        ARCHIVE DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetRetrieveData"
        )

    INSTALL(TARGETS royaleCAPI
        RUNTIME DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetCamInfo"
        LIBRARY DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetCamInfo"
        ARCHIVE DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetCamInfo"
        )
        
ELSE()
    INSTALL(TARGETS royaleCAPI
        RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR}
        LIBRARY DESTINATION ${ROYALE_INSTALL_BIN_DIR}
        ARCHIVE DESTINATION ${ROYALE_INSTALL_BIN_DIR}
        )
ENDIF()

if(NOT ${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    add_subdirectory(test)
    add_subdirectory(test_v220)
endif()
