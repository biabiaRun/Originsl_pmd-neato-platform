/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <usb/descriptor/CameraDescriptorLibUsb.hpp>

#include <memory>

using namespace royale::usb::descriptor;

#if defined(TARGET_PLATFORM_ANDROID)
CameraDescriptorLibUsb::CameraDescriptorLibUsb (ContextRef &context, libusb_device *dev, uint32_t androidUsbDeviceFD) :
#else
CameraDescriptorLibUsb::CameraDescriptorLibUsb (ContextRef & context, libusb_device * dev) :
#endif
    m_usbContext {context},
    m_dev {dev}
#if defined(TARGET_PLATFORM_ANDROID)
    , m_androidUsbDeviceFD (androidUsbDeviceFD)
#endif
{
#ifndef TARGET_PLATFORM_ANDROID
    libusb_ref_device (m_dev);
#endif
}

std::shared_ptr<libusb_context *> CameraDescriptorLibUsb::getContext()
{
    return m_usbContext;
}

libusb_device *CameraDescriptorLibUsb::getLibUsbDevice()
{
    return m_dev;
}

CameraDescriptorLibUsb::~CameraDescriptorLibUsb()
{
#ifndef TARGET_PLATFORM_ANDROID
    libusb_unref_device (m_dev);
#endif
}
