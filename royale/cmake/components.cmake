#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

#
# Defines, includes and linker settings for configured components.
#

if (ROYALE_LOGGING_VERBOSE_BRIDGE)
    ADD_DEFINITIONS ("-DROYALE_LOGGING_VERBOSE_BRIDGE")
endif()

# Amundsen bridges
# The protocol is UVC-like, but not using the operating system's UVC stack, nor using Win10's Frame
# Server.  The implementation uses the same USB access libraries as Enclustra.
if (ROYALE_BRIDGE_AMUNDSEN)
    # Common to all variants
    ADD_DEFINITIONS ("-DROYALE_BRIDGE_AMUNDSEN")
endif()
if (ROYALE_BRIDGE_AMUNDSEN_LIBUSB)
    ADD_DEFINITIONS ("-DROYALE_BRIDGE_AMUNDSEN_LIBUSB")
endif()
if (ROYALE_BRIDGE_AMUNDSEN_CYAPI)
    ADD_DEFINITIONS ("-DROYALE_BRIDGE_AMUNDSEN_CYAPI")
endif()

# Enclustra bridges
if (ROYALE_BRIDGE_ENCLUSTRA)
    # Common to all variants
    ADD_DEFINITIONS ("-DROYALE_BRIDGE_ENCLUSTRA")
endif()
if (ROYALE_BRIDGE_ENCLUSTRA_LIBUSB)
    ADD_DEFINITIONS ("-DROYALE_BRIDGE_ENCLUSTRA_LIBUSB")
endif()
if (ROYALE_BRIDGE_ENCLUSTRA_CYAPI)
    ADD_DEFINITIONS ("-DROYALE_BRIDGE_ENCLUSTRA_CYAPI")
endif()


# Both Video4Linux and DirectShow could be used for non-UVC devices.

# UVC bridges
if (ROYALE_BRIDGE_UVC)
    # Common to all variants
    ADD_DEFINITIONS ("-DROYALE_BRIDGE_UVC")
    ADD_DEFINITIONS ("-DTEST_UVC")
endif()
if (ROYALE_BRIDGE_UVC_AMUNDSEN)
    # Use the Amundsen implementation instead of the system's UVC implementation,
    # even for devices that identify themselves as UVC.
    ADD_DEFINITIONS ("-DROYALE_BRIDGE_UVC_AMUNDSEN")
endif()
if (ROYALE_BRIDGE_UVC_V4L)
    ADD_DEFINITIONS ("-DBRIDGE_UVC_IMPLEMENTATION_V4L")
    ADD_DEFINITIONS ("-DROYALE_BRIDGE_UVC_V4L")
    ADD_DEFINITIONS ("-DTEST_UVC_V4L")
endif()
if (ROYALE_BRIDGE_UVC_DIRECTSHOW)
    ADD_DEFINITIONS ("-DROYALE_BRIDGE_UVC_DIRECTSHOW")
endif()
if (ROYALE_BRIDGE_EXTENSION_ARCTIC)
    ADD_DEFINITIONS ("-DROYALE_BRIDGE_EXTENSION_ARCTIC")
endif()

# External libraries:
set (ROYALE_USB_LINK_COMMAND "" CACHE STRING "" FORCE)

# CyApi
if (ROYALE_USE_CYAPI)
    list(APPEND ROYALE_USB_LINK_COMMAND "CyAPI" "SetupAPI")
endif()

# DirectShow
if (ROYALE_USE_DIRECTSHOW)
    list(APPEND ROYALE_USB_LINK_COMMAND "SetupAPI" "strmiids")
endif()


# LibUsb
if (ROYALE_USE_LIBUSB)
    if (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
        include_directories( ${LIBUSBFOLDER} )
        list(APPEND ROYALE_USB_LINK_COMMAND "usb_android")
    else (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
        find_library(LIBUSB_LIBRARY_NAMES usb-1.0)
        find_path(LIBUSB_INCLUDE_DIR libusb-1.0/libusb.h)
        include_directories(
            ${LIBUSB_INCLUDE_DIR}
            ${LIBUSB_INCLUDE_DIR}/libusb-1.0
            )
        list(APPEND ROYALE_USB_LINK_COMMAND ${LIBUSB_LIBRARY_NAMES})
    endif (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
endif()

# Using udev to enumerate V4L devices
if (ROYALE_USE_V4L)
    list(APPEND ROYALE_USB_LINK_COMMAND udev)
endif()

# ROYALE_USE_V4L uses the kernel API only, and does not need libv4l.

# ROYALE_USB_LINK_COMMAND is now a local variable, have to put it into the cache.
list(REMOVE_DUPLICATES ROYALE_USB_LINK_COMMAND)
set (ROYALE_USB_LINK_COMMAND "${ROYALE_USB_LINK_COMMAND}" CACHE STRING "" FORCE)
