/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <collector/IFrameCaptureListener.hpp>
#include <collector/IFrameCaptureReleaser.hpp>

#include <common/exceptions/NotImplemented.hpp>

#include <royale/IDepthDataListener.hpp>

namespace royale
{
    namespace stub
    {
        namespace processing
        {
            /**
             * Simulates data that has been captured from an imager and collected in to the sets to be
             * passed to the processing.
             *
             * This is for testing the thread-handling and glue layer provided by Royale, not the
             * Spectre libraries.  It currently doesn't generate any image data in the
             * ICapturedRawFrames, just the ICapturedRawFrames themselves.
             */
            class FrameGeneratorStub : public royale::collector::IFrameCaptureReleaser
            {
            public:
                FrameGeneratorStub();

                void releaseCapturedFrames (std::vector<royale::common::ICapturedRawFrame *> frames) override;

                void setFrameCaptureListener (std::weak_ptr<royale::collector::IFrameCaptureListener> processing);

                /**
                 * Sends to the listener a group of frames corresponding to a FrameGroup in the
                 * given stream.
                 *
                 * These correspond to the set that is returned by
                 * UseCaseDefinition::getRawFrameSetIndices (streamId, frameGroup).
                 *
                 * The temperature will be a fixed value.
                 *
                 * The timestamp is monotonically increasing.
                 */
                void generateCallback (
                    const royale::usecase::UseCaseDefinition &definition,
                    royale::StreamId streamId,
                    std::size_t frameGroup = 0);

                /**
                 * Configures the use case to the minimum size needed.
                 *
                 * The BufferGeneratorStub's equivalent of this reduces the time that the
                 * FrameCollector's unit tests take by about 50%.
                 */
                void reduceToMinimumImage (royale::usecase::UseCaseDefinition *useCase);

            private:
                std::unique_ptr<royale::common::IPseudoDataInterpreter> m_pdi;
                std::weak_ptr<royale::collector::IFrameCaptureListener> m_processing;
            };
        }
    }
}
