cmake_minimum_required(VERSION 3.0)

set (IMAGER_SOURCES
    "src/DphyPllStrategyM2450_A12.cpp"
    "src/DphyPllStrategyM2452.cpp"
    "src/IImagerModeStrategy.cpp"
    "src/ImagerRawFrame.cpp"
    "src/ImagerUseCaseDefinition.cpp"
    "src/ImagerUseCaseDefinitionUpdater.cpp"
    "src/Imager.cpp"
    "src/ImagerBase.cpp"
    "src/ImagerMeasurementBlockBase.cpp"
    "src/ImagerEmpty.cpp"
    "src/ImagerLenaReader.cpp"
    "src/ImagerMXXXX_Dummy.cpp"    
    "src/ImagerM2450.cpp"
    "src/ImagerM2450_A11.cpp"
    "src/ImagerM2450_A12_AIO.cpp"
    "src/ImagerM2452.cpp"
    "src/ImagerM2452_B1x_AIO.cpp"
    "src/ImagerM2453.cpp"
    "src/ImagerM2453_A11.cpp"
    "src/ImagerM2453_B11.cpp"
    "src/ImagerM2455_A11.cpp"
    "src/ImagerM2455_A14.cpp"
    "src/ImagerRegisterAccess.cpp"
    "src/ModPllStrategyM2450_A12.cpp"
    "src/ModPllStrategyM2452.cpp"
    )

set (IMAGER_HEADERS
    "inc/imager/DphyPllStrategyM2450_A12.hpp"
    "inc/imager/DphyPllStrategyM2452.hpp"
    "inc/imager/IPllStrategy.hpp"
    "inc/imager/IImagerComponent.hpp"
    "inc/imager/IImagerModeStrategy.hpp"
    "inc/imager/IImageSensorLoggingListener.hpp"
    "inc/imager/Imager.hpp"
    "inc/imager/ImagerBase.hpp"
    "inc/imager/ImagerDirectAccess.hpp"
    "inc/imager/ImagerMeasurementBlockBase.hpp"
    "inc/imager/ImagerEmpty.hpp"
    "inc/imager/ImagerLenaReader.hpp"
    "inc/imager/ImagerMXXXX_Dummy.hpp"    
    "inc/imager/ImagerCommon.hpp"
    "inc/imager/ImagerRawFrame.hpp"
    "inc/imager/ImagerUseCaseDefinition.hpp"
    "inc/imager/ImagerUseCaseDefinitionUpdater.hpp"
    "inc/imager/ImagerUseCaseIdentifier.hpp"
    "inc/imager/ImagerParameters.hpp"
    "inc/imager/ImagerM2450.hpp"
    "inc/imager/ImagerM2450_A11.hpp"
    "inc/imager/ImagerM2450_A12_AIO.hpp"
    "inc/imager/ImagerM2452.hpp"
    "inc/imager/ImagerM2452_B1x_AIO.hpp"
    "inc/imager/ImagerM2453.hpp"
    "inc/imager/ImagerM2453_A11.hpp"
    "inc/imager/ImagerM2453_B11.hpp"
    "inc/imager/ImagerM2455_A11.hpp"
    "inc/imager/ImagerM2455_A14.hpp"
    "inc/imager/ImagerRegisterAccess.hpp"
    "inc/imager/ImagerSimpleHexSerialNumber.hpp"
    "inc/imager/M2450_A11/ImagerBaseConfig.hpp"
    "inc/imager/M2450_A11/ImagerRegisters.hpp"
    "inc/imager/M2450_A11/PseudoDataInterpreter.hpp"
    "inc/imager/M2450_A12/ImagerBaseConfig.hpp"
    "inc/imager/M2450_A12/ImagerRegisters.hpp"
    "inc/imager/M2450_A12/PseudoDataInterpreter.hpp"
    "inc/imager/M2450_A12/PseudoDataInterpreter_AIO.hpp"
    "inc/imager/M2450_A12/ConstantsAIOFirmware.hpp"
    "inc/imager/M2450_A12/ImagerAIOFirmware.hpp"
    "inc/imager/M2452/ImagerRegisters.hpp"
    "inc/imager/M2452/PseudoDataInterpreter.hpp"
    "inc/imager/M2452/PseudoDataInterpreter_AIO.hpp"
    "inc/imager/M2453/ImagerRegisters.hpp"
    "inc/imager/M2453/PseudoDataInterpreter.hpp"
    "inc/imager/M2453/PseudoDataInterpreter_B11.hpp"
    "inc/imager/M2452_B1x/ImagerBaseConfig.hpp"
    "inc/imager/M2452_B1x/ImagerAIOFirmware.hpp"
    "inc/imager/M2455/ImagerRegisters.hpp"
    "inc/imager/M2455/PseudoDataInterpreter.hpp"
    "inc/imager/ModPllStrategyM2450_A12.hpp"
    "inc/imager/ModPllStrategyM2452.hpp"
    "inc/imager/MeasurementBlock.hpp"
    "inc/imager/ISequenceGenerator.hpp"
    "inc/imager/WrapperImagerExternalConfig.hpp"
    )

set (IMAGER_TESTS
    "test/inc/LenaTestUtils.hpp"
    "test/inc/ISimImager.hpp"
    "test/inc/SimImagerM2450_A12_AIO.hpp"
    "test/inc/SimImagerM2452.hpp"
    "test/inc/SimImagerM2453.hpp"
    "test/inc/SimImagerMXXXX_Dummy.hpp"
    "test/inc/SimSpiGenericFlash.hpp"
    "test/inc/StubBridgeImager.hpp"
    "test/inc/TestImagerCommon.hpp"
    "test/src/LenaTestUtils.cpp"
    "test/src/SimImagerM2450_A12_AIO.cpp"
    "test/src/SimImagerM2452.cpp"
    "test/src/SimImagerM2453.cpp"
    "test/src/SimImagerM2455.cpp"
    "test/src/SimImagerMXXXX_Dummy.cpp"
    "test/src/SimSpiGenericFlash.cpp"
    "test/src/StubBridgeImager.cpp"
    "test/src/TestImagerLenaReader.cpp"
    "test/src/TestImagerEmpty.cpp"
    "test/src/TestImagerMXXXX_Dummy.cpp"
    "test/src/TestImagerM2450_A11.cpp"
    "test/src/TestImagerM2450_A12_AIO_AlternateBaseConfigs.cpp"
    "test/src/TestImagerM2450_A12_AIO.cpp"
    "test/src/TestImagerM2452.cpp"
    "test/src/TestImagerM2452_B1x_AIO.cpp"
    "test/src/TestImagerM2453_A11.cpp"
    "test/src/TestImagerM2453_B11.cpp"
    "test/src/TestImagerM2455_A11.cpp"
    "test/src/TestImagerRegisterAccess.cpp"
    "test/src/TestImagerSimpleHexSerialNumber.cpp"
    "test/src/TestImagerUseCaseDefinition.cpp"
    "test/src/TestImagerUseCaseIdentifier.cpp"
    "test/src/TestPllStrategyM2450_A12.cpp"
    "test/src/TestPllStrategyM2452.cpp"
    "test/src/TestPseudoDataInterpreterM2450.cpp"
    "test/src/TestPseudoDataInterpreterM2452_AIO.cpp"
    "test/src/TestPseudoDataInterpreterM2453.cpp"
    "test/src/TestPseudoDataInterpreterM2455.cpp"
    )

set(EXTERNAL_CONFIGURATIONS "")    
    
macro(transform_lena_file IN_NAME IN_FOLDER)
    # Read in the lena file and transform the file to a CMake String List
    # (as Lena uses ; as separator we have to replace this first)
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/${IN_FOLDER}/${IN_NAME}.lena" EXTERNAL_CONFIG_STRING)
    string(REGEX REPLACE ";" "%" EXTERNAL_CONFIG_STRING "${EXTERNAL_CONFIG_STRING}")
    string(REGEX REPLACE "\n" ";" EXTERNAL_CONFIG_LIST "${EXTERNAL_CONFIG_STRING}")

    # Put every line into quotes
    set(COUNTER 0)
    set(EXTERNAL_CONFIG_VALUE "")
    list(LENGTH EXTERNAL_CONFIG_LIST LAST_ELEMENT)
    foreach(CURLINE ${EXTERNAL_CONFIG_LIST})
        MATH(EXPR COUNTER "${COUNTER}+1")
        string(STRIP ${CURLINE} CURLINE)
        string(CONCAT EXTERNAL_CONFIG_VALUE "${EXTERNAL_CONFIG_VALUE}        \"${CURLINE}")
        if(NOT COUNTER EQUAL LAST_ELEMENT)
            string(CONCAT EXTERNAL_CONFIG_VALUE "${EXTERNAL_CONFIG_VALUE}\\n\"\n")
        else()
            string(CONCAT EXTERNAL_CONFIG_VALUE "${EXTERNAL_CONFIG_VALUE}\"")
        endif()
    endforeach(CURLINE)
    # and replace the original separators
    string(REGEX REPLACE "%" ";" EXTERNAL_CONFIG_VALUE "${EXTERNAL_CONFIG_VALUE}")

    set(EXTERNAL_CONFIG_NAME ${IN_NAME})
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/inc/imager/ExternalConfig.template" "${CMAKE_CURRENT_BINARY_DIR}/${IN_FOLDER}/${IN_NAME}.hpp" @ONLY)   

    set (EXTERNAL_CONFIGURATIONS
        ${EXTERNAL_CONFIGURATIONS}
        "${CMAKE_CURRENT_BINARY_DIR}/${IN_FOLDER}/${IN_NAME}.hpp"
        )
endmacro()    

transform_lena_file("Salome_M2453" "inc/imager/M2453")
transform_lena_file("Salome2Rev940" "inc/imager/M2453")

add_library(component_imager      OBJECT ${IMAGER_SOURCES} ${IMAGER_HEADERS} ${EXTERNAL_CONFIGURATIONS})
set_target_properties(component_imager
    PROPERTIES
    FOLDER components
    EXCLUDE_FROM_ALL true
    )

# The unit tests can use this to load test data, independent of the current working directory that
# the test is run from.  String literals are concated when side-by-side in the source, so
#    const std::string testM2452_B1xFileName = IMAGER_TEST_FILE_PATH "/C2_M2452_B1x.lena";
# is handled by the compiler.
ADD_DEFINITIONS(-DIMAGER_TEST_FILE_PATH="${ROYALE_RUNTIME_OUTPUT_DIRECTORY}")

set (IMAGER_LENA_TESTDATA
    "C2_M2452_B1x.lena")

# VERSION_GREATER_EQUAL was only added in CMake 3.7 so we need to check both
if((${CMAKE_VERSION} VERSION_GREATER "3.8.0") OR 
   (${CMAKE_VERSION} VERSION_EQUAL "3.8.0"))
   # Group the headers according to their folders
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/inc/imager PREFIX "Header Files" FILES ${IMAGER_HEADERS})
endif()
    
add_custom_target(comptests_imager_copytestdata ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/test/data/${IMAGER_LENA_TESTDATA}" "${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/${IMAGER_LENA_TESTDATA}"
    )
 
SET_TARGET_PROPERTIES(comptests_imager_copytestdata
    PROPERTIES
    FOLDER components/tests
    )
    
include_directories( ${gmock_SOURCE_DIR}/include ${gtest_SOURCE_DIR}/include test/inc )
add_library(comptests_imager OBJECT ${IMAGER_TESTS} )
set_target_properties(comptests_imager
    PROPERTIES
    FOLDER components/tests
    EXCLUDE_FROM_ALL true
    )
    
add_dependencies(comptests_imager comptests_imager_copytestdata)

#TODO Rework copying of lena file to simplify copying and and adding of further lena files
 
add_custom_target(component_imager_copy_configs ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/inc/imager/M2453/Salome2Rev940.lena" "${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/Salome2Rev940.lena"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/inc/imager/M2453/Salome_M2453.lena" "${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/Salome_M2453.lena"
    )

SET_TARGET_PROPERTIES(component_imager_copy_configs
    PROPERTIES
    FOLDER components
    )
    
add_dependencies(component_imager component_imager_copy_configs)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/inc")
