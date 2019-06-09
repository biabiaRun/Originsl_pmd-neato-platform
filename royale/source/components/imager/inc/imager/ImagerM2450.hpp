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
#include <iomanip>
#include <sstream>

namespace royale
{
    namespace imager
    {
        struct ImagerM2450Serial
        {
            std::string  serialNumber;
            const uint16_t i2c_addr_sel_1;
            const uint16_t i2c_addr_sel_2;
            const uint16_t roi_sel;
            const uint16_t csi2_disable;
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

            ImagerM2450Serial (uint16_t efuseval1, uint16_t efuseval2, uint16_t efuseval3, uint16_t efuseval4) :
                i2c_addr_sel_1 ( (efuseval1 >> 0) & 1),
                i2c_addr_sel_2 ( (efuseval1 >> 1) & 1),
                roi_sel ( (efuseval1 >> 2) & 1),
                csi2_disable ( (efuseval1 >> 3) & 1),
                touch_down_counter ( (efuseval1 >> 4) & 0x1F),
                temp_sens_cal ( (efuseval1 >> 9) & 0x7F),
                hamming_code (efuseval2 & 0x3F),
                week ( (efuseval2 >> 6) & 0x3F),
                year ( (efuseval2 >> 12) & 0xF),
                waver ( (efuseval3) & 0x1F),
                lot ( (efuseval3 >> 5) & 0x3FF),
                fab (static_cast<uint16_t> (efuseval3 >> 15)),
                y_coor ( (efuseval4 >> 2) & 0x1F),
                x_coor ( (efuseval4 >> 7) & 0x1F)
            {
                std::ostringstream  serialNr;
                uint32_t serial1 = 0;
                uint32_t serial2 = 0;

                serial1 = ( (efuseval3 & 0x7FE0) << 11) + ( (efuseval4 & 0xF80) << 1) + ( (efuseval4 & 0x7C) >> 2);
                serial2 = ( (efuseval2 & 0xF000) << 4) + ( (efuseval2 & 0xFC0) << 2) + (efuseval3 & 0x1F);

                serialNr << std::setfill ('0') << std::setw (4) << std::dec << (serial2 >> 16);
                serialNr << "-";
                serialNr << std::setfill ('0') << std::setw (2) << std::dec << ( (serial2 >> 8) & 0xFF);
                serialNr << std::setfill ('0') << std::setw (2) << std::dec << (serial2 & 0xFF);
                serialNr << "-";
                serialNr << std::setfill ('0') << std::setw (4) << std::dec << (serial1 >> 16);
                serialNr << "-";
                serialNr << std::setfill ('0') << std::setw (2) << std::dec << ( (serial1 >> 8) & 0xFF);
                serialNr << std::setfill ('0') << std::setw (2) << std::dec << (serial1 & 0xFF);

                serialNumber = serialNr.str();
            };

            /**
            * Use efuses to create an unique string that identifies the sensor.
            */
            static std::string calculateSerialNumber (const std::vector < uint16_t > &serialRegisters)
            {
                return ImagerM2450Serial (serialRegisters.at (0), serialRegisters.at (1), serialRegisters.at (2), serialRegisters.at (3)).serialNumber;
            }
        };

        /**
        * This abstract class is prepared for the MiraCE and is only valid
        * for the PicoS module running the Enclustra firmware that uses the
        * imager's parallel interface. The implementation only supports phase
        * shifts of multiples of 45 degrees.
        * The implementation loads a default configuration for a 133MHz system clock.
        */
        class ImagerM2450 : public Imager
        {
        public:
            ImagerM2450 (const ImagerParameters &params,
                         size_t measurementBlockCount,
                         size_t measurementBlockCapacity);
            ~ImagerM2450();

            void initialize() override;
            std::string getSerialNumber() override;

        protected:
            virtual ImagerVerificationStatus verifyModulationSettings (const ImagerUseCaseDefinition &useCase) override;
            uint16_t calcRegExposure (const uint32_t &expoTime, const uint32_t modfreq) const override;
            bool isValidExposureTime (bool enabledMixedMode,
                                      size_t overallRawFrameCount,
                                      uint32_t exposureTime,
                                      uint32_t modFreq) override;
            void adjustRowCount (uint16_t &rowCount) override;

            virtual bool calcRawFrameRateTime (
                const ImagerUseCaseDefinition &useCase,
                uint32_t expoTime,
                uint32_t modFreq,
                bool isFirstRawFrame,
                double &rawFrameTime) const override;

            const std::map < ImagerRawFrame::ImagerDutyCycle, std::map < uint16_t, uint16_t > > &getPhaseMapping() override;

        private:
            /**
            * Contains the mapping phase-angle to register value for each supported dutycycle.
            */
            const static std::map < ImagerRawFrame::ImagerDutyCycle, std::map < uint16_t, uint16_t > > PSMAPPING_CLK4;
        };
    }
}
