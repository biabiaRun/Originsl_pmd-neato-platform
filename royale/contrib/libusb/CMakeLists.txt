IF (NOT(${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID))
    RETURN()
ENDIF()

SET(LIBUSBFOLDER "${CMAKE_CURRENT_SOURCE_DIR}/libusb" CACHE STRING "" FORCE)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${LIBUSBFOLDER}
   )

SET(HEADER_FILES
    config.h
    ${LIBUSBFOLDER}/hotplug.h
    ${LIBUSBFOLDER}/libusb.h
    ${LIBUSBFOLDER}/libusbi.h
    ${LIBUSBFOLDER}/version.h
    ${LIBUSBFOLDER}/version_nano.h
    ${LIBUSBFOLDER}/os/linux_usbfs.h
    ${LIBUSBFOLDER}/os/poll_posix.h
    ${LIBUSBFOLDER}/os/threads_posix.h
    )
SET(SOURCE_FILES
    ${LIBUSBFOLDER}/core.c
    ${LIBUSBFOLDER}/descriptor.c
    ${LIBUSBFOLDER}/hotplug.c
    ${LIBUSBFOLDER}/io.c
    ${LIBUSBFOLDER}/strerror.c
    ${LIBUSBFOLDER}/sync.c
    ${LIBUSBFOLDER}/os/linux_netlink.c
    ${LIBUSBFOLDER}/os/linux_usbfs.c
    ${LIBUSBFOLDER}/os/poll_posix.c
    ${LIBUSBFOLDER}/os/threads_posix.c
    )
    
add_library( usb_android SHARED
    ${HEADER_FILES}
    ${SOURCE_FILES}
    )

target_link_libraries(usb_android log)    
    
INSTALL(TARGETS usb_android
    LIBRARY DESTINATION ${ROYALE_INSTALL_BIN_DIR}
)
INSTALL(TARGETS usb_android
    LIBRARY DESTINATION ${ROYALE_INSTALL_ANDROID_NATIVE_SAMPLE_LIB_DIR}
)
