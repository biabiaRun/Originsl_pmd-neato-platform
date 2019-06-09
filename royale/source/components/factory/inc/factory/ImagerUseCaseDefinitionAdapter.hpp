/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerUseCaseDefinition.hpp>
#include <usecase/UseCaseDefinition.hpp>
#include <common/exceptions/LogicError.hpp>
#include <algorithm>
#include <set>

#pragma once

namespace royale
{
    namespace factory
    {
        class ImagerRawFrameAdapterHelper : public royale::imager::ImagerRawFrame
        {
        public:
            ImagerRawFrameAdapterHelper (const royale::usecase::RawFrameSet &rfs, uint16_t pa, bool isLastofSet, bool isFirstOfSet)
            {
                switch (rfs.alignment)
                {
                    case royale::usecase::RawFrameSet::Alignment::CLOCK_ALIGNED:
                        alignment = royale::imager::ImagerRawFrame::ImagerAlignment::CLOCK_ALIGNED;
                        break;
                    case royale::usecase::RawFrameSet::Alignment::NEXTSTOP_ALIGNED:
                        alignment = royale::imager::ImagerRawFrame::ImagerAlignment::NEXTSTOP_ALIGNED;
                        break;
                    case royale::usecase::RawFrameSet::Alignment::START_ALIGNED:
                        alignment = royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
                        break;
                    case royale::usecase::RawFrameSet::Alignment::STOP_ALIGNED:
                        alignment = royale::imager::ImagerRawFrame::ImagerAlignment::STOP_ALIGNED;
                        break;
                    default:
                        throw royale::common::LogicError ("unknown alignment");
                }

                //a clock-aligned RawFrameSet translates into a clock-aligned raw frame
                //followed by start-aligned raw frames (semantically equal)
                if (!isFirstOfSet && ImagerAlignment::CLOCK_ALIGNED == alignment)
                {
                    alignment = ImagerAlignment::START_ALIGNED;
                }

                switch (rfs.dutyCycle)
                {
                    case royale::usecase::RawFrameSet::DutyCycle::DC_0:
                        dutyCycle = royale::imager::ImagerRawFrame::ImagerDutyCycle::DC_0;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_100:
                        dutyCycle = royale::imager::ImagerRawFrame::ImagerDutyCycle::DC_100;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_25:
                        dutyCycle = royale::imager::ImagerRawFrame::ImagerDutyCycle::DC_25;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_25_DEPRECATED:
                        dutyCycle = royale::imager::ImagerRawFrame::ImagerDutyCycle::DC_25_DEPRECATED;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_50:
                        dutyCycle = royale::imager::ImagerRawFrame::ImagerDutyCycle::DC_50;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_37_5:
                        dutyCycle = royale::imager::ImagerRawFrame::ImagerDutyCycle::DC_37_5;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_37_5_DEPRECATED:
                        dutyCycle = royale::imager::ImagerRawFrame::ImagerDutyCycle::DC_37_5_DEPRECATED;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_75:
                        dutyCycle = royale::imager::ImagerRawFrame::ImagerDutyCycle::DC_75;
                        break;
                    case royale::usecase::RawFrameSet::DutyCycle::DC_AUTO:
                        dutyCycle = royale::imager::ImagerRawFrame::ImagerDutyCycle::DC_AUTO;
                        break;
                    default:
                        throw royale::common::LogicError ("unknown duty cycle");
                }

                switch (rfs.phaseDefinition)
                {
                    case royale::usecase::RawFrameSet::PhaseDefinition::GRAYSCALE:
                        grayscale = true;
                        break;
                    case royale::usecase::RawFrameSet::PhaseDefinition::MODULATED_4PH_CW:
                        grayscale = false;
                        break;
                    default:
                        throw royale::common::LogicError ("unknown phase definition");
                }

                if (royale::usecase::RawFrameSet::MODFREQ_AUTO() == rfs.modulationFrequency)
                {
                    modulationFrequency = royale::imager::ImagerRawFrame::MODFREQ_AUTO();
                }
                else
                {
                    modulationFrequency = rfs.modulationFrequency;
                }

                phaseAngle = pa;
                tEyeSafety = rfs.tEyeSafety;
                ssc_freq = rfs.ssc_freq;
                ssc_kspread = rfs.ssc_kspread;
                ssc_delta = rfs.ssc_delta;
                isEndOfLinkedRawFrames = isLastofSet;
                isEndOfLinkedMeasurement = false;
                isStartOfLinkedRawFrames = isFirstOfSet;
            }
        };

        /**
         * This class provides a helper method to convert from royale::usecase::UseCaseDefinition specific
         * data structures to the data structure used by the imager component (royale::imager::ImagerUseCaseDefinition).
         * Please note that different forks of Royale may have different adapter implementations,
         * but as long-term goal there should only be one common version of ImagerUseCaseDefinition shared
         * by all forks of Royale.
         */
        class ImagerUseCaseDefinitionAdapter : public royale::imager::ImagerUseCaseDefinition
        {
        public:
            ImagerUseCaseDefinitionAdapter (const royale::usecase::UseCaseDefinition &useCase, uint16_t roiCMin, uint16_t roiRMin, uint16_t flowControlRate)
            {
                m_enableMixedMode = useCase.getStreamIds().count() > 1;
                m_enableSSC = useCase.getSSCEnabled();
                m_rawFrameRate = flowControlRate;
                m_targetRate = useCase.getTargetRate();

                uint16_t imageColumns;
                uint16_t imageRows;
                useCase.getImage (imageColumns, imageRows);
                m_imageColumns = imageColumns;
                m_imageRows = imageRows;
                m_roiCMin = roiCMin;
                m_roiRMin = roiRMin;

                const auto rfsets = useCase.getRawFrameSets();

                if (rfsets.size())
                {
                    for (const auto rfs : rfsets)
                    {
                        size_t rfIndex = 0;

                        for (const auto phaseAngle : rfs.getPhaseAngles())
                        {
                            auto imagerRawFrame = ImagerRawFrameAdapterHelper (rfs, phaseAngle, rfIndex == rfs.getPhaseAngles().size() - 1, 0 == rfIndex);
                            imagerRawFrame.exposureTime = useCase.getExposureGroups().at (rfs.exposureGroupIdx).m_exposureTime;
                            m_rawFrames.push_back (imagerRawFrame);
                            rfIndex++;
                        }
                    }

                    setSafeReconfigPoints (useCase);
                }
            }

            virtual ~ImagerUseCaseDefinitionAdapter() = default;

        private:
            /**
            * Alters the isEndOfLinkedMeasurement flag of the raw frames in m_rawFrames based on the frame group information.
            * This method expects to have the isEndOfLinkedMeasurement flag set to false for all objects
            * of m_rawFrames prior to its call.
            * The isEndOfLinkedMeasurement flag will be set for raw frames at the end of each frame group. The only exception
            * is if a frame group is surrounded by raw frame sets of another frame group (interleaved case).
            * For such a frame group the end of the measurement is defined to be at the last raw frame of the
            * surrounding frame group.
            *
            * \param useCase       To get information about the frame groups
            */
            void setSafeReconfigPoints (const royale::usecase::UseCaseDefinition &useCase)
            {
                // IDs of raw frames that might be followed by a safe reconfig points
                std::set<size_t> rfPossiblySafeForReconfig;

                // IDs of raw frames that aren't immediately before a safe reconfig point, even if other logic also
                // puts them in the rfSafeForReconfig list
                std::set<size_t> rfNotSafeForReconfig;

                for (const auto sid : useCase.getStreamIds())
                {
                    for (const auto fg : useCase.getStream (sid)->m_frameGroups)
                    {
                        auto myFg = fg; //FrameGroup doesn't guarantee any order of m_frameSetIds
                        std::sort (myFg.m_frameSetIds.begin(), myFg.m_frameSetIds.end());
                        const royale::usecase::RawFrameSetId rfsIdFirst = myFg.m_frameSetIds.front();
                        const royale::usecase::RawFrameSetId rfsIdLast = myFg.m_frameSetIds.back();

                        rfPossiblySafeForReconfig.insert (useCase.getSequenceIndicesForRawFrameSet (rfsIdLast).back());

                        // If there exists any frame group such that the reconfig point would be in the middle
                        // of the group then that is not a safe reconfig point. This also applies to the
                        // interleaved case - if we have an "ES1 HT1 ES2 HT2 HT3" interleaved mode, then all RFS
                        // in HT1 are in the middle of the ES FG.
                        //
                        // Note: this is not iterating over myFg, it's iterating over every integer ID that's in
                        // the range covered by the frame group.
                        for (auto rfsId = rfsIdFirst; rfsId < rfsIdLast; rfsId++)
                        {
                            for (const auto rfIdx : useCase.getSequenceIndicesForRawFrameSet (rfsId))
                            {
                                rfNotSafeForReconfig.insert (rfIdx);
                            }
                        }
                    }
                }

                for (size_t rfIdx = 0; rfIdx < m_rawFrames.size(); rfIdx++)
                {
                    m_rawFrames.at (rfIdx).isEndOfLinkedMeasurement =
                        m_rawFrames.at (rfIdx).isEndOfLinkedRawFrames &&
                        rfPossiblySafeForReconfig.count (rfIdx) &&
                        !rfNotSafeForReconfig.count (rfIdx);
                }
            }
        };
    }
}
