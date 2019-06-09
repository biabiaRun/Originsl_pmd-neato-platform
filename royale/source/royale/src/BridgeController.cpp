/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <memory>

#include <common/events/EventProbedDevicesNotMatched.hpp>
#include <common/RoyaleLogger.hpp>

#include <factory/BridgeController.hpp>
#include <factory/CoreConfigFactory.hpp>
#include <factory/CameraCoreBuilderFactory.hpp>
#include <factory/ICameraCoreBuilder.hpp>
#include <factory/IProcessingParameterMapFactory.hpp>

#include <modules/UsbProbeDataListRoyale.hpp>

#if defined(ROYALE_BRIDGE_AMUNDSEN)
#if defined(ROYALE_BRIDGE_AMUNDSEN_LIBUSB)
#include <usb/enumerator/BusEnumeratorAmundsenLibUsb.hpp>
#elif defined(ROYALE_BRIDGE_AMUNDSEN_CYAPI)
#include <usb/enumerator/BusEnumeratorAmundsenCyApi.hpp>
#else
#error AMUNDSEN_DEFINED_BUT_NO_AMUNDSEN_BRIDGE_IMPLEMENTATION
#endif
#endif

#if defined(ROYALE_BRIDGE_ENCLUSTRA)
#if defined(ROYALE_BRIDGE_ENCLUSTRA_LIBUSB)
#include <usb/enumerator/BusEnumeratorEnclustraLibUsb.hpp>
#elif defined(ROYALE_BRIDGE_ENCLUSTRA_CYAPI)
#include <usb/enumerator/BusEnumeratorEnclustraCyApi.hpp>
#else
#error ENCLUSTRA_DEFINED_BUT_NO_ENCLUSTRA_BRIDGE_IMPLEMENTATION
#endif
#endif

#if defined(ROYALE_BRIDGE_UVC)
#if defined(ROYALE_BRIDGE_UVC_DIRECTSHOW)
#include <usb/enumerator/BusEnumeratorUvcDirectShow.hpp>
#elif defined(ROYALE_BRIDGE_UVC_AMUNDSEN)
#if not defined(ROYALE_BRIDGE_AMUNDSEN)
#error UVC_USING_AMUNDSEN_BUT_AMUNDSEN_NOT_ENABLED
#endif
#elif defined(ROYALE_BRIDGE_UVC_V4L)
#include <usb/enumerator/BusEnumeratorUvcV4l.hpp>
#else
#error UVC_DEFINED_BUT_NO_UVC_BRIDGE_IMPLEMENTATION
#endif
#endif

#include <common/exceptions/CouldNotOpen.hpp>

using namespace royale::usb::config;
using namespace royale::factory;
using namespace royale::hal;
using namespace royale::common;
using namespace royale::usb::enumerator;
using namespace royale;

using royale::factory::ICameraCoreBuilder;


namespace
{
    // Function object suitable for passing to IBusEnumerator::enumerateDevices
    // which creates the CameraCoreBuilder and adds it to a list.
    class CameraCoreBuilderInserter
    {
    public:
        explicit CameraCoreBuilderInserter (std::vector<std::unique_ptr<ICameraCoreBuilder>> &coreBuilders,
                                            std::shared_ptr<IProcessingParameterMapFactory> &paramFactory) :
            m_coreBuilders (coreBuilders),
            m_paramFactory (paramFactory)
        {
        }


        void operator() (const UsbProbeData &pd, std::unique_ptr<royale::factory::IBridgeFactory> bridgeFactory)
        {
            try
            {
                bridgeFactory->initialize();
                auto moduleConfig = pd.moduleConfigFactory->probeAndCreate (*bridgeFactory);
                if (moduleConfig == nullptr)
                {
                    throw RuntimeError ("Unable to determine moduleConfig");
                }
                std::shared_ptr<const royale::config::ICoreConfig> coreConfig = CoreConfigFactory (*moduleConfig, m_paramFactory) ();
                auto cameraCoreBuilder = CameraCoreBuilderFactory (pd.bridgeType, *moduleConfig) ();
                cameraCoreBuilder->setBridgeFactory (std::move (bridgeFactory));
                cameraCoreBuilder->setConfig (coreConfig,
                                              std::make_shared<const config::ImagerConfig> (moduleConfig->imagerConfig),
                                              moduleConfig->illuminationConfig);
                m_coreBuilders.push_back (std::move (cameraCoreBuilder));
            }
            catch (const Exception &e)
            {
                LOG (DEBUG) << "Unable to configure device: " << e.getTechnicalDescription();
            }
        }

    private:
        std::vector<std::unique_ptr<ICameraCoreBuilder>> &m_coreBuilders;
        std::shared_ptr<IProcessingParameterMapFactory> m_paramFactory;
    };

    /**
     * The BridgeAmundsen implementation may also be used with UVC devices; however on some operating systems the
     * system will give the system UVC stack ownership of the device, and it won't be possible to
     * use the BridgeAmundsen.
     *
     * Therefore this function may be called with both probeListAmundsen and probeListUVC.
     */
#if defined(ROYALE_BRIDGE_AMUNDSEN)
#if defined(TARGET_PLATFORM_ANDROID)
    void probeAmundsenDevices (
        CameraCoreBuilderInserter &appendCoreBuilder,
        uint32_t androidUsbDeviceFD,
        uint32_t androidUsbDeviceVid,
        uint32_t androidUsbDevicePid,
        UsbProbeDataList &probeListAmundsen)
#else
    void probeAmundsenDevices (
        CameraCoreBuilderInserter &appendCoreBuilder,
        UsbProbeDataList &probeListAmundsen)
#endif
    {
#if defined(ROYALE_BRIDGE_AMUNDSEN_LIBUSB)
#if defined(TARGET_PLATFORM_ANDROID)
        BusEnumeratorAmundsenLibUsb (probeListAmundsen).enumerateDevices (appendCoreBuilder, androidUsbDeviceFD,
                androidUsbDeviceVid, androidUsbDevicePid);
#else
        BusEnumeratorAmundsenLibUsb (probeListAmundsen).enumerateDevices (appendCoreBuilder);
#endif
#endif
#if defined(ROYALE_BRIDGE_AMUNDSEN_CYAPI)
        BusEnumeratorAmundsenCyApi (probeListAmundsen).enumerateDevices (appendCoreBuilder);
#endif
    }
#endif
} // anonymous namespace

BridgeController::BridgeController() :
    m_probeData (royale::config::getUsbProbeDataRoyale()),
    m_paramFactory (royale::config::getProcessingParameterMapFactoryRoyale())
{
}

BridgeController::BridgeController (const royale::usb::config::UsbProbeDataList &probeData,
                                    const std::shared_ptr<IProcessingParameterMapFactory> &paramFactory) :
    m_probeData (probeData),
    m_paramFactory (paramFactory)
{
}

#if defined(TARGET_PLATFORM_ANDROID)
std::vector<std::unique_ptr<ICameraCoreBuilder>> BridgeController::probeDevices (uint32_t androidUsbDeviceFD,
        uint32_t androidUsbDeviceVid,
        uint32_t androidUsbDevicePid)
#else
std::vector<std::unique_ptr<ICameraCoreBuilder>> BridgeController::probeDevices()
#endif
{
    std::vector<std::unique_ptr<ICameraCoreBuilder> > deviceList;
    CameraCoreBuilderInserter appendCoreBuilder (deviceList, m_paramFactory);
    royale::device::ProbeResultInfo probeResultInfo;

#if defined(ROYALE_BRIDGE_AMUNDSEN)
    auto probeListAmundsen = filterUsbProbeDataByBridgeType (m_probeData, BridgeType::AMUNDSEN);
#if defined(TARGET_PLATFORM_ANDROID)
    probeAmundsenDevices (appendCoreBuilder, androidUsbDeviceFD, androidUsbDeviceVid,
                          androidUsbDevicePid, probeListAmundsen);
#else
    probeAmundsenDevices (appendCoreBuilder, probeListAmundsen);
#endif
#endif

#if defined(ROYALE_BRIDGE_ENCLUSTRA)
    auto probeListEnclustra = filterUsbProbeDataByBridgeType (m_probeData, BridgeType::ENCLUSTRA);
#if defined(ROYALE_BRIDGE_ENCLUSTRA_LIBUSB)
#if defined(TARGET_PLATFORM_ANDROID)
    BusEnumeratorEnclustraLibUsb (probeListEnclustra).enumerateDevices (appendCoreBuilder, androidUsbDeviceFD,
            androidUsbDeviceVid, androidUsbDevicePid);
#else
    BusEnumeratorEnclustraLibUsb (probeListEnclustra).enumerateDevices (appendCoreBuilder);
#endif
#endif
#if defined(ROYALE_BRIDGE_ENCLUSTRA_CYAPI)
    BusEnumeratorEnclustraCyApi (probeListEnclustra).enumerateDevices (appendCoreBuilder);
#endif
#endif

#if defined(ROYALE_BRIDGE_UVC)
    auto probeListUVC = filterUsbProbeDataByBridgeType (m_probeData, BridgeType::UVC);
#if defined(ROYALE_BRIDGE_UVC_DIRECTSHOW)
    BusEnumeratorUvcDirectShow busEnumeratorUvcDirectShow (probeListUVC);

    busEnumeratorUvcDirectShow.enumerateDevicesWithInfo (appendCoreBuilder, &probeResultInfo);
#endif
#if defined(ROYALE_BRIDGE_UVC_AMUNDSEN)
#if defined(TARGET_PLATFORM_ANDROID)
    probeAmundsenDevices (appendCoreBuilder, androidUsbDeviceFD, androidUsbDeviceVid,
                          androidUsbDevicePid, probeListUVC);
#else
    probeAmundsenDevices (appendCoreBuilder, probeListUVC);
#endif
#endif
#if defined(ROYALE_BRIDGE_UVC_V4L)
    BusEnumeratorUvcV4l (probeListUVC).enumerateDevices (appendCoreBuilder);
#endif
#endif

    if (m_listener && deviceList.empty() &&
            probeResultInfo.devicesWereFound())
    {
        std::unique_ptr<event::EventProbedDevicesNotMatched> event
            = common::makeUnique<event::EventProbedDevicesNotMatched>();

        event->setProbeResultInfo (probeResultInfo);
        m_listener->onEvent (std::move (event));
    }

    return deviceList;
}

void BridgeController::setEventListener (std::shared_ptr<royale::IEventListener> listener)
{
    m_listener = listener;
}
