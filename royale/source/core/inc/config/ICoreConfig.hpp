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

#include <royale/Definitions.hpp>
#include <royale/String.hpp>

#include <config/FrameTransmissionMode.hpp>
#include <config/BandwidthRequirementCategory.hpp>

#include <usecase/UseCase.hpp>

#include <cstdint>

namespace royale
{
    namespace config
    {
        /**
        * Information about the camera hardware. This includes the connected sensors,
        * the imager, the illumination unit, the XOSC and the bridge/imager interconnection.
        */
        class ICoreConfig
        {
        public:
            ROYALE_API virtual ~ICoreConfig() = default;

            /*!
            * Get the lens center that is fixed by design of the camera module (given pixels).
            * This lens center is a rough guess and may be fine-tuned by the calibration routine.
            */
            ROYALE_API virtual void getLensCenterDesign (uint16_t &column, uint16_t &row) const = 0;

            /*!
            * Get the maximal image width which is supported by the camera module
            */
            ROYALE_API virtual uint16_t getMaxImageWidth() const = 0;

            /*!
            * Get the maximal image height which is supported by the camera module
            */
            ROYALE_API virtual uint16_t getMaxImageHeight() const = 0;

            /**
            * Get a list of supported use cases for this camera module. The use case name
            * is also exposed in the API in order to let the API user switch between different
            * use cases. Each derived configuration is responsible to fill in the correct values.
            *
            * @return a const reference to the supported use cases
            */
            ROYALE_API virtual const usecase::UseCaseList &getSupportedUseCases() const = 0;

            /**
            * Gets the type of frame collector which should be used
            */
            ROYALE_API virtual FrameTransmissionMode getFrameTransmissionMode() const = 0;

            /**
            * Gets the camera name as string which will be used to identify known modules
            */
            ROYALE_API virtual royale::String getCameraName() const = 0;

            /**
            * Soft temperature limit for the camera module.
            * If the illumination unit gets hotter than this, the application is notified
            * (so it could react appropriately, e.g. by turning on cooling if available or
            * lowering the frame rate), but capturing is allowed to continue.
            *
            * @return illumination unit temperature soft limit in degrees celsius
            */
            ROYALE_API virtual float getTemperatureLimitSoft() const = 0;

            /**
            * Returns the hard temperature limit for the camera module.
            * If the illumination unit gets hotter than this, capturing is stopped
            * (and illumination turned off) to prevent hardware damage.
            *
            * @return illumination unit temperature hard limit in degrees celsius
            */
            ROYALE_API virtual float getTemperatureLimitHard() const = 0;

            /**
            * The module may not be able to run auto-exposure due to hardware constraints.
            * This method returns the capability if auto-exposure can be used
            */
            ROYALE_API virtual bool isAutoExposureSupported() const = 0;

            /**
            * Get the bandwidth requirement category for the module.
            */
            ROYALE_API virtual BandwidthRequirementCategory getBandwidthRequirementCategory() const = 0;
        };
    }
}
