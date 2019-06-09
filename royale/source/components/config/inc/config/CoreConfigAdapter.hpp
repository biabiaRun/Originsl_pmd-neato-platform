/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <config/CoreConfig.hpp>

#include <memory>

namespace royale
{
    namespace config
    {
        /**
         * Adapter class that allows using CoreConfig with code expecting an ICoreConfig interface.
         */
        class CoreConfigAdapter : public ICoreConfig
        {
        public:
            /**
             * This is called by CoreConfigFactory, the object passed must be a CoreConfig that has
             * already had all of its cross-references resolved (so the use cases' image sizes are
             * correctly set to match the imager, etc).
             */
            ROYALE_API explicit CoreConfigAdapter (std::unique_ptr<CoreConfig> config);

            /**
             * Get the lens center that is fixed by design of the camera module (given pixels).
             * This lens center is a rough guess and may be fine-tuned by the calibration routine.
             */
            ROYALE_API void getLensCenterDesign (uint16_t &column, uint16_t &row) const override;

            /**
             * Get the maximal image width which is supported by the camera module
             */
            ROYALE_API uint16_t getMaxImageWidth() const override;

            /**
             * Get the maximal image height which is supported by the camera module
             */
            ROYALE_API uint16_t getMaxImageHeight() const override;

            /**
             * Get a list of supported use cases for this camera module. The use case name
             * is also exposed in the API in order to let the API user switch between different
             * use cases. Each derived configuration is responsible to fill in the correct values.
             *
             * @return a const reference to the supported use cases
             */
            ROYALE_API const usecase::UseCaseList &getSupportedUseCases() const override;

            /**
             * Gets the type of frame collector which should be used
             */
            ROYALE_API FrameTransmissionMode getFrameTransmissionMode() const override;

            /**
             * Gets the camera name as string which can be arbitrarily set from outside
             */
            ROYALE_API royale::String getCameraName() const override;

            /**
             * Soft temperature limit for the camera module.
             * If the illumination unit gets hotter than this, the application is notified
             * (so it could react appropriately, e.g. by turning on cooling if available or
             * lowering the frame rate), but capturing is allowed to continue.
             *
             * @return illumination unit temperature soft limit in degrees celsius
             */
            ROYALE_API float getTemperatureLimitSoft() const override;

            /**
             * Returns the hard temperature limit for the camera module.
             * If the illumination unit gets hotter than this, capturing is stopped
             * (and illumination turned off) to prevent hardware damage.
             *
             * @return illumination unit temperature hard limit in degrees celsius
             */
            ROYALE_API float getTemperatureLimitHard() const override;

            /**
             * The module may not be able to run auto-exposure due to hardware constraints.
             * This method returns the capability if auto-exposure can be used
             */
            ROYALE_API bool isAutoExposureSupported() const override;

            /**
             * Get the bandwidth requirement category for the module.
             */
            ROYALE_API BandwidthRequirementCategory getBandwidthRequirementCategory() const override;

        private:
            std::unique_ptr<CoreConfig> m_config;
        };
    }
}
