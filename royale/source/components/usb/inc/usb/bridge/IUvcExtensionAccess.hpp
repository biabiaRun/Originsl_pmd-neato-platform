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

#include <common/exceptions/RuntimeError.hpp>
#include <usb/config/UvcExtensionType.hpp>

#include <mutex>
#include <cstdint>
#include <vector>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            /**
             * The USB Video Class specification allows devices to have vendor-specific extensions,
             * which are accessed in a platform specific way.  This class provides a common interface to
             * separate the platform-specific part of an IBridgeImager from the implementation of our
             * vendor extension.
             *
             * Every UVC extension has a GUID (the configuration of which isn't handled by this class),
             * and a number of controls.  The control numbers are the setupId and dataId arguments to
             * these methods.
             *
             * What the extension commands mean is defined by the IBridgeImager implementation, they're
             * opaque to this class.  The reference protocol's control channel is based on first sending
             * a setup command, and then separately reading or writing data.
             *
             * All the methods in this class must be threadsafe.  Some protocols need to make multiple
             * Get/Set calls with thread-safety between those calls, in this case lockVendorExtension()
             * provides that safety.  To support lockVendorExtension(), all the individual Get/Set
             * methods must also lock the same (recursive) mutex.
             *
             * All the I/O methods can throw generic RuntimeErrors, indicating an unknown I/O error.
             * They may also throw PossiblyUsbStallError, which specifically means that the error may
             * be that the device has detected and signalled an error by sending a USB STALL.  The
             * higher-layer protocol may be able to query the firmware for more details about the error.
             */
            class IUvcExtensionAccess
            {
            public:
                virtual ~IUvcExtensionAccess() = default;

                /**
                 * Which vendor-specific extension this interface is providing access to.
                 */
                virtual royale::usb::config::UvcExtensionType getVendorExtensionType() const = 0;

                /**
                 * Take exclusive access for a sequence of multiple calls to vendorExtensionGet/vendorExtensionSet.
                 */
                virtual std::unique_lock<std::recursive_mutex> lockVendorExtension() = 0;

                /**
                 * The UVC specification documents how to query the size of each control, but does
                 * not document whether or not the size can change, and the different platforms have
                 * different behavior.  There is at least one that queries on every Get or Set, one
                 * that doesn't query at all, and one that queries once and caches the result.
                 *
                 * If this method returns true, then the higher-layer protocol must not use variable
                 * sized controls.
                 */
                virtual bool onlySupportsFixedSize() const = 0;

                /**
                 * This reads a vendor extension property.
                 *
                 * The size of the data buffer is an input parameter, and determines how much data the
                 * IUvcExtensionAccess will try to read.
                 *
                 * \param id which control to read from
                 * \param data where to store the data
                 */
                virtual void vendorExtensionGet (uint16_t id, std::vector<uint8_t> &data) = 0;

                /**
                 * This writes a vendor extension property.
                 *
                 * \param id which control to write to
                 * \param data what to write
                 */
                virtual void vendorExtensionSet (uint16_t id, const std::vector<uint8_t> &data) = 0;
            };
        }
    }
}
