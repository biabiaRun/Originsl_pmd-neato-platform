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

#include <usecase/UseCaseDefinition.hpp>
#include <cstdint>

namespace royale
{
    namespace common
    {
        /**
         * Interface for flow control strategies.
         *
         * The USB camera modules have an internal buffer for transferring data from the imager.
         * This may overflow, leading to frame drops.
         * Flow control, as implemented here, works by setting the frame rate of the individual
         * exposures (i.e. the raw frame rate). With a lowered raw frame rate the imager can fill
         * the buffer at a lower rate which may allow the USB transfers to keep up with it.
         * From the application perspective a high raw frame rate is desireable as the phase images should
         * be captured in as close succession as possible (to keep errors introduced by motion small).
         * So the goal of the flow control strategy should be to get the raw frame as high
         * as possible without introducing frame drops.
         *
         * Note that the flow control strategy will only compute a raw frame rate; to make this work
         * as intended some support from the imager is required, especially if the internal buffer is
         * smaller than a complete frame. In this case readout is done in chunks smaller than a frame
         * while USB transfers happen simultaneously so the imager must slow down the rate at which chunks
         * are produced (e.g. by slowing the actual readout or by pausing between chunks) to avoid buffer
         * overruns within a frame.
         */
        class IFlowControlStrategy
        {
        public:
            virtual ~IFlowControlStrategy() {}

            /**
            * Compute raw frame rate to be set for this usecase.
            * The current usecase definition is passed as a parameter. Different strategy
            * implementations may use additional information not contained in the usecase;
            * this has to be passed by other means (e.g. in the constructor).
            *
            * \param useCase currently active UCD
            * \return raw frame rate setting for the imager
            */
            virtual uint16_t getRawFrameRate (const royale::usecase::UseCaseDefinition &useCase) = 0;

            enum : uint16_t
            {
                /** Value for getRawFrameRate meaning to use the maximum possible rate. **/
                NoFlowControl = 0
            };
        };
    }
}
