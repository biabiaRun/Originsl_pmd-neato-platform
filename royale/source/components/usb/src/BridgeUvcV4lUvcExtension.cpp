/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <royalev4l/bridge/BridgeV4l.hpp>

#include <usb/bridge/BridgeUvcV4lUvcExtension.hpp>

#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/Disconnected.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/Timeout.hpp>

#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/PossiblyUsbStallError.hpp>

#include <common/MakeUnique.hpp>

#include <common/NarrowCast.hpp>
#include <common/RoyaleLogger.hpp>

#include <algorithm>
#include <array>
#include <atomic>

#include <linux/videodev2.h>
#include <linux/uvcvideo.h>
#include <linux/usb/video.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>


using namespace royale::common;
using namespace royale::usb::config;
using namespace royale::usb::bridge;
using namespace royale::v4l::bridge;
using royale::common::makeUnique;
using std::size_t;

namespace
{
    /**
     * The UVC protocol identifies vendor extensions by UUID, and Arctic could be detected by its
     * id, 559596A5-9D86-4F62-BF52-ACBFC251F6CC.  However, this implementation only needs to support
     * Arctic, and for this implementation the code needed to interpret the UVC USB descriptor is
     * omitted.  The Amundsen protocol has already defined that Arctic will always use this unit id.
     */
    const auto VENDOR_EXTENSION_UNIT = uint8_t
                                       {
                                           3
                                       };
    const auto VENDOR_EXTENSION_TYPE = royale::usb::config::UvcExtensionType::Arctic;

}

BridgeUvcV4lUvcExtension::BridgeUvcV4lUvcExtension (const std::shared_ptr<BridgeV4l> &bridge) :
    m_bridge {bridge},
    m_vendorExtensionUnit {VENDOR_EXTENSION_UNIT},
    m_vendorExtensionType {VENDOR_EXTENSION_TYPE}
{
}

BridgeUvcV4lUvcExtension::~BridgeUvcV4lUvcExtension ()
{
    // it doesn't own the file descriptor, the BridgeV4l does
}

royale::usb::config::UvcExtensionType BridgeUvcV4lUvcExtension::getVendorExtensionType() const
{
    return m_vendorExtensionType;
}

std::unique_lock<std::recursive_mutex> BridgeUvcV4lUvcExtension::lockVendorExtension()
{
    std::unique_lock<std::recursive_mutex> lock (m_controlLock);
    return lock;
}

bool BridgeUvcV4lUvcExtension::onlySupportsFixedSize() const
{
    return true;
}

void BridgeUvcV4lUvcExtension::vendorExtensionGet (uint16_t id, std::vector<uint8_t> &data)
{
    struct uvc_xu_control_query query = {0};
    query.unit = m_vendorExtensionUnit;
    query.selector = narrow_cast<uint8_t> (id);
    query.query = UVC_GET_CUR;
    query.size = narrow_cast<uint16_t> (data.size());
    query.data = data.data();

    auto ret = ioctl (m_bridge->getDeviceHandle()->fd, UVCIOC_CTRL_QUERY, &query);
    if (ret == -1)
    {
        if (errno == EIO)
        {
            // Although sending USB stalls are Arctic's standard error handling, with the kernel
            // UVC driver every stall triggers a line in the dmesg / syslog.
            // "Failed to query (GET_CUR) UVC control 2 on unit 3: -32 (exp. 64)."
            throw PossiblyUsbStallError();
        }
        else
        {
            throw RuntimeError ("Data transfer failed (status)");
        }
    }
}

void BridgeUvcV4lUvcExtension::vendorExtensionSet (uint16_t id, const std::vector<uint8_t> &data)
{
    struct uvc_xu_control_query query = {0};
    query.unit = m_vendorExtensionUnit;
    query.selector = narrow_cast<uint8_t> (id);
    query.query = UVC_SET_CUR;
    query.size = narrow_cast<uint16_t> (data.size());
    query.data = const_cast<uint8_t *> (data.data());

    auto ret = ioctl (m_bridge->getDeviceHandle()->fd, UVCIOC_CTRL_QUERY, &query);
    if (ret == -1)
    {
        if (errno == EIO)
        {
            throw PossiblyUsbStallError();
        }
        else
        {
            throw RuntimeError ("Data transfer failed (status)");
        }
    }
}
