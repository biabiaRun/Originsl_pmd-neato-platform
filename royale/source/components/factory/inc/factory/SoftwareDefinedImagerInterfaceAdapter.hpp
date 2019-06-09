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

#include <imager/IImagerComponent.hpp>
#include <hal/IImager.hpp>

namespace royale
{
    namespace factory
    {
        /**
         * This class implements an adapter for converting calls to a royale::hal::IImager
         * object to calls to a royale::imager::ISoftwareDefinedImagerComponent object.
         * Please note that different forks of Royale may have different royale::hal::IImager implementations,
         * but as long-term goal there should only be one common version of royale::imager::ISoftwareDefinedImagerComponent
         * shared by all forks of Royale.
         */
        class SoftwareDefinedImagerInterfaceAdapter : public royale::hal::IImager
        {
        public:
            ROYALE_API explicit SoftwareDefinedImagerInterfaceAdapter (std::shared_ptr<royale::imager::ISoftwareDefinedImagerComponent> imager);
            ROYALE_API virtual ~SoftwareDefinedImagerInterfaceAdapter() = default;

            static std::shared_ptr<royale::hal::IImager> createImager (std::shared_ptr<royale::imager::ISoftwareDefinedImagerComponent> imager);

            ROYALE_API std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter() override;
            ROYALE_API std::string getSerialNumber() override;
            ROYALE_API royale::Vector<std::size_t> getMeasurementBlockSizes() const override;
            ROYALE_API void wake() override;
            ROYALE_API void initialize() override;
            ROYALE_API void sleep() override;
            ROYALE_API royale::usecase::VerificationStatus verifyUseCase (
                const royale::usecase::UseCaseDefinition &useCase,
                uint16_t roiCMin,
                uint16_t roiRMin,
                uint16_t flowControlRate) override;
            ROYALE_API void executeUseCase (const royale::usecase::UseCaseDefinition &useCase,
                                            uint16_t roiCMin,
                                            uint16_t roiRMin,
                                            uint16_t flowControlRate) override;
            ROYALE_API void startCapture() override;
            ROYALE_API void reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex) override;
            ROYALE_API void reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex) override;
            ROYALE_API void stopCapture() override;
            ROYALE_API void setExternalTrigger (bool useExternalTrigger) override;
            ROYALE_API void writeRegisters (const royale::Vector<royale::Pair<royale::String, uint64_t>> &registers) override;
            ROYALE_API void readRegisters (royale::Vector<royale::Pair<royale::String, uint64_t>> &registers) override;

        private:
            std::shared_ptr<royale::imager::ISoftwareDefinedImagerComponent> m_imager;
        };
    }
}
