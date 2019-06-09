/****************************************************************************\
 * Copyright (C) 2019 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <usb/enumerator/BusEnumeratorUvcV4l.hpp>
#include <usb/factory/BridgeFactoryUvcV4l.hpp>
#include <usb/config/UsbProbeData.hpp>

#include <common/exceptions/CouldNotOpen.hpp>

#include <common/MakeUnique.hpp>
#include <common/RoyaleLogger.hpp>

#include <algorithm>
#include <iomanip>
#include <limits>
#include <string>

#include <linux/version.h>
#include <linux/videodev2.h>

#include <libudev.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace royale::common;
using namespace royale::usb::enumerator;
using namespace royale::usb::config;
using namespace royale::config;
using namespace royale::usb::bridge;
using std::size_t;

namespace
{
    /**
     * Simple RAII wrapper for a filehandle, without automatically opening streams as <fstream> does
     */
    class WrappedFD
    {
    public:
        WrappedFD (const std::string &filename) :
            fd {open (filename.c_str(), O_RDWR) }
        {
            if (!isConnected())
            {
                throw CouldNotOpen();
            }
        }

        WrappedFD (WrappedFD &&a) :
            fd (-1)
        {
            std::swap (a.fd, fd);
        }

        WrappedFD (const WrappedFD &) = delete;

        bool isConnected() const
        {
            return fd >= 0;
        }

        int get()
        {
            return fd;
        }

        ~WrappedFD()
        {
            if (isConnected())
            {
                LOG (DEBUG) << "Closing fd " << fd;
                close (fd);
            }
        }

    private:
        int fd;
    };

    /**
     * Deleter functions for std::unique_ptrs to udev objects.
     */
    struct UdevDeleters
    {
        void operator() (udev *x) const
        {
            udev_unref (x);
        }

        void operator() (udev_enumerate *x) const
        {
            udev_enumerate_unref (x);
        }

        void operator() (udev_device *x) const
        {
            udev_device_unref (x);
        }
    };
}

BusEnumeratorUvcV4l::BusEnumeratorUvcV4l (const UsbProbeDataList &probeData) :
    m_probeData (probeData)
{
}

BusEnumeratorUvcV4l::~BusEnumeratorUvcV4l() = default;

void BusEnumeratorUvcV4l::enumerateDevices (std::function<void (const UsbProbeData &, std::unique_ptr<royale::factory::IBridgeFactory>) > callback)
{
    auto udevContext = std::unique_ptr<udev, UdevDeleters> { udev_new() };
    if (!udevContext)
    {
        LOG (ERROR) << "Could not create udev context";
        return;
    }

    auto udevEnumerate = std::unique_ptr<udev_enumerate, UdevDeleters> { udev_enumerate_new (udevContext.get()) };
    if (!udevEnumerate)
    {
        LOG (ERROR) << "Could not enumerate udev devices";
        return;
    }

    if (0 > udev_enumerate_add_match_subsystem (udevEnumerate.get(), "video4linux"))
    {
        LOG (ERROR) << "Failed to set udev match rule for subsystem";
        return;
    }

    if (0 > udev_enumerate_scan_devices (udevEnumerate.get()))
    {
        LOG (ERROR) << "Failed to scan udev devices";
        return;
    }

    auto devices = udev_enumerate_get_list_entry (udevEnumerate.get());
    struct udev_list_entry *entry;

    udev_list_entry_foreach (entry, devices)
    {
        try
        {
            const auto syspath = udev_list_entry_get_name (entry);
            auto device = std::unique_ptr<udev_device, UdevDeleters> {udev_device_new_from_syspath (udevContext.get(), syspath) };
            if (!device)
            {
                continue;
            }

            // usb is just a raw pointer, this udev function doesn't add a refcount
            auto usb = udev_device_get_parent_with_subsystem_devtype (device.get(), "usb", "usb_device");
            if (!usb)
            {
                continue;
            }

            const auto idVendor = udev_device_get_sysattr_value (usb, "idVendor");
            const auto idProduct = udev_device_get_sysattr_value (usb, "idProduct");

            uint16_t vid;
            std::istringstream (idVendor) >> std::hex >> vid;
            uint16_t pid;
            std::istringstream (idProduct) >> std::hex >> pid;

            for (auto &pd : m_probeData)
            {
                if ( (vid == pd.vid) && (pid == pd.pid))
                {
                    LOG (DEBUG) << "Found a possible device, USB ID "
                                << std::setfill ('0') << std::setw (4) << std::hex << pd.vid << ":"
                                << std::setfill ('0') << std::setw (4) << std::hex << pd.pid;
                    const auto filename = udev_device_get_devnode (device.get());
                    probeDevice (callback, pd, filename);
                }
            }
        }
        catch (Exception &)
        {
            // just continue to the next device
        }
    }
}

void BusEnumeratorUvcV4l::probeDevice (std::function<void (const UsbProbeData &, std::unique_ptr<royale::factory::IBridgeFactory>) > callback, const UsbProbeData &pd, const std::string &filename)
{
    LOG (DEBUG) << "Trying to open " << filename << " ... ";
    auto deviceHandle = WrappedFD (filename);

    struct v4l2_capability caps;
    auto err = ioctl (deviceHandle.get(), VIDIOC_QUERYCAP, &caps);
    if (err)
    {
        LOG (ERROR) << "Failed to probe V4L2 capabilities for " << filename << ", error " << errno;
        throw CouldNotOpen();
    }

    // On Linux 4.16 and above, two /dev/video devices are created for each UVC camera, one
    // of which supports metadata capture but not video capture. The documentation allows a
    // /dev/video file to support both metadata and video capture, but this isn't used by
    // the uvcvideo driver.
    if (! (caps.device_caps & V4L2_CAP_VIDEO_CAPTURE))
    {
        LOG (DEBUG) << "Ignoring " << filename << " as it isn't a video capture device";
        const auto v4l2_cap_meta_capture = __u32 {0x800000};
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)
        static_assert (v4l2_cap_meta_capture == V4L2_CAP_META_CAPTURE, "Wrong value for the meta capture flag");
#endif
        if (caps.device_caps & v4l2_cap_meta_capture)
        {
            LOG (DEBUG) << "... " << filename << " is a metadata device";
        }
        throw RuntimeError ("probed a metadata device");
    }

    LOG (DEBUG) << "Probing found " << filename << " (" << caps.card << ")";

    auto bridgeFactory = common::makeUnique<factory::BridgeFactoryUvcV4l> (filename);
    callback (pd, std::move (bridgeFactory));
}
