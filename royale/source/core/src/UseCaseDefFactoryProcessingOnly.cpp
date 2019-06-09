/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usecase/UseCaseDefFactoryProcessingOnly.hpp>

#include <common/exceptions/InvalidValue.hpp>
#include <common/NarrowCast.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::usecase;

namespace
{
    /**
     * Helper class for constructing the UCD that will be sliced and returned by
     * UseCaseDefFactoryProcessingOnly.
     */
    class UcdProcessingOnly : public UseCaseDefinition
    {
    public:
        using ProcOnlyExpo = UseCaseDefFactoryProcessingOnly::ProcOnlyExpo;
        using ProcOnlyRFS = UseCaseDefFactoryProcessingOnly::ProcOnlyRFS;

        UcdProcessingOnly (
            royale::usecase::UseCaseIdentifier identifier,
            royale::Pair<uint16_t, uint16_t> imageSize,
            royale::Pair<uint16_t, uint16_t> frameRateLimits,
            uint16_t targetFrameRate,
            const royale::Vector<ProcOnlyExpo> &expoGroups,
            const royale::Vector<ProcOnlyRFS> &rfses) :
            UseCaseDefinition (frameRateLimits.second)
        {
            m_identifier = identifier;
            m_typeName = "ProcessingOnly";
            m_targetRate = targetFrameRate;
            m_enableSSC = false;
            m_imageColumns = imageSize.first;
            m_imageRows = imageSize.second;

            for (auto i = std::size_t (0u); i < expoGroups.size(); i++)
            {
                auto name = String ("group") + String::fromInt (narrow_cast<uint32_t> (i));
                m_exposureGroups.push_back (ExposureGroup {std::move (name), expoGroups[i].exposureLimits, expoGroups[i].exposureTime});
            }

            royale::Vector<RawFrameSet> rawFrameSets;
            for (const auto &rfs : rfses)
            {
                RawFrameSet::PhaseDefinition phaseDef;
                switch (rfs.frameCount)
                {
                    case 1:
                        phaseDef = RawFrameSet::PhaseDefinition::GRAYSCALE;
                        break;
                    case 4:
                        phaseDef = RawFrameSet::PhaseDefinition::MODULATED_4PH_CW;
                        break;
                    default:
                        throw InvalidValue ("Unsupported frame count");
                }
                rawFrameSets.emplace_back (rfs.modulationFrequency, phaseDef, RawFrameSet::DutyCycle::DC_AUTO, rfs.exposureGroupIdx);
            }

            constructNonMixedUseCase (std::move (rawFrameSets));
            verifyClassInvariants();
        }
    };
}

std::shared_ptr<UseCaseDefinition> UseCaseDefFactoryProcessingOnly::createUcd (
    royale::usecase::UseCaseIdentifier identifier,
    royale::Pair<uint16_t, uint16_t> imageSize,
    royale::Pair<uint16_t, uint16_t> frameRateLimits,
    uint16_t targetFrameRate,
    const royale::Vector<ProcOnlyExpo> &expoGroups,
    const royale::Vector<ProcOnlyRFS> &rfses)
{
    return std::make_shared<UcdProcessingOnly> (
               std::move (identifier),
               std::move (imageSize),
               std::move (frameRateLimits),
               targetFrameRate,
               expoGroups,
               rfses);
}

std::shared_ptr<UseCaseDefinition> UseCaseDefFactoryProcessingOnly::createUcd (const UseCaseDefinition &other)
{
    royale::Vector<ProcOnlyExpo> expoGroups;
    for (const auto &expo : other.getExposureGroups())
    {
        expoGroups.push_back (ProcOnlyExpo {expo.m_exposureLimits, expo.m_exposureTime});
    }

    royale::Vector<ProcOnlyRFS> rawFrameSets;
    for (const auto &rfs : other.getRawFrameSets())
    {
        rawFrameSets.push_back (ProcOnlyRFS {rfs.countRawFrames(), rfs.modulationFrequency, rfs.exposureGroupIdx});
    }

    uint16_t columns, rows;
    other.getImage (columns, rows);
    return createUcd (other.getIdentifier(), {columns, rows}, {other.getMinRate(), other.getMaxRate() }, other.getTargetRate(), expoGroups, rawFrameSets);
}
