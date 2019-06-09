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

#include <imager/ImagerRawFrame.hpp>

#include <chrono>
#include <vector>

namespace royale
{
    namespace imager
    {
        /**
        * The information about the sequence structure, frame rates, modulation and exposure for a
        * UseCase.
        */
        class ImagerUseCaseDefinition
        {
        public:
            /**
            * Default ctor. The only users of this are the ones that will overwrite the contents by
            * assigning a complete UseCaseDefinition.
            */
            IMAGER_EXPORT ImagerUseCaseDefinition();
            ImagerUseCaseDefinition (const ImagerUseCaseDefinition &iucd);
            bool operator== (const ImagerUseCaseDefinition &rhs) const;
            bool operator!= (const ImagerUseCaseDefinition &rhs) const;

            //! Get the target raw frame rates for all raw frames [Hz]
            uint16_t getRawFrameRate() const;

            /**
            * Get the target rate [in Hz] of the clock for ImagerRawFrame::ImagerAlignment::CLOCK_ALIGNED
            * frames.  For non-mixed mode use cases there will normally be one CLOCK_ALIGNED frame,
            * but mixed mode use cases are likely to have several.
            */
            IMAGER_EXPORT uint16_t getTargetRate() const;

            IMAGER_EXPORT const std::vector<ImagerRawFrame> &getRawFrames() const;

            /**
            * Get the width and height of the region of interest.
            *
            * \param columns   size x of the image
            * \param rows      size y of the image
            */
            void getImage (uint16_t &columns, uint16_t &rows) const;

            /**
            * Get the column and row index of the first active pixel.
            *
            * \param roiCMin   The zero-based index of the first active column
            * \param roiRMin   The zero-based index of the first active row
            */
            void getStartOfROI (uint16_t &roiCMin, uint16_t &roiRMin) const;

            //! Returns true if the spread spectrum feature is turned on for this use case
            bool getSSCEnabled() const;

            //! Returns true if the imager should enable the mixed mode operation
            IMAGER_EXPORT bool getMixedModeEnabled() const;

            /**
            * Compute the time between frames where the illumination is turned off.
            * When switching between use cases, there may have to be a pause
            * to ensure eye safety, this pause is calculated from this tail time.
            */
            IMAGER_EXPORT std::chrono::microseconds getTailTime() const;

        protected:
            uint16_t                       m_targetRate;
            std::vector<ImagerRawFrame>    m_rawFrames;
            uint16_t                       m_imageColumns;     //!< The number of columns the target image should have.
            uint16_t                       m_imageRows;        //!< The number of rows the target image should have.
            uint16_t                       m_roiCMin;          //!< The zero-based index of the first active column.
            uint16_t                       m_roiRMin;          //!< The zero-based index of the first active row.


            /**
            * Frame rate of individual raw frames, or zero for maximum possible. Used by flow control.
            */
            uint16_t                       m_rawFrameRate;

            /**
            * Turn spread spectrum feature (spread spectrum clock) on or off for modulated raw frames.
            */
            bool                           m_enableSSC;

            bool                           m_enableMixedMode;  //!< Enable the mixed mode operation.
        };
    }
}
