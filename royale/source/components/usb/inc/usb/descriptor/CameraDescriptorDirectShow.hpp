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

#include <usb/bridge/WrappedComPtr.hpp>

#include <ObjIdl.h>

namespace royale
{
    namespace usb
    {
        namespace descriptor
        {
            /**
             * The result of probing for USB Video Class devices via Windows' media framework.
             *
             * This holds a refcount on the pointed-to object.
             */
            class CameraDescriptorDirectShow
            {
            public:
                /**
                 * Constructor which adds its own refcount to the IMoniker.
                 */
                explicit CameraDescriptorDirectShow (royale::usb::bridge::WrappedComPtr<IMoniker> device);
                CameraDescriptorDirectShow (const CameraDescriptorDirectShow &) = delete;
                CameraDescriptorDirectShow &operator= (const CameraDescriptorDirectShow &) = delete;
                ~CameraDescriptorDirectShow ();

                /**
                 * Return a pointer to the device.
                 */
                royale::usb::bridge::WrappedComPtr<IMoniker> getDevice ();

            private:
                royale::usb::bridge::WrappedComPtr<IMoniker> m_dev;
            };
        }
    }
}
