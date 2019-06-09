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

#include <imager/ImagerUseCaseDefinition.hpp>

/**
 * A specialized ImagerUseCaseDefinition class to create 4-phase raw frame sequences.
 *
 * The raw frames will be generates using the phases 0, 90, 180 and 270 degrees.
 * Each raw frame with a phase of 270 degree will have the isEndOfLinkedRawFrames flag set.
 * Each raw frame with a phase of 0 degree will be a clock-aligned one.
 * The last raw frame always will always have the isEndOfLinkedRawFrames and the
 * isEndOfLinkedMeasurement flag set.
 * The first four raw frames will have the modulation frequency freqFirst set, the next
 * four raw frames will have the modulation frequency secondFreq set. The next four will
 * again have the modulation frequency freqFirst set and so on.
 * If there is a grayscaleExpo value defined (a value greater than zero) a single grayscale
 * raw frame will be inserted at the begin or tail, depending on the grayscaleFirst parameter.
 */
class ImagerUseCaseFourPhase : public royale::imager::ImagerUseCaseDefinition
{
public:
    explicit ImagerUseCaseFourPhase (uint16_t targetRate = 50u,
                                     size_t rawFrameCount = 4,
                                     uint32_t expoTime = 1000u,
                                     uint32_t freqFirst = 80320000u,
                                     uint32_t secondFreq = 60240000u,
                                     uint32_t grayscaleExpo = 0u,
                                     bool grayscaleFirst = false,
                                     bool enableSSC = false,
                                     double ssc_freq = 0.,
                                     double ssc_kspread = 0.,
                                     double ssc_deltaFirst = 0.,
                                     double ssc_deltaSecond = 0.)
    {
        m_targetRate = targetRate;
        m_roiCMin = 80;
        m_roiRMin = 84;
        m_imageColumns = 176;
        m_imageRows = 120;
        m_enableSSC = enableSSC;

        if (rawFrameCount)
        {
            auto freq = freqFirst;
            auto ssc_delta = ssc_deltaFirst;
            const double tEye = 0.;
            const size_t phases = 4;
            uint16_t phase = 0u;

            for (size_t rfCnt = 0; rfCnt < rawFrameCount; rfCnt++)
            {
                if (rfCnt % phases)
                {
                    m_rawFrames.push_back (royale::imager::ImagerRawFrame (freq, false, false, (0 == (rfCnt + 1) % phases),
                                           false, royale::imager::ImagerRawFrame::ImagerDutyCycle::DC_AUTO,
                                           expoTime, phase, royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED,
                                           tEye, ssc_freq, ssc_kspread, ssc_delta));
                }
                else
                {
                    const auto useFirstFreq = (rfCnt / phases % 2);
                    freq = useFirstFreq ? secondFreq : freqFirst;
                    ssc_delta = useFirstFreq ? ssc_deltaSecond : ssc_deltaFirst;
                    phase = 0u;
                    m_rawFrames.push_back (royale::imager::ImagerRawFrame (freq, false, true, false, false,
                                           royale::imager::ImagerRawFrame::ImagerDutyCycle::DC_AUTO, expoTime, phase,
                                           rfCnt ? royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED :
                                           royale::imager::ImagerRawFrame::ImagerAlignment::CLOCK_ALIGNED,
                                           tEye, ssc_freq, ssc_kspread, ssc_delta));
                }

                phase = static_cast<uint16_t> (phase + 90u);
            }

            m_rawFrames.back().isEndOfLinkedRawFrames = true;
            m_rawFrames.back().isEndOfLinkedMeasurement = true;
        }

        if (grayscaleExpo)
        {
            if (grayscaleFirst)
            {
                m_rawFrames.insert (m_rawFrames.begin(), royale::imager::ImagerRawFrame (royale::imager::ImagerRawFrame::MODFREQ_AUTO(),
                                    true, true, true, false, royale::imager::ImagerRawFrame::ImagerDutyCycle::DC_0, grayscaleExpo, 0u,
                                    royale::imager::ImagerRawFrame::ImagerAlignment::CLOCK_ALIGNED));

                if (m_rawFrames.size() > 1)
                {
                    m_rawFrames.at (1).alignment = royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
                }
            }
            else
            {
                if (m_rawFrames.size())
                {
                    m_rawFrames.back().isEndOfLinkedMeasurement = false;
                }
                m_rawFrames.push_back (royale::imager::ImagerRawFrame (royale::imager::ImagerRawFrame::MODFREQ_AUTO(),
                                       true, true, true, true, royale::imager::ImagerRawFrame::ImagerDutyCycle::DC_0, grayscaleExpo, 0u,
                                       m_rawFrames.size() ? royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED :
                                       royale::imager::ImagerRawFrame::ImagerAlignment::CLOCK_ALIGNED));
            }
        }
    }

    void setImage (uint16_t columns, uint16_t rows)
    {
        m_imageColumns = columns;
        m_imageRows = rows;
    }

    void setROI (uint16_t roiCMin, uint16_t roiRMin)
    {
        m_roiCMin = roiCMin;
        m_roiRMin = roiRMin;
    }

    void setTargetRate (uint16_t framerate)
    {
        m_targetRate = framerate;
    }

    void setRawFrameRate (uint16_t rawFrameRate)
    {
        m_rawFrameRate = rawFrameRate;
    }

    void setExposureTime (uint32_t expoTime, size_t startId = 0, size_t endId = std::numeric_limits<size_t>::max())
    {
        size_t index = 0;

        for (auto &rf : m_rawFrames)
        {
            if (!rf.grayscale && index >= startId && index <= endId)
            {
                rf.exposureTime = expoTime;
            }
            index++;
        }
    }

    void setDutyCycle (royale::imager::ImagerRawFrame::ImagerDutyCycle dutyCycle)
    {
        for (auto &rf : m_rawFrames)
        {
            if (!rf.grayscale)
            {
                rf.dutyCycle = dutyCycle;
            }
        }
    }
};

class UseCaseForM2450 : public ImagerUseCaseFourPhase
{
public:
    explicit UseCaseForM2450 (uint16_t targetRate = 50u,
                              size_t rawFrameCount = 4,
                              uint32_t expoTime = 1000u,
                              uint32_t freqFirst = 80320000u,
                              uint32_t secondFreq = 60240000u,
                              uint32_t grayscaleExpo = 0u,
                              bool grayscaleFirst = false,
                              bool enableSSC = false,
                              double ssc_freq = 0.,
                              double ssc_kspread = 0.,
                              double ssc_deltaFirst = 0.,
                              double ssc_deltaSecond = 0.) :
        ImagerUseCaseFourPhase (targetRate,
                                rawFrameCount,
                                expoTime,
                                freqFirst,
                                secondFreq,
                                grayscaleExpo,
                                grayscaleFirst,
                                enableSSC,
                                ssc_freq,
                                ssc_kspread,
                                ssc_deltaFirst,
                                ssc_deltaSecond)
    {
        m_imageColumns = 352;
        m_imageRows = 287;
        m_roiCMin = 0;
        m_roiRMin = 0;
    }
};

class UseCaseAlternateRF_M2450 : public UseCaseForM2450
{
public:
    explicit UseCaseAlternateRF_M2450 (size_t mbCount) : UseCaseForM2450 (5u, 4, 134u, 30000000u, 0u)
    {
        m_enableMixedMode = true;
        auto sscFreq = m_rawFrames.front().ssc_freq;

        while (--mbCount)
        {
            //clone a MB and change ssc_freq as this is actually not used because SSC is off,
            //but it will be used to check for raw frame equality
            sscFreq = sscFreq + (double) mbCount;

            for (size_t firstMB_rfIdx = 0; firstMB_rfIdx < 4; firstMB_rfIdx++)
            {
                royale::imager::ImagerRawFrame newRF = m_rawFrames.at (firstMB_rfIdx);
                newRF.ssc_freq = sscFreq;
                m_rawFrames.emplace_back (newRF);
            }
        }
    }
};

class UseCaseAlternateRF_M2452 : public UseCaseForM2450
{
public:
    explicit UseCaseAlternateRF_M2452 (size_t mbCount) : UseCaseForM2450 (5u, 4, 135u, 60240000, 0u)
    {
        m_imageColumns = 32;
        m_imageRows = 32;
        m_enableMixedMode = true;
        auto sscFreq = m_rawFrames.front().ssc_freq;

        while (--mbCount)
        {
            //clone a MB and change ssc_freq as this is actually not used because SSC is off,
            //but it will be used to check for raw frame equality
            sscFreq = sscFreq + (double) mbCount;

            for (size_t firstMB_rfIdx = 0; firstMB_rfIdx < 4; firstMB_rfIdx++)
            {
                royale::imager::ImagerRawFrame newRF = m_rawFrames.at (firstMB_rfIdx);
                newRF.ssc_freq = sscFreq;
                m_rawFrames.emplace_back (newRF);
            }
        }
    }
};

class UseCaseCalibration_M2450 : public ImagerUseCaseFourPhase
{
public:
    explicit UseCaseCalibration_M2450 (uint32_t expoModulated = 2000u, uint32_t expoGray = 27u) :
        ImagerUseCaseFourPhase (5u, 8, expoModulated, 80320000u, 60240000u, expoGray, true)
    {
        m_imageColumns = 224;
        m_imageRows = 172;
        m_rawFrames.front().isEndOfLinkedRawFrames = true;
        m_rawFrames.back().isEndOfLinkedMeasurement = false;

        auto rf1 = m_rawFrames.back();
        rf1.exposureTime = expoGray;
        rf1.dutyCycle = royale::imager::ImagerRawFrame::ImagerDutyCycle::DC_0;
        rf1.phaseAngle = 0u;
        rf1.isStartOfLinkedRawFrames = true;
        m_rawFrames.emplace_back (rf1);

        auto rf2 = m_rawFrames.back();
        rf2.dutyCycle = royale::imager::ImagerRawFrame::ImagerDutyCycle::DC_AUTO;
        rf2.isEndOfLinkedMeasurement = true;
        m_rawFrames.emplace_back (rf2);
    }
};

class UseCaseForM2452 : public ImagerUseCaseFourPhase
{
public:
    explicit UseCaseForM2452 (bool enableSSC = false, uint32_t expoGray = 0u) :
        ImagerUseCaseFourPhase (50u, 4, 100u, 80320000u, 60240000u, expoGray, true, enableSSC,
                                enableSSC ? 10000. : 0., enableSSC ? 0.5 : 0., enableSSC ? 0.0125 : 0.)
    {
        m_imageColumns = 224;
        m_imageRows = 172;
        m_roiCMin = 0;
        m_roiRMin = 0;
    }
};

/**
 * This use case is similar to a royale::usecase::UseCaseCalibration.
 */
class UseCaseCalibrationForM2452 : public ImagerUseCaseFourPhase
{
public:
    explicit UseCaseCalibrationForM2452 () :
        ImagerUseCaseFourPhase (5u, 8, 2000u, 80320000u, 60240000u, 200u, true, false,
                                0.,  0., 0.)
    {
        m_imageColumns = 224;
        m_imageRows = 172;
        m_roiCMin = 0;
        m_roiRMin = 0;

        m_rawFrames.back().isEndOfLinkedMeasurement = false;

        auto rfGray1 = m_rawFrames.front();
        rfGray1.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
        m_rawFrames.emplace_back (rfGray1);

        auto rfGray2 = m_rawFrames.front();
        rfGray2.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
        m_rawFrames.emplace_back (rfGray2);

        m_rawFrames.back().isEndOfLinkedMeasurement = true;
    }

    void setSecondToLastExpo (uint32_t expo)
    {
        m_rawFrames.at (m_rawFrames.size() - 2).exposureTime = expo;
    }
};

class UseCaseCustomFreq_M2450 : public ImagerUseCaseFourPhase
{
public:
    explicit UseCaseCustomFreq_M2450 (bool enableSSC = true, uint32_t expoGray = 1000u) :
        ImagerUseCaseFourPhase (45u, 4, 1000u, 29500000u, 0u, expoGray, false, enableSSC, 10000., 0.5, 0.0125)
    {
        m_imageColumns = 352;
        m_imageRows = 287;
        m_roiCMin = 0;
        m_roiRMin = 0;
    }

    void setFrequency (uint32_t freq)
    {
        for (auto &rf : m_rawFrames)
        {
            if (rf.grayscale)
            {
                continue;
            }
            if (80320000u == freq)
            {
                rf.ssc_delta = 0.0125;
            }
            if (60240000u == freq)
            {
                rf.ssc_delta = 0.0166;
            }
            rf.modulationFrequency = freq;
        }
    }
};

class UseCaseCustomFreq_M2452 : public UseCaseCustomFreq_M2450
{
public:
    explicit UseCaseCustomFreq_M2452 (bool enableSSC = false, uint32_t expoGray = 0u) :
        UseCaseCustomFreq_M2450 (enableSSC, expoGray)
    {
        m_imageColumns = 224;
        m_imageRows = 172;
    }
};

class UseCaseCustomMM_M2450 : public UseCaseForM2450
{
public:
    explicit UseCaseCustomMM_M2450 (uint16_t targetRate = 5u,
                                    uint16_t ratio = 1u,
                                    uint32_t expoHT = 1000u,
                                    uint32_t expoES = 1000u,
                                    uint32_t freqHT = 30000000u,
                                    uint32_t freqES1 = 20200000u,
                                    uint32_t freqES2 = 20600000u,
                                    uint32_t grayscaleExpo = 0u,
                                    bool grayscaleFirst = false,
                                    bool enableSSC = false,
                                    double ssc_freq = 0.,
                                    double ssc_kspread = 0.,
                                    double ssc_deltaFirst = 0.,
                                    double ssc_deltaSecond = 0.,
                                    bool isIrregular = false) :
        UseCaseForM2450 (targetRate, 4, expoHT, freqHT, 0u, grayscaleExpo, grayscaleFirst,
                         enableSSC, ssc_freq, ssc_kspread, ssc_deltaFirst, ssc_deltaSecond)
    {
        m_enableMixedMode = true;

        while (--ratio)
        {
            for (size_t rfIdx = 0; rfIdx < 5; rfIdx++)
            {
                auto rf = m_rawFrames.at (rfIdx);
                m_rawFrames.emplace_back (rf);
            }
        }

        if (grayscaleFirst)
        {
            auto rf = m_rawFrames.front();
            rf.alignment = isIrregular ? royale::imager::ImagerRawFrame::ImagerAlignment::NEXTSTOP_ALIGNED :
                           royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
            m_rawFrames.emplace_back (rf);

            for (size_t rfIdx = 1; rfIdx < 5; rfIdx++)
            {
                auto rf = m_rawFrames.at (rfIdx);
                rf.modulationFrequency = freqES1;
                rf.ssc_delta = ssc_deltaSecond;
                rf.exposureTime = expoES;
                if (isIrregular)
                {
                    rf.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::NEXTSTOP_ALIGNED;
                }
                else if (1 == rfIdx)
                {

                    rf.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
                }
                m_rawFrames.emplace_back (rf);
            }
            for (size_t rfIdx = 1; rfIdx < 5; rfIdx++)
            {
                auto rf = m_rawFrames.at (rfIdx);
                rf.modulationFrequency = freqES2;
                rf.ssc_delta = ssc_deltaSecond;
                rf.exposureTime = expoES;
                if (isIrregular)
                {
                    rf.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::NEXTSTOP_ALIGNED;
                }
                else if (1 == rfIdx)
                {

                    rf.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
                }
                m_rawFrames.emplace_back (rf);
            }
        }
        else
        {
            for (size_t rfIdx = 0; rfIdx < 4; rfIdx++)
            {
                auto rf = m_rawFrames.at (rfIdx);
                rf.modulationFrequency = freqES1;
                rf.ssc_delta = ssc_deltaSecond;
                rf.exposureTime = expoES;
                if (!rfIdx)
                {
                    rf.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
                }
                m_rawFrames.emplace_back (rf);
            }
            for (size_t rfIdx = 0; rfIdx < 4; rfIdx++)
            {
                auto rf = m_rawFrames.at (rfIdx);
                rf.modulationFrequency = freqES2;
                rf.ssc_delta = ssc_deltaSecond;
                rf.exposureTime = expoES;
                if (!rfIdx)
                {
                    rf.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
                }
                m_rawFrames.emplace_back (rf);
            }

            auto rf = m_rawFrames.at (4);
            m_rawFrames.emplace_back (rf);
        }
    }
};

class UseCaseCustomMM_M2452 : public UseCaseCustomMM_M2450
{
public:
    explicit UseCaseCustomMM_M2452 (uint16_t targetRate = 5u,
                                    uint16_t ratio = 1u,
                                    uint32_t expoHT = 1000u,
                                    uint32_t expoES = 1000u,
                                    uint32_t freqHT = 30000000u,
                                    uint32_t freqES1 = 20200000u,
                                    uint32_t freqES2 = 20600000u,
                                    uint32_t grayscaleExpo = 0u,
                                    bool grayscaleFirst = false,
                                    bool enableSSC = false,
                                    double ssc_freq = 0.,
                                    double ssc_kspread = 0.,
                                    double ssc_deltaFirst = 0.,
                                    double ssc_deltaSecond = 0.,
                                    bool isIrregular = false) :
        UseCaseCustomMM_M2450 (targetRate, ratio, expoHT, expoES, freqHT, freqES1, freqES2, grayscaleExpo, grayscaleFirst,
                               enableSSC, ssc_freq, ssc_kspread, ssc_deltaFirst, ssc_deltaSecond, isIrregular)
    {
        m_imageColumns = 224;
        m_imageRows = 172;
    }
};

class UseCaseCustomInterleaved_M2450 : public UseCaseForM2450
{
public:
    explicit UseCaseCustomInterleaved_M2450 (uint16_t targetRate = 5u,
            uint16_t ratio = 1u, uint32_t expo = 1000u,
            uint32_t freqHT = 30000000u,
            uint32_t freqES1 = 20200000u,
            uint32_t freqES2 = 20600000u,
            bool grayscaleFirst = true,
            bool enableSSC = false,
            double ssc_freq = 0.,
            double ssc_kspread = 0.,
            double ssc_deltaFirst = 0.,
            double ssc_deltaSecond = 0.) :
        UseCaseForM2450 (targetRate, 4, expo, freqHT, 0u, expo, grayscaleFirst,
                         enableSSC, ssc_freq, ssc_kspread, ssc_deltaFirst, ssc_deltaSecond)
    {
        m_enableMixedMode = true;

        ratio--; //for the interleaved one

        while (--ratio)
        {
            for (size_t rfIdx = 0; rfIdx < 5; rfIdx++)
            {
                auto rf = m_rawFrames.at (rfIdx);
                m_rawFrames.emplace_back (rf);
            }
        }

        auto rf = m_rawFrames.front();
        rf.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::STOP_ALIGNED;
        rf.isEndOfLinkedRawFrames = true;
        m_rawFrames.emplace_back (rf);

        //first part of the 8+1 measurement
        for (size_t rfIdx = 1; rfIdx < 5; rfIdx++)
        {
            auto rf = m_rawFrames.at (rfIdx);
            rf.modulationFrequency = freqES1;
            rf.ssc_delta = ssc_deltaSecond;
            rf.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::STOP_ALIGNED;
            m_rawFrames.emplace_back (rf);
        }

        //as the second part comes later this is not the end...
        m_rawFrames.back().isEndOfLinkedMeasurement = false;

        //the interleaved 4+1 measurement
        for (size_t rfIdx = 0; rfIdx < 5; rfIdx++)
        {
            auto rf = m_rawFrames.at (rfIdx);
            m_rawFrames.emplace_back (rf);
        }

        //as the 4+1 measurement is interleaved it is not the end...
        m_rawFrames.back().isEndOfLinkedMeasurement = false;


        //second part of the 8+1 measurement
        for (size_t rfIdx = 1; rfIdx < 5; rfIdx++)
        {
            auto rf = m_rawFrames.at (rfIdx);
            rf.modulationFrequency = freqES2;
            rf.ssc_delta = ssc_deltaSecond;
            if (!rfIdx)
            {
                rf.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
            }
            m_rawFrames.emplace_back (rf);
        }
    }
};

class UseCaseCustomInterleaved_M2452 : public UseCaseCustomInterleaved_M2450
{
public:
    explicit UseCaseCustomInterleaved_M2452 (uint16_t targetRate = 5u,
            uint16_t ratio = 1u,
            uint32_t expo = 135u,
            uint32_t freqHT = 80320000,
            uint32_t freqES1 = 80320000,
            uint32_t freqES2 = 80320000,
            bool grayscaleFirst = true,
            bool enableSSC = false,
            double ssc_freq = 0.,
            double ssc_kspread = 0.,
            double ssc_deltaFirst = 0.,
            double ssc_deltaSecond = 0.) :
        UseCaseCustomInterleaved_M2450 (targetRate, ratio, expo, freqHT, freqES1, freqES2, grayscaleFirst,
                                        enableSSC, ssc_freq, ssc_kspread, ssc_deltaFirst, ssc_deltaSecond)
    {
        m_imageColumns = 224;
        m_imageRows = 172;
    }
};

/**
 *  Imager use case equivalent to Royale 3.x use case with 2 FrameGroups
 * (RFS(FG1) = {gs(0),gs(1),gs(2)}, RFS(FG2) = {gs(3),gs(4),gs(5)} |
 *              gs(i) is a grayscale rawframe with sequence index i)
 */
class UseCaseTwoGrayscaleFGs_M2452 : public ImagerUseCaseFourPhase
{
public:
    explicit UseCaseTwoGrayscaleFGs_M2452() :
        ImagerUseCaseFourPhase (10u, 0, 100u, 80320000u, 0u, 200u)
    {
        m_imageColumns = 224;
        m_imageRows = 172;
        m_roiCMin = 0;
        m_roiRMin = 0;
        m_enableMixedMode = true;
        m_rawFrames.front().isEndOfLinkedMeasurement = false;

        {
            auto rf = m_rawFrames.front();
            rf.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
            m_rawFrames.emplace_back (rf);
        }

        {
            auto rf = m_rawFrames.front();
            rf.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
            rf.isEndOfLinkedMeasurement = true;
            m_rawFrames.emplace_back (rf);
        }

        {
            auto rf = m_rawFrames.front();
            m_rawFrames.emplace_back (rf);
        }

        {
            auto rf = m_rawFrames.front();
            rf.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
            m_rawFrames.emplace_back (rf);
        }

        {
            auto rf = m_rawFrames.back();
            rf.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
            rf.isEndOfLinkedMeasurement = true;
            m_rawFrames.emplace_back (rf);
        }
    }
};

/**
 * Imager use case equivalent to Royale 3.x use case with
 * 2 streams, 2 FrameGroups, and a shared raw frame.
 *
 * With grayscaleAtEnd = false, and onlyGrayscaleForFG2 = false:
 * RFS(FG1) = { 4phase(0, 1, 2, 3), gs(4) }, RFS(FG2) = { gs(4), 4phase(5, 6, 7, 8) }
 *
 * With grayscaleAtEnd = true, and onlyGrayscaleForFG2 = false:
 * RFS(FG1) = { 4phase(0, 1, 2, 3), gs(8) }, RFS(FG2) = { gs(8), 4phase(4, 5, 6, 7) }
 *
 * With onlyGrayscaleForFG2 = true:
 * RFS(FG1) = { 4phase(0, 1, 2, 3), gs(4) }, RFS(FG2) = { gs(4) }
 * Note that the imager use case equivalent is basically a standard 4+1
 * measurement (UseCaseForM2452 with expoGray != 0).
 *
 * gs(i) is a grayscale rawframe with sequence index i
 * 4phase is a modulated set of rawframe with indices i to i+3
 */
class UseCaseTwoFGsSharedRawFrame_M2452 : public ImagerUseCaseFourPhase
{
public:
    explicit UseCaseTwoFGsSharedRawFrame_M2452 (bool grayscaleAtEnd = false, bool onlyGrayscaleForFG2 = false) :
        ImagerUseCaseFourPhase (10u, 4, 200u, 80320000u, 0u, 200u)
    {
        m_imageColumns = 224;
        m_imageRows = 172;
        m_roiCMin = 0;
        m_roiRMin = 0;
        m_enableMixedMode = true;

        if (!onlyGrayscaleForFG2)
        {
            m_rawFrames.back().isEndOfLinkedMeasurement = false;

            for (size_t rfIdx = 0; rfIdx < 4; rfIdx++)
            {
                auto rf = m_rawFrames.at (rfIdx);
                rf.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
                m_rawFrames.emplace_back (rf);
            }

            if (grayscaleAtEnd)
            {
                auto rf = m_rawFrames.at (4);
                m_rawFrames.emplace_back (rf);
                m_rawFrames.erase (m_rawFrames.begin() + 4);
            }

            m_rawFrames.back().isEndOfLinkedMeasurement = true;
        }
    }
};

/**
 * A use case with 2 streams, 4 FrameGroups, and shared raw frames between (FG1, FG2) and (FG3, FG4).
 *
 * RFS(FG1) = { 4phase(0, 1, 2, 3), gs(4) }, RFS(FG2) = { gs(4), 4phase(5, 6, 7, 8) }
 * RFS(FG3) = { 4phase(9, 10, 11, 12), gs(13) }, RFS(FG4) = { gs(13), 4phase(14, 15, 16, 17) } |
 *
 * gs(i) is a grayscale rawframe with sequence index i
 * 4phase is a modulated set of rawframe with indices i to i+3
 */
class UseCaseFourFGsTwoSharedRawFrames_M2452 : public ImagerUseCaseFourPhase
{
public:
    explicit UseCaseFourFGsTwoSharedRawFrames_M2452 () :
        ImagerUseCaseFourPhase (10u, 4, 200u, 80320000u, 0u, 200u)
    {
        m_imageColumns = 224;
        m_imageRows = 172;
        m_roiCMin = 0;
        m_roiRMin = 0;
        m_enableMixedMode = true;

        m_rawFrames.back().isEndOfLinkedMeasurement = false;
        m_rawFrames.back().isEndOfLinkedRawFrames = false;

        for (size_t rfIdx = 0; rfIdx < 4; rfIdx++)
        {
            auto rf = m_rawFrames.at (rfIdx);
            rf.isStartOfLinkedRawFrames = false;
            rf.alignment = royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED;
            m_rawFrames.emplace_back (rf);
        }

        m_rawFrames.back().isEndOfLinkedMeasurement = true;
        m_rawFrames.back().isEndOfLinkedRawFrames = true;

        for (size_t rfIdx = 0; rfIdx < 9; rfIdx++)
        {
            auto rf = m_rawFrames.at (rfIdx);
            rf.alignment = rfIdx ? royale::imager::ImagerRawFrame::ImagerAlignment::START_ALIGNED :
                           royale::imager::ImagerRawFrame::ImagerAlignment::CLOCK_ALIGNED;
            m_rawFrames.emplace_back (rf);
        }

        m_rawFrames.back().isEndOfLinkedMeasurement = true;
    }
};
