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

#include <imager/Imager.hpp>
#include <imager/IPllStrategy.hpp>
#include <array>
#include <iomanip>
#include <mutex>

namespace royale
{
    namespace imager
    {
        struct ImagerM2452Serial
        {
            std::string  serialNumber;
            const uint16_t i2c_addr_sel_1;
            const uint16_t i2c_addr_sel_2;
            const uint16_t touch_down_counter;
            const uint16_t temp_sens_cal;
            const uint16_t hamming_code;
            const uint16_t week;
            const uint16_t year;
            const uint16_t waver;
            const uint16_t lot;
            const uint16_t fab;
            const uint16_t y_coor;
            const uint16_t x_coor;

            ImagerM2452Serial (uint16_t efuseval1, uint16_t efuseval2, uint16_t efuseval3, uint16_t efuseval4) :
                i2c_addr_sel_1 ( (efuseval1 >> 0) & 1),
                i2c_addr_sel_2 ( (efuseval1 >> 1) & 1),
                touch_down_counter ( (efuseval1 >> 4) & 0x1F),
                temp_sens_cal ( (efuseval1 >> 9) & 0x7F),
                hamming_code (efuseval2 & 0x3F),
                week ( (efuseval2 >> 6) & 0x3F),
                year ( (efuseval2 >> 12) & 0xF),
                waver ( (efuseval3) & 0x1F),
                lot ( (efuseval3 >> 5) & 0x3FF),
                fab (static_cast<uint16_t> (efuseval3 >> 15)),
                y_coor ( (efuseval4 >> 2) & 0x3F),
                x_coor ( (efuseval4 >> 8) & 0x3F)
            {
                std::ostringstream  serialNr;

                serialNr << std::setfill ('0') << std::setw (4) << std::uppercase << std::hex << efuseval1;
                serialNr << "-";
                serialNr << std::setfill ('0') << std::setw (4) << std::uppercase << std::hex << efuseval2;
                serialNr << "-";
                serialNr << std::setfill ('0') << std::setw (4) << std::uppercase << std::hex << efuseval3;
                serialNr << "-";
                serialNr << std::setfill ('0') << std::setw (4) << std::uppercase << std::hex << efuseval4;

                serialNumber = serialNr.str();
            };
        };

        /**
        * This class is prepared for common functions for the design steps A11/B11/B12 of the M2452 imager.
        * It provides an implementation that utilizes the LPFSM mode and runs with the
        * Safe Reconfiguration ROM firmware.
        * The hardware can support individual frames, but as everything can be supported with
        * superframe mode this class doesn't support the CFGCNT_CSICFG settings for it.
        */
        class ImagerM2452 : public Imager
        {
        protected:
            explicit ImagerM2452 (const ImagerParameters &params,
                                  size_t measurementBlockCount = 1,
                                  size_t measurementBlockCapacity = 17);
        public:
            ~ImagerM2452();

            std::string getSerialNumber() override;
            void startCapture() override;
            void reconfigure (const ImagerUseCaseDefinition &useCase, uint16_t &reconfigIndex) override;
            void stopCapture() override;
            std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter() override;

        protected:
            ImagerVerificationStatus verifyFrameRateSettings (const ImagerUseCaseDefinition &useCase,
                    bool rawFrameBasedFrameRate) override;
            void downloadInitialConfig (const std::map < uint16_t, uint16_t > &baseConfig);
            std::map < uint16_t, uint16_t > prepareUseCase (const ImagerUseCaseDefinition &useCase) override;
            void prepareMeasurementBlockTargetTime (MeasurementBlockId mbId,
                                                    const double mbTargetTime,
                                                    const double mbMeasurementTime) override;
            uint16_t calcRegExposure (const uint32_t &expoTime, const uint32_t modfreq) const override;
            std::vector < uint16_t > getSerialRegisters() override;
            void getReadoutDelays (double &ifdel, double &lblank, double &cycAdcSocd, double &cycAdcOddd) const override;
            bool isValidExposureTime (bool enabledMixedMode,
                                      size_t overallRawFrameCount,
                                      uint32_t exposureTime,
                                      uint32_t modFreq) override;
            ImagerVerificationStatus verifyModulationSettings (const ImagerUseCaseDefinition &useCase) override;
            void adjustRowCount (uint16_t &row) override;
            void evaluatePostStartState();
            const std::map < ImagerRawFrame::ImagerDutyCycle, std::map < uint16_t, uint16_t > > &getPhaseMapping() override;

            const static std::string MSG_STARTLPFSMIDLE;
            const static std::string MSG_DPHYPLLLOCK;
            const static std::string MSG_MODPLLLOCK;
            const static std::string MSG_ISMACTIVE;

            bool calcRawFrameRateTime (
                const ImagerUseCaseDefinition &useCase,
                uint32_t expoTime,
                uint32_t modFreq,
                bool isFirstRawFrame,
                double &rawFrameTime) const override;

            std::mutex m_reconfigLock;

        private:
            const static std::array<uint16_t, 4> EXPO_PRESCALE_MAP;

            /**
            * Contains the mapping phase-angle to register value for each supported dutycycle.
            */
            const static std::map < ImagerRawFrame::ImagerDutyCycle, std::map < uint16_t, uint16_t > > PSMAPPING_CLK4;
        };
    }
}
