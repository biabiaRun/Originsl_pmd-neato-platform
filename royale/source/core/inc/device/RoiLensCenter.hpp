/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>

#include <cstdint>
#include <memory>
#include <usecase/UseCaseDefinition.hpp>

namespace royale
{
    namespace device
    {
        /**
         * Calculate the Region Of Interest for a given image size, so that it's aligned to the lens
         * center.
         *
         * This class implements a calculation with two offsets in each direction.
         *
         * First the designCenter, which is chosen for each type of device, this could be the center
         * of the sensor, but may be offset depending on the module.
         *
         * Second the lensOffset, which is measured during the calibration of each device, and can
         * be changed by L3 users.
         *
         * Given these offsets and a ROI size, it will calculate where on the sensor the ROI is.
         *
         * This class knows that the minimum row and column on the imager are row zero and column
         * zero, but it doesn't know what the maximums are.  Therefore it can verify that the values
         * don't underflow a uint16_t, but the software imager's verifyUseCase needs to compare the
         * sizes to the hardware imager.
         */
        class RoiLensCenter
        {
        public:
            ROYALE_API explicit RoiLensCenter (uint16_t designColumn, uint16_t designRow);

            /**
             * Change the offset that is provided by the calibration or L3 user.
             */
            ROYALE_API void setLensOffset (int16_t pixelColumn, int16_t pixelRow);

            /**
             * Must be called during the verification stage before getRoiCorner, returns SUCCESS if everything is OK,
             * or REGION if either of the values that getRoiCorner will return would be negative.
             */
            ROYALE_API royale::usecase::VerificationStatus verifyUseCase (const royale::usecase::UseCaseDefinition &ucd) const;

            /**
             * Returns the coordinates of one corner of the image.
             *
             * \param ucd Provides the size of the ROI
             * \param activeColumnOffset The zero-based index of the first active column
             * \param activeRowOffset The zero-based index of the first active row
             */
            ROYALE_API void getRoiCorner (const royale::usecase::UseCaseDefinition &ucd,
                                          uint16_t &activeColumnOffset,
                                          uint16_t &activeRowOffset) const;

        private:
            const uint16_t m_designCenterCol;
            const uint16_t m_designCenterRow;

            int16_t m_lensOffsetCol;
            int16_t m_lensOffsetRow;

            royale::usecase::VerificationStatus verifySizeAndLensCombination (uint16_t sizeColumns,
                    uint16_t sizeRows,
                    int16_t offsetCol,
                    int16_t offsetRow) const;
        };
    }
}
