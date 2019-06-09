# Generic
set (USB_SOURCES
    "src/UsbProbeData.cpp"
    "src/UsbSpeed.cpp"
    )

set (USB_HEADERS
    "inc/usb/bridge/WrappedComPtr.hpp"
    "inc/usb/config/BridgeType.hpp"
    "inc/usb/config/UsbProbeData.hpp"
    "inc/usb/enumerator/IBusEnumerator.hpp"
    "inc/usb/factory/IBridgeFactory.hpp"
    "inc/usb/factory/IBridgeFactoryMira.hpp"
    "inc/usb/pal/UsbSpeed.hpp"
    )

set (USB_TESTS
    "test/TestWrappedComPtr.cpp"
    )

# Configuration dependant (i.e. OS and USB access layer specific)
set (USB_SOURCES_LIBUSB
    "src/BusEnumeratorLibUsb.cpp"
    "src/CameraDescriptorLibUsb.cpp"
    )

set (USB_SOURCES_CYAPI
    "src/BusEnumeratorCyApi.cpp"
    )

set (USB_SOURCES_AMUNDSEN_COMMON
    "src/BridgeAmundsenCommon.cpp"
    )

set (USB_SOURCES_AMUNDSEN_LIBUSB
    "src/BridgeAmundsenLibUsb.cpp"
    "src/BusEnumeratorAmundsenLibUsb.cpp"
    "src/BridgeFactoryAmundsenLibUsb.cpp"
    )

set (USB_SOURCES_AMUNDSEN_CYAPI
    "src/BridgeAmundsenCyApi.cpp"
    "src/BridgeFactoryAmundsenCyApi.cpp"
    "src/BusEnumeratorAmundsenCyApi.cpp"
    )

set (USB_SOURCES_ENCLUSTRA_COMMON
    "src/BridgeEnclustra.cpp"
    )

set (USB_SOURCES_ENCLUSTRA_LIBUSB
    "src/BridgeEnclustraLibUsb.cpp"
    "src/BridgeFactoryEnclustraLibUsb.cpp"
    "src/BusEnumeratorEnclustraLibUsb.cpp"
    )

set (USB_SOURCES_ENCLUSTRA_CYAPI
    "src/BridgeEnclustraCyApi.cpp"
    "src/BridgeFactoryEnclustraCyApi.cpp"
    "src/BusEnumeratorEnclustraCyApi.cpp"
    )

set (USB_SOURCES_ARCTIC
    "src/BridgeFactoryArcticCommon.cpp"
    "src/BridgeImagerArctic.cpp"
    "src/DeviceControlArctic.cpp"
    "src/GpioAccessArctic.cpp"
    "src/I2cBusAccessArctic.cpp"
    "src/SpiBusAccessArctic.cpp"
    "src/StorageSpiFlashArctic.cpp"
    "src/UvcExtensionArctic.cpp"
    )

set (USB_SOURCES_UVC_COMMON
    "src/BridgeUvcCommon.cpp"
    )

set (USB_SOURCES_UVC_V4L
    "src/BridgeFactoryUvcV4l.cpp"
    "src/BusEnumeratorUvcV4l.cpp"
    "src/BridgeUvcV4l.cpp"
    "src/BridgeUvcV4lUvcExtension.cpp"
    )

set (USB_SOURCES_UVC_DIRECTSHOW
    "src/BridgeFactoryUvcDirectShow.cpp"
    "src/BusEnumeratorUvcDirectShow.cpp"
    "src/CameraDescriptorDirectShow.cpp"
    "src/BridgeUvcDirectShow.cpp"
    "src/BridgeUvcWindowsUvcExtension.cpp"
    )


set (USB_HEADERS_LIBUSB
    "inc/usb/descriptor/CameraDescriptorLibUsb.hpp"
    "inc/usb/enumerator/BusEnumeratorLibUsb.hpp"
    )

set (USB_HEADERS_CYAPI
    "inc/usb/enumerator/BusEnumeratorCyApi.hpp"
    )

set (USB_HEADERS_AMUNDSEN_COMMON
    "inc/usb/bridge/AmundsenCircularBuffer.hpp"
    "inc/usb/bridge/BridgeAmundsenCommon.hpp"
    )

set (USB_HEADERS_AMUNDSEN_LIBUSB
    "inc/usb/bridge/BridgeAmundsenLibUsb.hpp"
    "inc/usb/enumerator/BusEnumeratorAmundsenLibUsb.hpp"
    "inc/usb/factory/BridgeFactoryAmundsenLibUsb.hpp"
    )

set (USB_HEADERS_AMUNDSEN_CYAPI
    "inc/usb/bridge/BridgeAmundsenCyApi.hpp"
    "inc/usb/enumerator/BusEnumeratorAmundsenCyApi.hpp"
    "inc/usb/factory/BridgeFactoryAmundsenCyApi.hpp"
    )

set (USB_HEADERS_ENCLUSTRA_COMMON
    "inc/usb/bridge/BridgeEnclustra.hpp"
    )

set (USB_HEADERS_ENCLUSTRA_LIBUSB
    "inc/usb/bridge/BridgeEnclustraLibUsb.hpp"
    "inc/usb/enumerator/BusEnumeratorEnclustraLibUsb.hpp"
    "inc/usb/factory/BridgeFactoryEnclustraLibUsb.hpp"
    )

set (USB_HEADERS_ENCLUSTRA_CYAPI
    "inc/usb/bridge/BridgeEnclustraCyApi.hpp"
    "inc/usb/enumerator/BusEnumeratorEnclustraCyApi.hpp"
    "inc/usb/factory/BridgeFactoryEnclustraCyApi.hpp"
    )

set (USB_HEADERS_ARCTIC
    "inc/usb/bridge/ArcticProtocolConstants.hpp"
    "inc/usb/bridge/BridgeImagerArctic.hpp"
    "inc/usb/bridge/IUvcExtensionAccess.hpp"
    "inc/usb/bridge/StorageSpiFlashArctic.hpp"
    "inc/usb/bridge/UvcExtensionArctic.hpp"
    "inc/usb/config/UvcExtensionType.hpp"
    "inc/usb/factory/BridgeFactoryArcticCommon.hpp"
    "inc/usb/pal/DeviceControlArctic.hpp"
    "inc/usb/pal/GpioAccessArctic.hpp"
    "inc/usb/pal/I2cBusAccessArctic.hpp"
    "inc/usb/pal/IDeviceStatus.hpp"
    "inc/usb/pal/SpiBusAccessArctic.hpp"
    )

set (USB_HEADERS_UVC_COMMON
    "inc/usb/bridge/BridgeUvcCommon.hpp"
    )

set (USB_HEADERS_UVC_DIRECTSHOW
    "inc/usb/bridge/BridgeUvcDirectShow.hpp"
    "inc/usb/bridge/BridgeUvcWindowsUvcExtension.hpp"
    "inc/usb/descriptor/CameraDescriptorDirectShow.hpp"
    "inc/usb/enumerator/BusEnumeratorUvcDirectShow.hpp"
    "inc/usb/factory/BridgeFactoryUvcDirectShow.hpp"
    )

set (USB_HEADERS_UVC_V4L
    "inc/usb/bridge/BridgeUvcV4l.hpp"
    "inc/usb/bridge/BridgeUvcV4lUvcExtension.hpp"
    "inc/usb/enumerator/BusEnumeratorUvcV4l.hpp"
    "inc/usb/factory/BridgeFactoryUvcV4l.hpp"
    )

set (USB_TESTS_AMUNDSEN
    "test/TestAmundsenCircularBuffer.cpp"
    "test/TestBridgeAmundsen.cpp"
    )

set (USB_TESTS_ENCLUSTRA
    "test/TestBridgeEnclustra.cpp"
    )

set (USB_TESTS_UVC
    "test/TestProbeUvc.cpp"
    "test/TestUvcExtensionArctic.cpp"
    )

# Amundsen bridges
if (ROYALE_BRIDGE_AMUNDSEN)
    # Common to all variants
    set(USB_SOURCES ${USB_SOURCES} ${USB_SOURCES_AMUNDSEN_COMMON})
    set(USB_HEADERS ${USB_HEADERS} ${USB_HEADERS_AMUNDSEN_COMMON})
    set(USB_TESTS ${USB_TESTS} ${USB_TESTS_AMUNDSEN})
endif()
if (ROYALE_BRIDGE_AMUNDSEN_LIBUSB)
    set(USB_SOURCES ${USB_SOURCES} ${USB_SOURCES_AMUNDSEN_LIBUSB})
    set(USB_HEADERS ${USB_HEADERS} ${USB_HEADERS_AMUNDSEN_LIBUSB})
endif()
if (ROYALE_BRIDGE_AMUNDSEN_CYAPI)
    set(USB_SOURCES ${USB_SOURCES} ${USB_SOURCES_AMUNDSEN_CYAPI})
    set(USB_HEADERS ${USB_HEADERS} ${USB_HEADERS_AMUNDSEN_CYAPI})
endif()

# Enclustra bridges
if (ROYALE_BRIDGE_ENCLUSTRA)
    # Common to all variants
    set(USB_SOURCES ${USB_SOURCES} ${USB_SOURCES_ENCLUSTRA_COMMON})
    set(USB_HEADERS ${USB_HEADERS} ${USB_HEADERS_ENCLUSTRA_COMMON})
    set(USB_TESTS ${USB_TESTS} ${USB_TESTS_ENCLUSTRA})
endif()
if (ROYALE_BRIDGE_ENCLUSTRA_LIBUSB)
    set(USB_SOURCES ${USB_SOURCES} ${USB_SOURCES_ENCLUSTRA_LIBUSB})
    set(USB_HEADERS ${USB_HEADERS} ${USB_HEADERS_ENCLUSTRA_LIBUSB})
endif()
if (ROYALE_BRIDGE_ENCLUSTRA_CYAPI)
    set(USB_SOURCES ${USB_SOURCES} ${USB_SOURCES_ENCLUSTRA_CYAPI})
    set(USB_HEADERS ${USB_HEADERS} ${USB_HEADERS_ENCLUSTRA_CYAPI})
endif()

# UVC bridges
if (ROYALE_BRIDGE_UVC)
    # Common to all variants
    set(USB_SOURCES ${USB_SOURCES} ${USB_SOURCES_UVC_COMMON})
    set(USB_HEADERS ${USB_HEADERS} ${USB_HEADERS_UVC_COMMON})
    set(USB_TESTS ${USB_TESTS} ${USB_TESTS_UVC})
endif()
if (ROYALE_BRIDGE_UVC_DIRECTSHOW)
    set(USB_SOURCES ${USB_SOURCES} ${USB_SOURCES_UVC_DIRECTSHOW})
    set(USB_HEADERS ${USB_HEADERS} ${USB_HEADERS_UVC_DIRECTSHOW})
endif()
if (ROYALE_BRIDGE_UVC_V4L)
    add_definitions ("-DROYALE_BRIDGE_UVC" "-DROYALE_BRIDGE_UVC_V4L")
    set(USB_SOURCES ${USB_SOURCES} ${USB_SOURCES_UVC_V4L})
    set(USB_HEADERS ${USB_HEADERS} ${USB_HEADERS_UVC_V4L})
endif()

# Arctic UVC extension, used by UVC and Amundsen
if (ROYALE_BRIDGE_EXTENSION_ARCTIC)
    set(USB_SOURCES ${USB_SOURCES} ${USB_SOURCES_ARCTIC})
    set(USB_HEADERS ${USB_HEADERS} ${USB_HEADERS_ARCTIC})
endif()

# LibUsb
if (ROYALE_USE_LIBUSB)
    set(USB_SOURCES ${USB_SOURCES} ${USB_SOURCES_LIBUSB})
    set(USB_HEADERS ${USB_HEADERS} ${USB_HEADERS_LIBUSB})

    if (TARGET usb_android)
        # We're building our own libusb, include the headers.
        include_directories( $<TARGET_PROPERTY:usb_android,INCLUDE_DIRECTORIES> )
    endif()
endif()

if (ROYALE_USE_CYAPI)
    set(USB_SOURCES ${USB_SOURCES} ${USB_SOURCES_CYAPI})
    set(USB_HEADERS ${USB_HEADERS} ${USB_HEADERS_CYAPI})

    # Add the transitive includes, in the same way as for uvc, above.
    include_directories( $<TARGET_PROPERTY:CyAPI,INTERFACE_INCLUDE_DIRECTORIES> )
endif()

add_library(component_usb OBJECT ${USB_SOURCES} ${USB_HEADERS} )
set_target_properties(component_usb
    PROPERTIES
    FOLDER components
    EXCLUDE_FROM_ALL true
    )

include_directories( ${gmock_SOURCE_DIR}/include 
                     ${gtest_SOURCE_DIR}/include 
                     test/inc 
                     $<TARGET_PROPERTY:test_framework_royalecore,INCLUDE_DIRECTORIES>
                     ${ROYALE_SAMPLE_UTILS_FOLDER})
add_library(comptests_usb OBJECT ${USB_TESTS} )
set_target_properties(comptests_usb
    PROPERTIES
    FOLDER components/tests
    EXCLUDE_FROM_ALL true
    )
