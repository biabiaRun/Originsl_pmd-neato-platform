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
#include <imager/ImagerRegisterAccess.hpp>
#include <hal/IImager.hpp>

namespace royale
{
    namespace factory
    {
        /**
         * This class implements an adapter for converting calls to a royale::hal::IImager
         * object to calls to a royale::imager::IFlashDefinedImagerComponent object.
         * Please note that different forks of Royale may have different royale::hal::IImager implementations,
         * but as long-term goal there should only be one common version of royale::imager::IFlashDefinedImagerComponent
         * shared by all forks of Royale.
         *
         * Please not that until \todo ROYAL-2874 is resolved a work-around is used that loads
         * the use case from an external configuration by using the royale::UseCaseDefinition::getName()
         * to retrieve the use case identifier. The module config has to provide a correct initialization
         * of the use case name, which is expected to match one of the use case identifiers from the
         * Salome_M2453.lena external configuration file.
         */
        class FlashDefinedImagerInterfaceAdapter : public royale::hal::IImager
        {
        public:
            ROYALE_API explicit FlashDefinedImagerInterfaceAdapter (std::shared_ptr<royale::imager::IFlashDefinedImagerComponent> imager,
                    std::shared_ptr<royale::imager::ImagerRegisterAccess> directAccess);
            ROYALE_API virtual ~FlashDefinedImagerInterfaceAdapter() = default;

            static std::shared_ptr<royale::hal::IImager> createImager (std::shared_ptr<royale::imager::IFlashDefinedImagerComponent> imager,
                    std::shared_ptr<royale::imager::ImagerRegisterAccess> directAccess);

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
            std::shared_ptr<royale::imager::IFlashDefinedImagerComponent> m_imager;
            std::shared_ptr<royale::imager::ImagerRegisterAccess> m_directAccess;
            royale::usecase::UseCaseDefinition m_executingUcd;
        };
    }
}
