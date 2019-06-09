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

#include <config/ExternalConfigFileConfig.hpp>
#include <config/IImagerExternalConfig.hpp>
#include <config/ImagerType.hpp>
#include <config/ImageDataTransferType.hpp>
#include <config/ImConnectedTemperatureSensor.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <string>

namespace royale
{
    namespace config
    {
        /**
         * Data container for imager centric parameters
         */
        struct ImagerConfig
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
            enum class Trigger
            {
                I2C,    //!< Use software defined trigger mode via I2C communication
                GPIO13, //!< Use external trigger at GPIO13 of the imager, GPIO13 is the name of the imager silicon input pin
                GPIO14  //!< Use external trigger at GPIO14 of the imager, GPIO14 is the name of the imager silicon input pin
            };

            ROYALE_API ImagerConfig (ImagerType imagerType, uint32_t systemFrequency,
                                     const std::map<uint16_t, uint16_t> &baseConfig,
                                     double interfaceDelay = 0.0f,
                                     ImageDataTransferType imageDataTransferType = ImageDataTransferType::PIF,
                                     Trigger externalTrigger = Trigger::I2C,
                                     ImConnectedTemperatureSensor tempSensor = ImConnectedTemperatureSensor::NONE,
                                     const ExternalConfigFileConfig &externalConfigFileConfig = ExternalConfigFileConfig::empty(),
                                     bool usesInternalCurrentMonitor = true) :
                imagerType (imagerType),
                systemFrequency (systemFrequency),
                baseConfig (baseConfig),
                interfaceDelay (interfaceDelay),
                imageDataTransferType (imageDataTransferType),
                externalTrigger (externalTrigger),
                tempSensor (tempSensor),
                externalConfigFileConfig { externalConfigFileConfig },
                usesInternalCurrentMonitor (usesInternalCurrentMonitor)
            {

            }

            /**
            *  The imager component to be instantiated for this module.
            *  This specifies the hardware (including revision) and firmware combination.
            */
            ImagerType imagerType;

            /**
            *  System frequency (in Hertz) provided by the crystal oscillator connected to the imager.
            *  Most timings on the imager are based on this clock; for exposure related timings
            *  the modulation frequency setting is also relevant.
            */
            uint32_t systemFrequency;

            /**
            *  Const array of register settings for initializing the camera module.
            *  This array contains only tested base configurations. The configuration
            *  is defined by the concrete type of the camera module. This configuration
            *  may contain things like setting GPIO pins of the imager to fit the
            *  camera module. This configuration does not contain any setting that is
            *  related specifically to the imager (e.g. MODPLL frequency setting).
            */
            std::map<uint16_t, uint16_t> baseConfig;

            /**
            * Interface delay time (in seconds).
            *
            * Defines the time the readout of a row/line of the sensor area should be delayed
            * before it is read by the interface. The interface is either PIF or CSI2.
            * According to the imager's development specification this delay must be used if
            * the interface (PIF or CSI2) is reading faster than the sensor line data is written.
            * In general using PIF will not require any interface delay, but it can be used also
            * to introduce line delays for external devices (e.g. TC35874x bridge device).
            * If the CSI2 interface is used, the imager and an external device might
            * require a delay.
            *
            * The calculation of the delay the CSI2 interface of the imager requires can be found
            * in the imager's development specification, the imager's user manual, or the
            * framerate calculation Excel sheet.
            * If it is not clear what the interface delay means or none of the documents for
            * its calculation is available please get in contact with your assigned Infineon
            * application engineer.
            *
            */
            double interfaceDelay;

            /**
            *  Type of of interface used for connection of imager to receiver
            *  (i.e. PIF, MIPI_1LANE, MIPI_2LANE).
            */
            ImageDataTransferType imageDataTransferType;

            /**
            * Defines the external trigger input for start and stop of capturing. I2C and external
            * triggering can be switched in the imager class.
            * The external trigger can either be I2C (if there is no other external trigger),
            * or a GPIO pin capable of being a trigger input. For GPIO triggering the GPIO pad
            * will be configured to input with pull-down when initializing the imager and the
            * GPIO-MUX will be set to external trigger input on start of capture and disabled on stop of
            * capture. For I2C triggering error flags are validated after the start or
            * stop triggers are done.
            * Please keep in mind, that if you choose a GPIO for triggering you might have
            * to adapt the base config for this module!
            */
            Trigger externalTrigger;

            /**
            *  Defines if and which kind of illumination temperature sensor is
            *  connected to the imager
            */
            ImConnectedTemperatureSensor tempSensor;

            /**
            * For devices using a Flash Defined Imager, the configuration required for that imager.
            * Will be nullptr if not used.
            */
            std::shared_ptr<royale::imager::IImagerExternalConfig> externalImagerConfig;

            /**
            * If an externalImagerConfig should be loaded, and it should be loaded from a location
            * other than the storage used for the product id, where to load it from.
            *
            * \todo ROYAL-3538 move this, but that will take more than one PR
            */
            ExternalConfigFileConfig externalConfigFileConfig;

            /**
             *  Defines if this module uses the internal current monitor of the imager (if supported).
             *  If this is set to true some pseudo data interpreters will check the status of the
             *  internal current monitor and will stop the capturing if the current monitor triggers.
             *  For imagers that don't support the internal current monitor this setting will be
             *  ignored.
             */
            bool usesInternalCurrentMonitor;
        };
    }
}
