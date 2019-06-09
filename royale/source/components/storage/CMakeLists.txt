cmake_policy(SET CMP0046 NEW)

# Generic
set (STORAGE_SOURCES
    "src/ImagerSpiFirmwareM2452.cpp"
    "src/NonVolatileStoragePersistent.cpp"
    "src/NonVolatileStorageShadow.cpp"
    "src/SpiBusMasterM2453.cpp"
    "src/SpiGenericFlash.cpp"
    "src/SpiImagerFactory.cpp"
    "src/StorageBuffer.cpp"
    "src/StorageFile.cpp"
    "src/StorageFormatPicoLegacy.cpp"
    "src/StorageFormatPolar.cpp"
    "src/StorageFormatZwetschge.cpp"
    "src/StorageI2cEeprom.cpp"
    "src/StorageSpiImagerM2452.cpp"
    )

set (STORAGE_HEADERS
    "inc/storage/IBridgeWithPagedFlash.hpp"
    "inc/storage/ImagerSpiFirmwareM2452.hpp"
    "inc/storage/NonVolatileStoragePersistent.hpp"
    "inc/storage/NonVolatileStorageShadow.hpp"
    "inc/storage/SpiBusMasterM2453.hpp"
    "inc/storage/SpiBusMasterM2455.hpp"
    "inc/storage/SpiGenericFlash.hpp"
    "inc/storage/SpiImagerFactory.hpp"
    "inc/storage/StorageBuffer.hpp"
    "inc/storage/StorageFile.hpp"
    "inc/storage/StorageFormatPicoLegacy.hpp"
    "inc/storage/StorageFormatPolar.hpp"
    "inc/storage/StorageFormatZwetschge.hpp"
    "inc/storage/StorageI2cEeprom.hpp"
    "inc/storage/StorageSpiImagerM2452.hpp"
    )

add_library(component_storage      OBJECT ${STORAGE_SOURCES} ${STORAGE_HEADERS} )
set_target_properties(component_storage
    PROPERTIES
    FOLDER components
    EXCLUDE_FROM_ALL true
    )

# The test subdirectory depends on the python subdirectory, so that the ZwetschgeTool's tests can
# use the generate_zwetschge_file CMake function.
add_subdirectory(python)
add_subdirectory(test)
