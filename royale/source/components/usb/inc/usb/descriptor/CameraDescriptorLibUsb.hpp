/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>

#include <libusb.h>

#include <memory>

namespace royale
{
    namespace usb
    {
        namespace descriptor
        {
            /**
             * The result of probing for attached USB devices using libusb.
             */
            class CameraDescriptorLibUsb
            {
            public:
                typedef std::shared_ptr<libusb_context *> ContextRef;

#if defined(TARGET_PLATFORM_ANDROID)
                CameraDescriptorLibUsb (ContextRef &context, libusb_device *dev, uint32_t androidUsbDeviceFD);
#else
                CameraDescriptorLibUsb (ContextRef &context, libusb_device *dev);
#endif
                CameraDescriptorLibUsb (const CameraDescriptorLibUsb &) = delete;
                CameraDescriptorLibUsb &operator= (const CameraDescriptorLibUsb &) = delete;
                ~CameraDescriptorLibUsb();

                /**
                 * The probing and bridge access must use the same libusb_context. This is a shared_ptr to
                 * ensure there is exactly one call to libusb_exit().
                 */
                ContextRef getContext();

                /**
                 * libusb uses refcounting. This CameraDescriptor will keep its reference,
                 * the caller should also increment the count if it intends to keep a reference.
                 */
                libusb_device *getLibUsbDevice();

#if defined(TARGET_PLATFORM_ANDROID)
                /**
                 * An already-opened device FD that should be passed to libusb.  Neither the
                 * CameraDescriptor nor the Bridge take ownership, ownership is held by the
                 * USB Manager Android system service.
                 */
                uint32_t getAndroidUsbDeviceFD()
                {
                    return m_androidUsbDeviceFD;
                }
#endif

            private:
                ContextRef m_usbContext;
                libusb_device *m_dev;
#if defined(TARGET_PLATFORM_ANDROID)
                uint32_t m_androidUsbDeviceFD;
#endif
            };
        }
    }
}

