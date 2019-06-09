/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <FrameGeneratorStub.hpp>

#include <processing/ProcessingSimple.hpp>

#include <royale/DepthData.hpp>

#include <common/exceptions/NotImplemented.hpp>
#include <common/MakeUnique.hpp>
#include <common/PseudoDataTwelveBitCalculator.hpp>

#include <vector>

using namespace royale;
using namespace royale::collector;
using namespace royale::common;
using namespace royale::processing;
using namespace royale::stub::processing;

namespace
{
    /**
     * Simple implementation of an ICapturedBuffer.
     *
     * The name is an abbreviation of FrameGeneratorStubCapturedFrame.
     */
    class FgsCapturedFrame : public ICapturedRawFrame
    {
    public:
        explicit FgsCapturedFrame (std::size_t size) :
            m_imageData (size)
        {
        }

        uint16_t *getImageData() override
        {
            return m_imageData.data();
        }

        const uint16_t *getPseudoData() const override
        {
            // As noted on UnimplementedPseudoDataHandler, the Processing doesn't need the
            // pseudodata. The Recording will need it, but the FrameGeneratorStub is not yet used
            // for testing the recording component.
            throw NotImplemented();
        }

    private:
        std::vector<uint16_t> m_imageData;
    };

    /**
     * CapturedUseCase must contain an IPseudoDataInterpreter, but it's currently not used by the
     * Processing.  This stub just throws for every getter function.
     *
     * \todo ROYAL-2105 Remove CapturedUseCase::getInterpreter(), if it's unused.
     */
    class UnimplementedPseudoDataHandler : public PseudoDataTwelveBitCalculator
    {
    public:
        ~UnimplementedPseudoDataHandler() = default;

        UnimplementedPseudoDataHandler *clone() override
        {
            return new UnimplementedPseudoDataHandler (*this);
        }

        uint16_t getFrameNumber (const ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }

        uint16_t getReconfigIndex (const ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }

        uint16_t getSequenceIndex (const ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }

        uint8_t getBinning (const ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }

        uint16_t getHorizontalSize (const ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }

        uint16_t getVerticalSize (const ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }

        float getImagerTemperature (const ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }

        void getTemperatureRawValues (const ICapturedRawFrame &frame,
                                      uint16_t &vRef1,
                                      uint16_t &vNtc1,
                                      uint16_t &vRef2,
                                      uint16_t &vNtc2,
                                      uint16_t &offset) const override
        {
            throw NotImplemented();
        }

        uint16_t getRequiredImageWidth() const override
        {
            throw NotImplemented();
        }

        void getEyeSafetyError (const common::ICapturedRawFrame &frame, uint32_t &eyeError) const override
        {
            eyeError = 0u;
        }

        bool supportsExposureFromPseudoData() const override
        {
            return false;
        }

        uint32_t getExposureTime (const royale::common::ICapturedRawFrame &frame, uint32_t modulationFrequency)  const override
        {
            throw NotImplemented();
        }
    };
}

FrameGeneratorStub::FrameGeneratorStub() :
    m_pdi {makeUnique<UnimplementedPseudoDataHandler>() }
{
}

void FrameGeneratorStub::releaseCapturedFrames (std::vector<royale::common::ICapturedRawFrame *> frames)
{
    for (auto frame : frames)
    {
        delete frame;
    }
}

void FrameGeneratorStub::setFrameCaptureListener (std::weak_ptr<royale::collector::IFrameCaptureListener> processing)
{
    m_processing = processing;
}

void FrameGeneratorStub::generateCallback (
    const royale::usecase::UseCaseDefinition &definition,
    royale::StreamId streamId,
    std::size_t frameGroup)
{
    uint16_t width, height;
    definition.getImage (width, height);
    const std::size_t frameSize = width * height;
    std::vector<common::ICapturedRawFrame *> frames;
    for (auto rfsIdx : definition.getRawFrameSetIndices (streamId, 0))
    {
        for (auto i = 0u; i < definition.getRawFrameSets().at (rfsIdx).countRawFrames(); i++)
        {
            frames.push_back (new FgsCapturedFrame {frameSize});
            memset (frames.back()->getImageData(), 0, frameSize * sizeof (uint16_t));
        }
    }
    const auto temperature = 20.f;
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds> (CapturedUseCase::CLOCK_TYPE::now().time_since_epoch());
    royale::Vector<uint32_t> exposures;
    for (auto limits : definition.getExposureLimits())
    {
        exposures.emplace_back (limits.second);
    }
    auto capturedCase = common::makeUnique<CapturedUseCase> (m_pdi.get(), temperature, timestamp, exposures);

    UnimplementedPseudoDataHandler pdi;

    auto processing = m_processing.lock();
    if (!processing)
    {
        throw LogicError ("The processing instance to test hasn't been set");
    }
    processing->captureCallback (frames, definition, streamId, std::move (capturedCase));
}

void FrameGeneratorStub::reduceToMinimumImage (royale::usecase::UseCaseDefinition *useCase)
{
    useCase->setImage (16u, 16u);
}
