/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <hal/IBridgeImager.hpp>
#include <config/IImagerExternalConfig.hpp>
#include <imager/ImagerRawFrame.hpp>

#include <map>

namespace royale
{
    namespace imager
    {
        /**
        * This is the enum for selecting the trigger mode, which can either be a software defined trigger
        * or a trigger by an external signal. If the trigger is set the imager will start capturing. If the
        * trigger is released the imager will end capturing after the framerate counter for the last raw
        * frame of a sequence finished counting. Please refer to the concrete imager's development specification
        * for the detailed description of the MTCU/LPFSM state machine.
        * If an external trigger is used, this enum specifies the trigger signal input pad of the imager itself.
        * The GPIO identifiers of this enum matches exactly the name you can find in the imager silicon's documentation,
        * these aren't the same GPIOs as the IGpioAccess interface or GPIOs in custom firmware of USB controllers.
        */
        enum class ImgTrigger : uint16_t
        {
            I2C = IMG_ENUM, //!< Use software defined trigger mode via I2C communication
            GPIO13,         //!< Use external trigger at GPIO13 of the imager, GPIO13 is the name of the imager silicon input pin
            GPIO14          //!< Use external trigger at GPIO14 of the imager, GPIO14 is the name of the imager silicon input pin
        };

        /**
        * Defines the type of interface that is used to connect the imager to a receiver module.
        */
        enum class ImgImageDataTransferType : uint16_t
        {

            PIF = IMG_ENUM, //!< Parallel Interface
            MIPI_1LANE,     //!< CSI2 MIPI 1 Lane
            MIPI_2LANE      //!< CSI2 MIPI 2 Lanes
        };

        /**
        * Illumination pad configuration used by the imager to enable the desired pad and adust the duty cycle configuration to it.
        */
        enum class ImgIlluminationPad : uint16_t
        {
            SE_P = IMG_ENUM, //!< The illumination circuit is connected to the imager pad labeled as 'MODSEP'.
            SE_N,            //!< The illumination circuit is connected to the imager pad labeled as 'MODSEN'.
            LVDS             //!< The illumination circuit is connected to the imager pads labeled as 'MODLVDSN' and 'MODLVDSP'.
        };

        /**
        * Struct for passing parameters to an imager's constructor.
        * All parameters are const to avoid default initialization.
        */
        struct ImagerParameters
        {
            const std::shared_ptr<royale::hal::IBridgeImager> bridge;
            const std::shared_ptr<const IImagerExternalConfig> externalConfig;
            const bool useSuperframe;
            const bool ntcSensorUsed;
            const ImgTrigger externalTrigger;
            const ImgImageDataTransferType imageDataTransferType;
            const double interfaceDelay;
            const std::map<uint16_t, uint16_t> baseConfig;
            const uint32_t systemFrequency;
            const ImagerRawFrame::ImagerDutyCycle dutyCycle;
            const ImgIlluminationPad illuminationPad;
            const uint32_t maxModulationFrequency;
            const bool usesInternalCurrentMonitor;
        };
    }
}
