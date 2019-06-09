/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <modules/UsbProbeDataListRoyale.hpp>
#include <factory/ModuleConfigFactoryFixed.hpp>
#include <factory/ProcessingParameterMapFactory.hpp>
#include <modules/ModuleConfigData.hpp>

#include <modules/ModuleConfigFactoryAnimatorBoard.hpp>
#include <modules/ModuleConfigFactoryOrpheus.hpp>
#include <modules/ModuleConfigFactoryPicoFlexx.hpp>
#include <modules/ModuleConfigFactoryPicoMonstar.hpp>
#include <modules/ModuleConfigFactoryPmdModule238x.hpp>
#include <modules/ModuleConfigFactoryPmdModule277x.hpp>
#include <modules/ModuleConfigFactorySalome.hpp>
#include <modules/ModuleConfigFactorySelene.hpp>

#include <modules/CommonProcessingParameters.hpp>
#include <modules/ModuleConfigPicoMonstarCommon.hpp>
#include <modules/ModuleConfigOrpheusCommon.hpp>

using namespace royale::usb::config;
using namespace royale::config;
using namespace royale::factory;

namespace
{
    auto factoryAnimatorBoard = std::make_shared<ModuleConfigFactoryAnimatorBoard> ();
}

static const UsbProbeDataList usbProbeDataRoyale
{
    { 0x1c28, 0xc012, BridgeType::ENCLUSTRA, std::make_shared<ModuleConfigFactoryPicoFlexx>()                                },
    { 0x058b, 0x00a1, BridgeType::AMUNDSEN,  factoryAnimatorBoard                                                            },
    { 0x058b, 0x00a6, BridgeType::UVC,       factoryAnimatorBoard                                                            },
    { 0x058b, 0x00a7, BridgeType::UVC,       factoryAnimatorBoard                                                            },
    { 0x1c28, 0xC018, BridgeType::UVC,       std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstar1)         },
    { 0x1c28, 0xC019, BridgeType::UVC,       std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstar2)         },
    { 0x1c28, 0xC025, BridgeType::AMUNDSEN,  std::make_shared<ModuleConfigFactoryPicoMonstar>()                              },
    { 0x1c28, 0xc028, BridgeType::AMUNDSEN,  std::make_shared<ModuleConfigFactorySalome>()                                   },
    { 0x1c28, 0xc029, BridgeType::UVC,       std::make_shared<ModuleConfigFactorySalome>()                                   },
    { 0x1c28, 0xc02D, BridgeType::AMUNDSEN,  std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::X18502W)              },
    { 0x1c28, 0xc031, BridgeType::AMUNDSEN,  std::make_shared<ModuleConfigFactorySelene>()                                   },
    { 0x1c28, 0xc032, BridgeType::AMUNDSEN,  std::make_shared<ModuleConfigFactoryOrpheus>()                                  },
    { 0x1c28, 0xc033, BridgeType::AMUNDSEN,  std::make_shared<ModuleConfigFactoryPmdModule238x>()                            },
    { 0x1c28, 0xc036, BridgeType::AMUNDSEN,  std::make_shared<ModuleConfigFactoryPmdModule277x>()                            },
};

royale::usb::config::UsbProbeDataList royale::config::getUsbProbeDataRoyale()
{
    // makes a deep copy
    return usbProbeDataRoyale;
}

std::shared_ptr<royale::factory::IProcessingParameterMapFactory> royale::config::getProcessingParameterMapFactoryRoyale()
{
    using namespace royale;
    using namespace royale::moduleconfig;
    const std::vector<uint8_t> emptyProductId {};
    const royale::Vector<ProcessingParameterMapFactory::value_type> defaultVector
    {
        {{emptyProductId, CommonId2Frequencies}, {CommonProcessingParams2Frequencies}},
        {{emptyProductId, CommonId1Frequency}, {CommonProcessingParams1Frequency}},
        {{emptyProductId, CommonIdMixedEsHt}, {CommonProcessingParams2Frequencies, CommonProcessingParams1Frequency}},
        {{emptyProductId, CommonIdMixedHtEs }, { CommonProcessingParams1Frequency, CommonProcessingParams2Frequencies}},
        {{emptyProductId, picomonstar::ProcessingParamsMonstarGlass5fps }, { picomonstar::ProcessingParams5fpsGlass}},
        {{emptyProductId, picomonstar::ProcessingParamsMonstarGlass10fps }, { picomonstar::ProcessingParams10fpsGlass}},

        { { emptyProductId, CommonIdLowNoiseExtended }, { CommonProcessingParamsLowNoiseExtended } },
        { { emptyProductId, CommonIdVideoExtended }, { CommonProcessingParamsVideoExtended } },
        { { emptyProductId, CommonIdVideoHalf }, { CommonProcessingParamsVideoHalf } },
        { { emptyProductId, CommonIdVideo }, { CommonProcessingParamsVideo } },
        { { emptyProductId, CommonIdFastAcquisition }, { CommonProcessingParamsFastAcquisition } },
        { { emptyProductId, CommonIdVeryFastAcquisition }, { CommonProcessingParamsVeryFastAcquisition } },
    };
    return std::make_shared<ProcessingParameterMapFactory> (defaultVector, CommonId2Frequencies);
}
