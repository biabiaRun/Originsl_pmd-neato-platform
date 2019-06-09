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

#include <imager/ImagerM2452.hpp>

namespace royale
{
    namespace imager
    {
        class IImagerModeStrategy;

        /**
        * This class is prepared for the M2452 imager and is compatible to the design steps B11/B12.
        * It extends the base class implementation with functions to load and run the
        * AllInOne firmware.
        */
        class ImagerM2452_B1x_AIO : public ImagerM2452
        {
        public:
            IMAGER_EXPORT explicit ImagerM2452_B1x_AIO (const ImagerParameters &params);
            ~ImagerM2452_B1x_AIO();

            std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter() override;

            void initialize() override;
            void startCapture() override;
            void reconfigure (const ImagerUseCaseDefinition &useCase, uint16_t &reconfigIndex) override;

        protected:
            /**
            * Write the RAM firmware to the hardware imager. Does not start the firmware. The firmware bytecode must
            * match the specified expected firmware version, otherwise an exception will be thrown.
            *
            * \param   version     The expected firmware version
            * \param   page1       The first firmware page, must not be empty
            */
            void loadFirmware (const uint32_t version, const std::map<uint16_t, uint16_t> &page1);

            ImagerVerificationStatus verifyFrameRateSettings (const ImagerUseCaseDefinition &useCase,
                    bool rawFrameBasedFrameRate) override;
            ImagerVerificationStatus verifySSCSettings (const ImagerUseCaseDefinition &useCase) final;
            std::map < uint16_t, uint16_t > prepareUseCase (const ImagerUseCaseDefinition &useCase) override;
            void prepareSSCSettings (const uint16_t lutIndex,
                                     const std::vector<uint16_t> &pllCfg,
                                     std::map < uint16_t, uint16_t > &regChanges) final;
            std::vector<MeasurementBlock> createMeasurementBlockList (const ImagerUseCaseDefinition &useCase) const override;
            bool isValidExposureTime (bool enabledMixedMode, size_t overallRawFrameCount, uint32_t exposureTime, uint32_t modFreq) final;
            void shutDownSequencer() override;

        private:
            bool m_currentModeIsMixedMode;

            std::unique_ptr<IImagerModeStrategy> m_strategy;
        };
    }
}
