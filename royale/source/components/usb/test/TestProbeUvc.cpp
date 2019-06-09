
#if defined(TEST_UVC)
#if defined(ROYALE_BRIDGE_UVC_DIRECTSHOW)
#include <usb/enumerator/BusEnumeratorUvcDirectShow.hpp>
#include <Windows.h>
#elif defined(ROYALE_BRIDGE_UVC_AMUNDSEN) and defined(ROYALE_BRIDGE_AMUNDSEN_CYAPI)
#include <usb/enumerator/BusEnumeratorAmundsenCyApi.hpp>
#include <Windows.h>
#elif defined(ROYALE_BRIDGE_UVC_AMUNDSEN) and defined(ROYALE_BRIDGE_AMUNDSEN_LIBUSB)
#include <usb/enumerator/BusEnumeratorAmundsenLibUsb.hpp>
#elif defined(TEST_UVC_V4L)
#include <usb/enumerator/BusEnumeratorUvcV4l.hpp>
#else
#error TEST_UVC_DEFINED_BUT_NO_UVC_BRIDGE_IMPLEMENTATION
#endif
#endif

#include <stdexcept>

#include <PlatformResources.hpp>

#include <memory>
#include <gtest/gtest.h>

class TestProbeUvc : public ::testing::Test
{
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

#if defined(TEST_UVC)

using namespace royale::usb::enumerator;
using namespace royale::usb::config;
using namespace royale::config;

namespace   // anonymous
{
    class BadModuleConfigFactory : public royale::factory::IModuleConfigFactory
    {
    public:
        std::shared_ptr<const royale::config::ModuleConfig>
        probeAndCreate (royale::factory::IBridgeFactory &) const override
        {
            throw std::logic_error ("not supposed to happen");
        }

        royale::Vector<std::shared_ptr<const royale::config::ModuleConfig>> enumerateConfigs() const override
        {
            return {};
        }
    };
}

TEST_F (TestProbeUvc, probeNonExistent)
{
#if defined(ROYALE_BRIDGE_UVC_DIRECTSHOW)
    sample_utils::PlatformResources resources;
#endif

    auto badModuleConfigFactory = std::make_shared<BadModuleConfigFactory>();

    // Probe data with VID/PID=0/0, which shouldn't match anything on the bus.
    UsbProbeData customProbeData{ 0, 0, BridgeType::UVC, badModuleConfigFactory };

#if defined(ROYALE_BRIDGE_UVC_DIRECTSHOW)
    BusEnumeratorUvcDirectShow probe ({ customProbeData });
#elif defined(ROYALE_BRIDGE_UVC_AMUNDSEN) and defined(ROYALE_BRIDGE_AMUNDSEN_CYAPI)
    BusEnumeratorAmundsenCyApi probe ({ customProbeData });
#elif defined(ROYALE_BRIDGE_UVC_AMUNDSEN) and defined(ROYALE_BRIDGE_AMUNDSEN_LIBUSB)
    BusEnumeratorAmundsenLibUsb probe ({customProbeData});
#elif defined(TEST_UVC_V4L)
    BusEnumeratorUvcV4l probe ({customProbeData});
#else
#error TEST_UVC_DEFINED_BUT_NO_UVC_BRIDGE_IMPLEMENTATION
#endif

    ASSERT_NO_THROW (probe.enumerateDevices ([] (const UsbProbeData &, std::unique_ptr<royale::factory::IBridgeFactory>)
    {
        throw std::logic_error ("not supposed to happen");
    }));
}
#endif
