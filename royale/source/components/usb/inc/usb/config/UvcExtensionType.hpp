/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

namespace royale
{
    namespace usb
    {
        namespace config
        {
            /**
             * There is more than one type of firmware for devices, and more than one protocol that
             * can be passed over the UVC Vendor Extension.  This enum lists the known protocols.
             *
             * UVC Vendor Extensions are recognised by GUIDs.  Because different platforms have
             * platform-specific ways to handle GUIDs, the mapping from this enum to the GUIDs is in
             * in separate platform-specific code.
             */
            enum class UvcExtensionType
            {
                None,
                Arctic
            };
        }
    }
}
