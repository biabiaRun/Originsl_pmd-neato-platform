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

#include <imager/ImagerM2450.hpp>

namespace royale
{
    namespace imager
    {
        /**
        * ImagerM2450_A11
        *
        * This implementation is for the MiraCE A11 and is only valid
        * for the PicoS module running the Enclustra firmware that uses the
        * imager's parallel interface. The implementation only supports phase
        * shifts of multiples of 45 degrees.
        * The implementation loads a default configuration for a 133MHz system clock.
        * Only individual frame mode is supported by this class.
        */
        class ImagerM2450_A11 : public ImagerM2450
        {
        public:
            IMAGER_EXPORT explicit ImagerM2450_A11 (const ImagerParameters &params);
            ~ImagerM2450_A11();

            void initialize() override;
            void startCapture() override;
            void reconfigure (const ImagerUseCaseDefinition &useCase, uint16_t &reconfigIndex) override;
            void stopCapture() override;
            std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter() override;
            ImagerVerificationStatus verifyUseCase (const ImagerUseCaseDefinition &useCase) override;

        protected:
            std::map < uint16_t, uint16_t > prepareUseCase (const ImagerUseCaseDefinition &useCase) override;
            std::vector < uint16_t > getSerialRegisters() override;
            void getReadoutDelays (double &ifdel, double &lblank, double &cycAdcSocd, double &cycAdcOddd) const override;
        };
    }
}
