/****************************************************************************\
* Copyright (C) 2015 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <ICapturedRawFrame.hpp>

#include <cstdint>
#include <cstddef>
#include <vector>

namespace royale
{
    namespace record
    {
        /**
         * An implementation of CapturedRawFrame that uses a single byte array as the data buffer. This
         * is intended to be used by the CameraPlayback class for the playback of recordings.
         */
        class RecordedRawFrame : public royale::common::ICapturedRawFrame
        {
        public:
            /**
             * Constructor.  Data includes both the image data and the pseudo data.
             *
             * \param data vector containing the data
             * \param columns number of columns (used to calculate the offset to the image data)
             */
            RecordedRawFrame (std::vector<uint16_t> &data, const uint16_t &columns);

            ~RecordedRawFrame();

            /**
            * Retrieve a pointer to the image data.
            */
            uint16_t *getImageData() override;

            /**
            * Retrieve a pointer to the pseudo data.
            */
            const uint16_t *getPseudoData() const override;

        private:

            uint16_t *m_imageData;
            uint16_t *m_pseudoData;
        };
    }
}
