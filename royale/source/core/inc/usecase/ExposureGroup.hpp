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

#include <royale/String.hpp>
#include <royale/Pair.hpp>
#include <cstdint>

namespace royale
{
    namespace usecase
    {
        using ExposureGroupIdx = uint16_t;

        /***
         * Exposure settings common to an exposure group.
         * These are kept in a vector in the UseCaseDefinition and an index in RawFrameSet refers
         * to that vector.
         * All raw frame sets belonging to an exposure group have the same exposure settings.
         *
         * In a typical non-mixed mode usecase, each raw frame set has its own exposure group; this
         * allows setting the exposure for grayscale and for the (one or two) blocks of phase
         * measurements independently.
         *
         * In a typical mixed-mode usecase, looking at only the first frame group of each stream,
         * each raw frame set has its own exposure group (similar to a typical non-mixed mode
         * usecase).  However, in a stream with multiple frame groups, all of the frame groups share
         * the same set of exposure groups, so that the corresponding frames have the same exposure
         * times.  This is enforced by UseCaseDefinition::verifyClassInvariants().
         */
        class ExposureGroup
        {
        public:
            bool operator== (const ExposureGroup &) const;
            bool operator!= (const ExposureGroup &) const;

            royale::String                   m_name;
            royale::Pair<uint32_t, uint32_t> m_exposureLimits;   //!< Holds the limits of the allowed exposure time as pair of [minExposure, maxExposure]
            uint32_t                         m_exposureTime;     //!< Exposure time (in us)
        };
    }
}
