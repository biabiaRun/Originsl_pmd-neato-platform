/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

namespace royale
{
    namespace processing
    {
        class IRefineExposureTime
        {
        public:
            virtual ~IRefineExposureTime() = default;

            /**
            * Retrieves a scaled exposure time based on a predefined scaling factor.
            * This is used to even out illumination and chip variations.
            * @return refined exposure time.
            */
            virtual uint32_t getRefinedExposureTime (const uint32_t exposureTime,
                    const royale::Pair<uint32_t, uint32_t> &exposureLimits) = 0;
        };
    }
}
