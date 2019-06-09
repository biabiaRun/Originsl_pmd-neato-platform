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

#include <imager/ImagerM2450.hpp>
#include <memory>
#include <mutex>

namespace royale
{
    namespace imager
    {
        class IImagerModeStrategy;

        /**
        * This class is prepared for the M2450 imager and is compatible to the design step A12.
        * It extends the plain M2450 A12 by loading the AllInOne firmware.
        * Please not that the AllInOne firmware offers two different modes, the mixed and the
        * normal mode. For each mode a different number of sequence entries is available.
        *
        * The implementation covers a 133MHz system clock (=400MHz DPHY frequency).
        * Different system frequencies (XTAL) are supported, when initializing this
        * imager a calculation of the DPHY PLL is done that ensure that the system
        * clock stays at 133Mhz.
        * The implementation only supports phase shifts of multiples of 45 degrees.
        * Although a complete range of system frequencies is possible to set it has
        * to be considered that only 24MHz and 26MHz are fully tested. Other system
        * frequencies should be set for experimental use only.
        * Only individual frame mode is supported by this class.
        */
        class ImagerM2450_A12_AIO : public ImagerM2450
        {
        public:
            IMAGER_EXPORT explicit ImagerM2450_A12_AIO (const ImagerParameters &params);
            ~ImagerM2450_A12_AIO();

            virtual std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter() override;

            void initialize() override;
            void startCapture() override;
            void stopCapture() override;
            void reconfigure (const ImagerUseCaseDefinition &useCase, uint16_t &reconfigIndex) override;

        protected:
            std::vector < uint16_t > getSerialRegisters() override;
            ImagerVerificationStatus verifyRegion (const ImagerUseCaseDefinition &useCase) override;
            std::map < uint16_t, uint16_t > prepareUseCase (const ImagerUseCaseDefinition &useCase) override;
            void getReadoutDelays (double &ifdel, double &lblank, double &cycAdcSocd, double &cycAdcOddd) const override;
            ImagerVerificationStatus verifySSCSettings (const ImagerUseCaseDefinition &useCase) final;
            void prepareSSCSettings (const uint16_t lutIndex,
                                     const std::vector<uint16_t> &pllCfg,
                                     std::map < uint16_t, uint16_t > &regChanges) final;
            void prepareMeasurementBlockTargetTime (MeasurementBlockId mbId,
                                                    const double mbTargetTime,
                                                    const double mbMeasurementTime) override;
            std::vector<MeasurementBlock> createMeasurementBlockList (const ImagerUseCaseDefinition &useCase) const override;
            bool isValidExposureTime (bool enabledMixedMode, size_t overallRawFrameCount, uint32_t exposureTime, uint32_t modFreq) final;
            void shutDownSequencer() override;

        private:
            /**
            * Write the RAM firmware to the hardware imager. Does not start the firmware.  For
            * M2450, Royale only supports writing one page.  The firmware bytecode must match the
            * specified expected firmware version, otherwise an exception will be thrown.
            *
            * \param   version     The expected firmware version
            * \param   page1       The first firmware page, page switch is done only if not empty
            * \param   page2       The second firmware page, optional.
            */
            void loadFirmware (const uint32_t version,
                               const std::map<uint16_t, uint16_t> &page1,
                               const std::map<uint16_t, uint16_t> &page2 = {});

            void startFirmware();
            void checkPostStartStatus (bool extTriggerUsed = false);

            std::mutex m_reconfigLock;
            bool m_currentModeIsMixedMode;

            bool m_useSuperFrame;

            std::unique_ptr<IImagerModeStrategy> m_strategy;
        };
    }
}
