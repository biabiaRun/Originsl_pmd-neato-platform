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

#include <collector/IFrameCollector.hpp>
#include <hal/ICapturedBuffer.hpp>
#include <hal/ITemperatureSensor.hpp>

#include <memory>
#include <vector>

namespace royale
{
    namespace stub
    {
        namespace hal
        {
            /**
             * Pre-generated buffer generator.  This is a utility for speed testing the frame
             * collectors, removing the overhead of generating buffers.  It takes buffers from
             * another source, for example the BufferGeneratorStubM2450, and then sends all of the
             * buffers to the frame collector every time, every time sendAllBuffers() is called.
             *
             * When calling IFrameCollector::executeUseCase, use the actual buffer generator (this
             * class' m_releaser) as the first argument.  That does not fit the assumptions
             * documented in IFrameCollector's interface, however it's sufficient for the purposes
             * of a speed.  The other option would be for BufferGeneratorPregen to implement the
             * full IBridgeDataReceiver interface and require that m_releaser is an
             * IBridgeDataReceiver, so that it can pass the executeUseCase() call to m_releaser.
             * That then opens questions about which of the other IBridgeDataReceiver functions
             * would be have pass-through implementions.
             */
            class BufferGeneratorPregen : public royale::hal::IBufferCaptureListener,
                public royale::hal::IBufferCaptureReleaser
            {
            public:
                explicit BufferGeneratorPregen (std::shared_ptr<royale::hal::IBufferCaptureReleaser> releaser) :
                    m_releaser {releaser},
                    m_listener {nullptr}
                {
                }

                ~BufferGeneratorPregen() override = default;

                /** Same name and effect as IBridgeDataReceiver's method */
                void setBufferCaptureListener (royale::hal::IBufferCaptureListener *listener)
                {
                    m_listener = listener;
                }

                void bufferCallback (royale::hal::ICapturedBuffer *buffer) override
                {
                    m_buffers.push_back (buffer);
                }

                void releaseAllBuffers() override
                {
                    for (auto buffer : m_buffers)
                    {
                        m_releaser->queueBuffer (buffer);
                    }
                    m_buffers.clear();
                }

                void queueBuffer (royale::hal::ICapturedBuffer *buffer) override
                {
                    // ignored, for this speed test we assume ValidBufferCallbacks passed
                    // and the IFrameCollector behaves predictably
                }

                void sendAllBuffers()
                {
                    for (auto buffer : m_buffers)
                    {
                        m_listener->bufferCallback (buffer);
                    }
                }

            private:
                std::vector<royale::hal::ICapturedBuffer *> m_buffers;
                std::shared_ptr<royale::hal::IBufferCaptureReleaser> m_releaser;
                royale::hal::IBufferCaptureListener *m_listener;
            };
        }
    }
}
