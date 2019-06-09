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

#include <imager/ImagerBase.hpp>

namespace royale
{
    namespace imager
    {
        /**
        * This is an empty imager implementation that can be fully customized via
        * external calls to write registers. This implementation allows full freedom
        * at the cost of having no tests in place. When calling the initialize method
        * a reset of the hardware imager will be done. A call to the sleep function
        * will keep the hardware imager in power-down state. Any other method has an empty
        * implementation and it is up to the user of this class to provide the certain
        * functionality using the right combination of manual register writes and
        * calls to the royalecore camera device class.
        */
        class ImagerEmpty : public ImagerBase
        {
        public:

            /**
            * Imagers ctor.
            * The lifecycle of the bridge and config object must be longer than the Imager.
            *
            * \param bridge  The bridge interface that should be used by the imager for communication.
            * \param pdi     Pseudo-data interpreter for the (actual) imager.
            */
            IMAGER_EXPORT ImagerEmpty (std::shared_ptr<royale::hal::IBridgeImager> bridge,
                                       std::unique_ptr<common::IPseudoDataInterpreter> pdi);
            ~ImagerEmpty();

            void initialize() override;
            void sleep() override;
            void wake() override;
            ImagerVerificationStatus verifyUseCase (const ImagerUseCaseDefinition &useCase) override;
            void executeUseCase (const ImagerUseCaseDefinition &useCase) override;
            std::vector<std::size_t> getMeasurementBlockSizes() const override;
            std::string getSerialNumber() override;
            void startCapture() override;
            void reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex) override;
            void reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex) override;
            void stopCapture() override;
            std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter() override;
            void writeRegisters (const std::vector<uint16_t> &registerAddresses,
                                 const std::vector<uint16_t> &registerValues) override;
            void readRegisters (const std::vector<uint16_t> &registerAddresses,
                                std::vector<uint16_t> &registerValues) override;
            void setExternalTrigger (bool useExternalTrigger) override;

        protected:
            std::unique_ptr<common::IPseudoDataInterpreter> m_pdi;

            std::vector < uint16_t > getSerialRegisters() override;
        };
    }
}
