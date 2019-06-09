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

#include <imager/ImagerBase.hpp>
#include <imager/ImagerParameters.hpp>

#include <vector>
#include <string>

namespace royale
{
    namespace imager
    {
        /**
        * This is a dummy implementation for bring-up of the M2453 imager.
        * It is not yet decided if and when to remove class.
        */
        class ImagerMXXXX_Dummy : public IFlashDefinedImagerComponent
        {
        public:
            /**
            * The lifecycle of the bridge and config object must be longer than the Imager.
            * This imager uses an external configuration, thus the externalConfig member of
            * the ImagerParameters parameter must point to a valid external configuration.
            *
            * Please note that this dummy implementation only executes a single use case
            * (the first use case from the external configuration).
            *
            * \param   params   Set of parameters that have to be passed to a concrete imager object.
            */
            IMAGER_EXPORT explicit ImagerMXXXX_Dummy (const ImagerParameters &params);
            ~ImagerMXXXX_Dummy() override;

            void initialize() override;
            void sleep() override;
            void wake() override;
            std::vector<std::size_t> getMeasurementBlockSizes() const override;
            std::string getSerialNumber() override;
            void startCapture() override;
            void reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex) override;
            void reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex) override;
            uint16_t stopCapture() override;
            std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter() override;
            void setLoggingListener (IImageSensorLoggingListener *pListener) override;
            void setExternalTrigger (bool useExternalTrigger) override;
            ImagerVerificationStatus verifyUseCase (const ImagerUseCaseIdentifier &useCaseIdentifier) override;
            void executeUseCase (const ImagerUseCaseIdentifier &useCaseIdentifier) override;

        private:
            std::shared_ptr<royale::hal::IBridgeImager> m_bridge;
            std::shared_ptr<const IImagerExternalConfig> m_externalConfig;
        };
    }
}
