/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/exceptions/LogicError.hpp>

#include <imager/ImagerMeasurementBlockBase.hpp>

#include <algorithm>
#include <cmath>
#include <set>

using namespace royale::imager;
using namespace royale::common;

const size_t ImagerMeasurementBlockBase::MB_REPEAT_LIMIT = 255;

ImagerMeasurementBlockBase::ImagerMeasurementBlockBase (
    const std::shared_ptr<royale::hal::IBridgeImager> &bridge,
    size_t defaultMeasurementBlockCount,
    size_t defaultMeasurementBlockCapacity) :
    ImagerBase (bridge),
    m_defaultMeasurementBlockCount (defaultMeasurementBlockCount),
    m_defaultMeasurementBlockCapacity (defaultMeasurementBlockCapacity),
    m_rfAssignment ( {})
{

}

std::vector<std::size_t> ImagerMeasurementBlockBase::getMeasurementBlockSizes() const
{
    std::vector<std::size_t> result;

    for (const auto &mb : m_mbList)
    {
        result.push_back (mb.sequence.size());

        for (size_t cnt = 1u; cnt < mb.cycles; cnt++)
        {
            result.push_back (mb.sequence.size());
        }
    }

    return result;
}

void ImagerMeasurementBlockBase::prepareMeasurementBlockTargetTime (MeasurementBlockId mbId,
        const double mbTargetTime,
        const double mbMeasurementTime)
{
    //This default implementation can be used if the imager is not supporting a measurement block
    //based frame rate counter. Note that the following assignment is for documentation purpose only
    //as the frameRateCounter member will already be initialized to zero and will never change afterwards.
    m_mbList[mbId].frameRateCounter = 0u;
}

const std::vector < double > ImagerMeasurementBlockBase::generateRawFrameTimings (const ImagerUseCaseDefinition &useCase) const
{
    const auto &rfList = useCase.getRawFrames();
    std::vector < double > rawFrameTimes (rfList.size());

    const double tMasterInterval = 1. / static_cast<double> (useCase.getTargetRate());

    //by definition the first RawFrame always must be a master-clock RF
    if (ImagerRawFrame::ImagerAlignment::CLOCK_ALIGNED != rfList[0].alignment)
    {
        throw LogicError();
    }

    //equidistant distribution of the master clock RFS and alignment of start/stop aligned RF
    for (size_t idx = 0u; idx < rfList.size(); idx++)
    {
        switch (rfList[idx].alignment)
        {
            case ImagerRawFrame::ImagerAlignment::CLOCK_ALIGNED:
                {
                    rawFrameTimes[idx] = tMasterInterval;
                }
                break;
            case ImagerRawFrame::ImagerAlignment::START_ALIGNED:
                {
                    //By definition the order of the alignments in the RF list must always be:
                    //1) One clock aligned RF
                    //2) Zero, one or more start aligned RF
                    //3) Zero, one or more stop aligned RF
                    //4) Zero, one or more nextstop aligned RF
                    //5) Zero, one or more repetitions of such a sequence, starting again at point 1)
                    if (ImagerRawFrame::ImagerAlignment::STOP_ALIGNED == rfList[idx - 1].alignment ||
                            ImagerRawFrame::ImagerAlignment::NEXTSTOP_ALIGNED == rfList[idx - 1].alignment)
                    {
                        throw LogicError();
                    }

                    double tPrevRF = 0.0;
                    calcRawFrameRateTime (useCase, rfList[idx - 1].exposureTime,
                                          rfList[idx - 1].modulationFrequency,
                                          rfList[idx - 1].isStartOfLinkedRawFrames,
                                          tPrevRF);
                    rawFrameTimes[idx] = rawFrameTimes[idx - 1] - tPrevRF;
                    rawFrameTimes[idx - 1] = tPrevRF;
                    //Note: rawFrameTimes[idx] is intended to become negative if the current RF does not fit
                    //      into the free time slot of the previous RF. A negative value indicates an error and
                    //      represents the lacking time that would be needed to be added to tMasterInterval to
                    //      make the use case feasible. This error will propagate if following non-master-clock RF
                    //      are appended. The last RF before the next master-clock RFS contains the overall error.
                }
                break;
            case ImagerRawFrame::ImagerAlignment::STOP_ALIGNED:
                //Note: rawFrameSetTimes[idx] will always get its time granted. However, if there is not enough
                //      time left in the current master clock interval, the missing time will be accumulated
                //      at the last start/clock aligned RFS of the sequence.
                if (rfList[idx - 1].alignment == ImagerRawFrame::ImagerAlignment::NEXTSTOP_ALIGNED)
                {
                    throw LogicError();
                }
            case ImagerRawFrame::ImagerAlignment::NEXTSTOP_ALIGNED:
                {
                    double tThisRF = 0.0;
                    auto lastReferenceIndex = idx;

                    while (--lastReferenceIndex &&
                            (ImagerRawFrame::ImagerAlignment::START_ALIGNED != rfList[lastReferenceIndex].alignment &&
                             ImagerRawFrame::ImagerAlignment::CLOCK_ALIGNED != rfList[lastReferenceIndex].alignment));

                    calcRawFrameRateTime (useCase,
                                          rfList[idx].exposureTime,
                                          rfList[idx].modulationFrequency,
                                          rfList[idx].isStartOfLinkedRawFrames,
                                          tThisRF);
                    rawFrameTimes[idx] = tThisRF;

                    if (rfList[idx].alignment == ImagerRawFrame::ImagerAlignment::NEXTSTOP_ALIGNED &&
                            idx - lastReferenceIndex == 1)
                    {
                        rawFrameTimes[lastReferenceIndex] += tMasterInterval - tThisRF;
                        //Note: This will place the raw frame after a "virtual" clock aligned one in the next
                        //      master clock interval. The time in between will be added to the preceding "real"
                        //      clock or start aligned RF.
                    }
                    else
                    {
                        rawFrameTimes[lastReferenceIndex] -= tThisRF;
                    }
                }
                break;
        }
    }

    return rawFrameTimes;
}

bool ImagerMeasurementBlockBase::isRawFrameAssignmentToMeasurementBlockEqual (const ImagerUseCaseDefinition &useCase,
        const std::map<size_t, MeasurementBlockId> &rfAssignment,
        const MeasurementBlockId first,
        const MeasurementBlockId second) const
{
    //assumption: measurement blocks are equal if the raw frames assigned to them are equal
    //and the target times of the measurement blocks are equal
    const auto &rfList = useCase.getRawFrames();
    const auto rfTargetTimes = generateRawFrameTimings (useCase);
    std::vector<size_t> firstRfList;
    std::vector<size_t> secondRfList;

    double firstMBTime = 0.;
    double secondMBTime = 0.;

    for (const auto mapping : rfAssignment)
    {
        if (first == mapping.second)
        {
            firstRfList.push_back (mapping.first);
            firstMBTime += rfTargetTimes.at (mapping.first);
        }

        if (second == mapping.second)
        {
            secondRfList.push_back (mapping.first);
            secondMBTime += rfTargetTimes.at (mapping.first);
        }
    }

    //using this epsilon is okay as it is prefers MBs to be considered as not being equal
    //which only results in one more MB used rather than a wrong resulting MB timing
    if (::fabs (firstMBTime - secondMBTime) > .1e-9)
    {
        return false;
    }

    if (firstRfList.size() == secondRfList.size())
    {
        for (size_t idRfs = 0u; idRfs < firstRfList.size(); idRfs++)
        {
            if (rfList.at (firstRfList.at (idRfs)) !=
                    rfList.at (secondRfList.at (idRfs)))
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}

std::vector<MeasurementBlock> ImagerMeasurementBlockBase::createMeasurementBlockList (const ImagerUseCaseDefinition &useCase) const
{
    return std::vector<MeasurementBlock> (m_defaultMeasurementBlockCount, MeasurementBlock (m_defaultMeasurementBlockCapacity));
}

bool ImagerMeasurementBlockBase::verifyRawFrameAssignment (const ImagerUseCaseDefinition &useCase) const
{
    auto mbList = createMeasurementBlockList (useCase);
    std::vector<size_t> mbFill (mbList.size(), 0);
    std::map<size_t, MeasurementBlockId> rfAssignment;
    const auto &rfList = useCase.getRawFrames();

    MeasurementBlockId currentMb = 0;

    for (size_t idRfs = 0u; idRfs < rfList.size(); idRfs++)
    {
        bool needsNewMB = false;
        const auto &rf = rfList.at (idRfs);

        //assuming that clock aligned raw frames will have some time gap in front of them,
        //so it is preferable to put them into separate MBs to allow the imager to enter the low power mode
        if (currentMb > 0 && rf.alignment == ImagerRawFrame::ImagerAlignment::CLOCK_ALIGNED)
        {
            needsNewMB = true;
        }

        if (mbList[currentMb].maxSequenceLength - mbFill[currentMb] < 1)
        {
            //at least two MBs are filled, check if the are equal
            if (currentMb > 0 &&
                    mbList[currentMb - 1].cycles < MB_REPEAT_LIMIT &&
                    isRawFrameAssignmentToMeasurementBlockEqual (useCase, rfAssignment, currentMb - 1, currentMb))
            {
                //if content of mbList[currentMb-1] equals mbList[currentMb] compact them
                mbList[currentMb - 1].cycles++;
                mbFill[currentMb] = 0;

                //remove repeated raw frames from the assignment map
                removeMappingForMeasurementBlock (rfAssignment, currentMb);
                needsNewMB = false;
            }
            else
            {
                needsNewMB = true;
            }
        }

        if (needsNewMB)
        {
            currentMb++;
        }

        if (currentMb >= mbList.size())
        {
            return false;
        }

        rfAssignment[idRfs] = currentMb;
        mbFill[currentMb] ++;
    }

    return ! (0 == rfAssignment.size());
}

void ImagerMeasurementBlockBase::generateRawFrameAssignment (const ImagerUseCaseDefinition &useCase)
{
    std::vector<size_t> mbFill (m_mbList.size(), 0);
    m_rfAssignment.clear();
    const auto &rfList = useCase.getRawFrames();
    MeasurementBlockId currentMb = 0;

    for (size_t idRf = 0u; idRf < rfList.size(); idRf++)
    {
        auto linkedRawFrameCount = idRf;

        for (; linkedRawFrameCount < rfList.size() && !rfList.at (linkedRawFrameCount).isEndOfLinkedRawFrames;
                linkedRawFrameCount++);
        linkedRawFrameCount -= idRf - 1;

        if (m_mbList[currentMb].maxSequenceLength - mbFill[currentMb] < linkedRawFrameCount)
        {
            //at least two MBs are filled, check if the are equal
            if (currentMb > 0 &&
                    m_mbList[currentMb - 1].cycles < MB_REPEAT_LIMIT &&
                    isRawFrameAssignmentToMeasurementBlockEqual (useCase, m_rfAssignment, currentMb - 1, currentMb))
            {
                //if content of mbList[currentMb-1] equals mbList[currentMb] compact them
                m_mbList[currentMb - 1].cycles++;
                mbFill[currentMb] = 0;

                //remove repeated raw frame sets from the assignment map
                removeMappingForMeasurementBlock (m_rfAssignment, currentMb);
            }
            else
            {
                m_mbList[currentMb].safeForReconfig = rfList.at (idRf - 1).isEndOfLinkedMeasurement;
                currentMb++;
            }
        }

        m_rfAssignment[idRf] = currentMb;
        mbFill[currentMb] ++;
    }

    m_mbList[currentMb].safeForReconfig = rfList.back().isEndOfLinkedMeasurement;

    for (MeasurementBlockId idx = 0; idx < m_mbList.size(); idx++)
    {
        m_mbList[idx].sequence.resize (mbFill[idx]);
    }
}

void ImagerMeasurementBlockBase::removeMappingForMeasurementBlock (std::map<size_t, MeasurementBlockId> &rfAssignment,
        MeasurementBlockId currentMb)
{
    auto mapping_it = rfAssignment.begin();
    while (mapping_it != rfAssignment.end())
    {
        if (mapping_it->second == currentMb)
        {
            mapping_it = rfAssignment.erase (mapping_it);
        }
        else
        {
            ++mapping_it;
        }
    }
}

uint32_t ImagerMeasurementBlockBase::getMaxSafeReconfigTimeMilliseconds (const ImagerUseCaseDefinition &useCase) const
{
    const auto rfTargetTimes = generateRawFrameTimings (useCase);

    double mbMaxTime = 0.;
    double mbSumTime = 0.;

    for (MeasurementBlockId mbIdx = 0; mbIdx < m_mbList.size(); mbIdx++)
    {
        //sum-up the MB durations
        {
            double mbTime = 0.;

            for (const auto rf_mapping : m_rfAssignment)
            {
                const auto mbIndex = rf_mapping.second;

                if (mbIndex == mbIdx)
                {
                    const auto &rfTime = rfTargetTimes.at (rf_mapping.first);
                    mbTime += rfTime;
                }
            }

            if (!m_mbList[mbIdx].safeForReconfig)
            {
                //repeats only count if the MB is not safe for reconfig
                mbTime *= m_mbList[mbIdx].cycles;
            }

            mbSumTime += mbTime;
        }

        //if the MB is safe for reconfig, summarizing MB durations can end
        //and it can be decided if this is the longest safe-for-reconfig
        //sequence of MBs found by now
        if (m_mbList[mbIdx].safeForReconfig)
        {
            if (mbSumTime > mbMaxTime)
            {
                mbMaxTime = mbSumTime;
            }

            //prepare for summarize the timings of the next
            //contiguous measurement block sequence
            mbSumTime = 0.;
        }
    }

    return static_cast<uint32_t> (1000. * mbMaxTime);
}
