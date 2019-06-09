/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerParameters.hpp>
#include <config/ICoreConfig.hpp>
#include <config/ImagerConfig.hpp>
#include <config/IlluminationConfig.hpp>
#include <common/exceptions/LogicError.hpp>

#pragma once

using namespace royale::imager;

namespace royale
{
    namespace factory
    {
        /**
         * This class provides a helper method to convert from royale::config specific
         * data structures to the data structure used by the imager component (royale::imager::ImagerParameters).
         * Please note that different forks of Royale may have different adapter implementations,
         * but as long-term goal there should only be one common version of ImagerParameters shared
         * by all forks of Royale.
         */
        class ImagerConfigAdapter
        {
        public:
            virtual ~ImagerConfigAdapter() = default;

            static ImagerParameters createImagerParameters (std::shared_ptr<royale::hal::IBridgeImager> bridge,
                    const std::shared_ptr<const IImagerExternalConfig> externalConfig,
                    const std::shared_ptr<const royale::config::ICoreConfig> &coreConfig,
                    const std::shared_ptr<const royale::config::ImagerConfig> &imagerConfig,
                    const royale::config::IlluminationConfig &illuminationConfig)
            {
                if (bridge == nullptr || coreConfig == nullptr || imagerConfig == nullptr)
                {
                    throw royale::common::LogicError ("nullref exception");
                }

                ImgTrigger trigger;
                ImgImageDataTransferType imageDataTransferType;
                ImgIlluminationPad illuminationPad;

                switch (imagerConfig->externalTrigger)
                {
                    case royale::config::ImagerConfig::Trigger::GPIO13:
                        trigger = ImgTrigger::GPIO13;
                        break;
                    case royale::config::ImagerConfig::Trigger::GPIO14:
                        trigger = ImgTrigger::GPIO14;
                        break;
                    case royale::config::ImagerConfig::Trigger::I2C:
                        trigger = ImgTrigger::I2C;
                        break;
                    default:
                        throw royale::common::LogicError ("unknown trigger type");
                }

                switch (imagerConfig->imageDataTransferType)
                {
                    case royale::config::ImageDataTransferType::MIPI_1LANE:
                        imageDataTransferType = ImgImageDataTransferType::MIPI_1LANE;
                        break;
                    case royale::config::ImageDataTransferType::MIPI_2LANE:
                        imageDataTransferType = ImgImageDataTransferType::MIPI_2LANE;
                        break;
                    case royale::config::ImageDataTransferType::PIF:
                        imageDataTransferType = ImgImageDataTransferType::PIF;
                        break;
                    default:
                        throw royale::common::LogicError ("unknown image data transfer type");
                }

                switch (illuminationConfig.illuminationPad)
                {
                    case royale::config::IlluminationPad::LVDS:
                        illuminationPad = ImgIlluminationPad::LVDS;
                        break;
                    case royale::config::IlluminationPad::SE_N:
                        illuminationPad = ImgIlluminationPad::SE_N;
                        break;
                    case royale::config::IlluminationPad::SE_P:
                        illuminationPad = ImgIlluminationPad::SE_P;
                        break;
                    default:
                        throw royale::common::LogicError ("unknown illumination pad");
                }

                ImagerRawFrame::ImagerDutyCycle dutyCycle;

                switch (illuminationConfig.dutyCycle)
                {
                    case royale::usecase::RawFrameSet::DutyCycle::DC_0:
                        dutyCycle = ImagerRawFrame::ImagerDutyCycle::DC_0;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_100:
                        dutyCycle = ImagerRawFrame::ImagerDutyCycle::DC_100;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_25:
                        dutyCycle = ImagerRawFrame::ImagerDutyCycle::DC_25;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_25_DEPRECATED:
                        dutyCycle = ImagerRawFrame::ImagerDutyCycle::DC_25_DEPRECATED;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_50:
                        dutyCycle = ImagerRawFrame::ImagerDutyCycle::DC_50;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_37_5:
                        dutyCycle = ImagerRawFrame::ImagerDutyCycle::DC_37_5;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_37_5_DEPRECATED:
                        dutyCycle = ImagerRawFrame::ImagerDutyCycle::DC_37_5_DEPRECATED;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_75:
                        dutyCycle = ImagerRawFrame::ImagerDutyCycle::DC_75;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_AUTO:
                        dutyCycle = ImagerRawFrame::ImagerDutyCycle::DC_AUTO;
                        break;
                    default:
                        throw royale::common::LogicError ("unknown duty cycle");
                }

                ImagerParameters params{ std::move (bridge),
                                         std::move (externalConfig),
                                         coreConfig->getFrameTransmissionMode() == royale::config::FrameTransmissionMode::SUPERFRAME,
                                         imagerConfig->tempSensor == royale::config::ImConnectedTemperatureSensor::NTC,
                                         trigger,
                                         imageDataTransferType,
                                         imagerConfig->interfaceDelay,
                                         imagerConfig->baseConfig,
                                         imagerConfig->systemFrequency,
                                         dutyCycle,
                                         illuminationPad,
                                         illuminationConfig.maxModulationFrequency,
                                         imagerConfig->usesInternalCurrentMonitor
                                       };

                return params;
            }
        };
    }
}
