set (FACTORY_TEST_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/FactoryTestHelpers.hpp"
    )

set(FACTORY_TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FactoryTestHelpers.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestCoreConfigFactory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestModuleConfigFactoryByStorageId.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestModuleConfigFactoryByStorageIdBase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestModuleConfigFactoryFixed.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestModuleConfigFactoryZwetschge.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestNonVolatileStorageFactory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestProcessingParameterMapFactory.cpp"
    )

# The TestNonVolatileStorageFactory.ConstructingStorageFile needs a file which it can read from, and
# have some expected data in it. Use the .cpp file itself.
ADD_DEFINITIONS(-DTEST_FILE_FOR_TESTNONVOLATILESTORAGEFACTORY_CONSTRUCTINGSTORAGEFILE="${CMAKE_CURRENT_SOURCE_DIR}/src/TestNonVolatileStorageFactory.cpp")

include_directories(
    ${gtest_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    $<TARGET_PROPERTY:comptests_imager,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:comptests_storage,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:test_framework_royalecore,INCLUDE_DIRECTORIES>
    )
add_library(comptests_factory OBJECT ${FACTORY_TEST_SOURCES} ${FACTORY_TEST_HEADERS})
set_target_properties(comptests_factory
    PROPERTIES
    FOLDER components/tests
    EXCLUDE_FROM_ALL true
    )
