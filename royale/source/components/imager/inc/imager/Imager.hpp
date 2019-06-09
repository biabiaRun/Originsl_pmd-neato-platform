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

#include <imager/ImagerMeasurementBlockBase.hpp>
#include <imager/ImagerParameters.hpp>
#include <imager/IPllStrategy.hpp>

namespace royale
{
    namespace imager
    {
        /**
         * The main sensor that's capturing the raw data.
         */
        class Imager : public ImagerMeasurementBlockBase
        {
        public:

            /**
             * It is expected that the constructor of a specialized imager will create and assign
             * a concrete modulation PLL calculation object of type IPllStrategy to the member variable m_pllCalc.
             *
             * \param   params                           Set of parameters that have to be passed to a concrete imager object.
             * \param   modPllLutCount                   The number of LUT available for assignment to a sequencer entry
             * \param   lutIdxOffset                     The offset between addresses for single modpll LUT entries
             * \param   clkDiv                           The clock divider used  for the modulation PLL
             * \param   defaultMeasurementBlockCount     The number of available measurement blocks (there must be at least 1)
             * \param   defaultMeasurementBlockCapacity  The number of sequence entries per measurement block available for
             *                                           storing raw frame descriptions
             */
            Imager (const ImagerParameters &params,
                    uint16_t modPllLutCount,
                    uint16_t lutIdxOffset,
                    uint16_t clkDiv,
                    size_t defaultMeasurementBlockCount,
                    size_t defaultMeasurementBlockCapacity);

            ~Imager();

            void wake() override;
            void initialize() override;
            void sleep() override;
            ImagerVerificationStatus verifyUseCase (const ImagerUseCaseDefinition &useCase) override;
            void executeUseCase (const ImagerUseCaseDefinition &useCase) override;
            void startCapture() override;
            void reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex) override;
            void reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex) override;
            void stopCapture() override;
            void setExternalTrigger (bool useExternalTrigger) override;

        protected:
            const static std::string MSG_CFGCONFLICT;
            const static std::string MSG_USECASEEXECUTION;
            const static std::string MSG_CAPTUREBUSY;
            const static std::string MSG_WRONGIF;
            const static std::string MSG_WRONGROI;
            const static std::string MSG_STARTMTCUIDLE;
            const static std::string MSG_STOPMTCUIDLE;

            const ImagerParameters m_imagerParams;
            ImagerState m_imagerState;
            ImagerUseCaseDefinition m_preparedUcd;
            ImagerUseCaseDefinition m_executingUcd;

            static const double FSYSCLK; //!< this imager implementation works only with 400/3 MHz system clock
            const uint16_t MODPLLLUTCOUNT; //!< The number of LUT available for assignment to a sequencer entry
            const uint16_t LUTIDXOFFSET; //!< The offset between addresses for single modpll LUT entries
            const uint16_t CLKDIV; //!< The clock divider used  for the modulation PLL

            /**
            * The PLL calculation is used to generate register settings for modulation frequencies
            */
            std::unique_ptr <IPllStrategy> m_pllCalc;

            uint16_t m_regTrigger;
            uint16_t m_valStartTrigger;
            uint16_t m_valEndTrigger;
            uint16_t m_regStatus;

            /**
            * The "idle" status that should make shutdownSequencer() return.  For example, with the
            * LPFSM then the sequencer effectively hasn't stopped until both MTCU and LPFSM are
            * idle.
            *
            * This is both the mask and the value.  shutdownSequencer() will ignore the bits that
            * aren't set, but will wait for all the bits that are set to also be set in the status
            * register.
            */
            uint16_t m_statusIdle;

            uint16_t m_regPllLutLower;
            uint16_t m_regReconfCnt;

            /**
            * Reconfigure imager for frame rate or exposure time changes (contained in the IUCD).
            * The implementation here only contains sanity checks; derived classes should reimplement it
            * with the actual functionality (and call this base class version first).
            *
            * This gets called from reconfigureExposureTimes and reconfigureRawFrameRate.
            */
            virtual void reconfigure (const ImagerUseCaseDefinition &useCase, uint16_t &reconfigIndex);

            /*
            * Contains the mapping phase - angle to register value for each supported dutycycle
            */
            virtual const std::map < ImagerRawFrame::ImagerDutyCycle, std::map < uint16_t, uint16_t > > &getPhaseMapping() = 0;

            /*
            * Finds a free index in the m_lutAssignment map.
            * \return The index of a free LUT or MODPLLLUTCOUNT if no LUT is free.
            */
            uint16_t getFreeLutIndex();

            /**
            * The map associates a modulation frequency to a certain LUT of the imager.
            */
            std::map < uint32_t, uint16_t > m_lutAssignment;

            /**
            * Returns the modulation frequency that shall be used for exposure
            * of grayscale only raw frames. If a specific imager is not supporting
            * the default modulation frequency for grayscale images it must
            * override this method and provides its own configuration.
            */
            virtual uint32_t getGrayscaleModulationFrequency() const;

            void initialRegisterConfig (const std::map < uint16_t, uint16_t > &baseConfig);
            virtual void shutDownSequencer();
            void updateModulationAssigmentLUT (const ImagerUseCaseDefinition &useCase);
            void prepareModulationAssigment (const ImagerUseCaseDefinition &useCase, std::map < uint16_t, uint16_t > &regChanges);
            void preparePsSettings (const ImagerUseCaseDefinition &useCase);
            void prepareFrameRateSettings (const ImagerUseCaseDefinition &useCase);
            void prepareExposureSettings (const ImagerUseCaseDefinition &useCase);
            void prepareModulationSettings (const ImagerUseCaseDefinition &useCase, std::map < uint16_t, uint16_t > &regChanges);
            virtual void prepareSSCSettings (const uint16_t lutIndex,
                                             const std::vector<uint16_t> &pllCfg,
                                             std::map < uint16_t, uint16_t > &regChanges);
            void prepareROI (const ImagerUseCaseDefinition &useCase,
                             uint16_t &roiCMin, uint16_t &roiCMax,
                             uint16_t &roiRMin, uint16_t &roiRMax);

            virtual uint16_t calcRegExposure (const uint32_t &expoTime, const uint32_t modfreq) const = 0;
            virtual void getReadoutDelays (double &ifdel, double &lblank, double &cycAdcSocd, double &cycAdcOddd) const = 0;

            /**
            * Each imager type (e.g. M2450 or M2452) defines where the pseudodata line is stored. Depending on
            * this information, either the first data line gets replaced, or one additional line is added to the
            * data stream. This information is required for subsequent processing steps and influences the row count
            * of the overall buffer. This method lets the derived imager define if a row needs to be added because
            * the pseudodata line is replacing the first data line.
            */
            virtual void adjustRowCount (uint16_t &rowCount) = 0;

            ImagerVerificationStatus verifyPhaseSettings (const ImagerUseCaseDefinition &useCase);
            virtual ImagerVerificationStatus verifyModulationSettings (const ImagerUseCaseDefinition &useCase) = 0;
            virtual ImagerVerificationStatus verifySSCSettings (const ImagerUseCaseDefinition &useCase);
            virtual bool isValidExposureTime (bool mixedModeActive,
                                              size_t overallRawFrameCount,
                                              uint32_t exposureTime,
                                              uint32_t modFreq) = 0;
            ImagerVerificationStatus verifyRawFrames (const ImagerUseCaseDefinition &useCase);

            /**
            * Validates if the frame rate of the given use case is feasible. This mainly depends on the concrete imager type
            * and the module configuration (raw frame rate and system frequency).
            *
            * \param   useCase                  The useCase that contains the frame rate information that should be validated.
            * \param   rawFrameBasedFrameRate   True if the overall frame rate is solely defined by the duration of the single raw frames.
            *                                   Specialized imagers can override this parameter and set it to false in case they provide
            *                                   some frame rate counter that is not based on the single raw frames. In this case the
            *                                   specialized imager should also override prepareMeasurementBlockTargetTime to set the
            *                                   frame rate counter of the last raw frame to zero.
            * \return                           True if the use case is feasible, false otherwise.
            */
            virtual ImagerVerificationStatus verifyFrameRateSettings (const ImagerUseCaseDefinition &useCase,
                    bool rawFrameBasedFrameRate = true);

            virtual ImagerVerificationStatus verifyRegion (const ImagerUseCaseDefinition &useCase);
            ImagerVerificationStatus verifyModulationSettings (const ImagerUseCaseDefinition &useCase,
                    const uint16_t clkdiv,
                    const uint16_t modpllcount);

            /**
            * Prepare use case. The concrete imager implementation should use
            * this method to a set of registers containing the changes required
            * to configure the imager for the new use-case
            * \param   useCase   The use case definition.
            * \return A set of registers prepared to be written to the imager
            */
            virtual std::map < uint16_t, uint16_t > prepareUseCase (const ImagerUseCaseDefinition &useCase) = 0;

            /**
            *  rowLimit ROI column upper limit defines the number of accessible rows the imager sensor array
            */
            uint16_t m_rowLimitSensor;

            /**
            *  columnLimit ROI column upper limit defines the number of accessible columns of the imager sensor array
            */
            uint16_t m_columnLimitSensor;

            /**
            *  The trigger signal that should be used by the imager. This can be I2C or an external trigger defined in the ModuleConfig
            */
            ImgTrigger m_currentTrigger;

            /**
            * Clock to use internally for computing pauses.
            *
            * \todo ROYAL-3508 Using system_clock instead of steady_clock here due to broken chrono implementation in MSVC 2013
            */
            using WaitClockType = std::chrono::system_clock;

            /**
            * The time the imager was last stopped. This is used to determine when it is (eye-)safe to start capturing again.
            */
            std::chrono::time_point<WaitClockType> m_lastStopTime;

            /**
            * Tail time of the last active use case (i.e. for which startCapture() was actually called).
            */
            std::chrono::microseconds m_lastActiveTailTime;

            /**
            * Tail time of the last executed use case.
            */
            std::chrono::microseconds m_lastExecutedTailTime;

        };
    }
}
