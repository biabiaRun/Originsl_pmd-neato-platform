/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <record/FileReaderBase.hpp>
#include <record/v3/FileHeader.h>

namespace royale
{
    namespace record
    {
        namespace v3
        {
            /**
             * This contains the file reader class for version 3.
             */
            class FileReader : public FileReaderBase
            {
            public:

                RRFACCESSAPI FileReader();
                RRFACCESSAPI virtual ~FileReader();

                RRFACCESSAPI void open (const std::string &filename) override;
                RRFACCESSAPI void close() override;
                RRFACCESSAPI void seek (uint32_t frameNumber) override;
                RRFACCESSAPI void get (std::vector <std::vector<uint16_t>> &imageData,
                                       std::vector <std::vector<uint16_t>> &pseudoData,
                                       royale_frameheader_v3 *frameHeader,
                                       std::vector <royale_streamheader_v3> &streamHeaders,
                                       std::vector <royale_framegroupheader_v3> &frameGroupHeaders,
                                       std::vector <royale_exposuregroupheader_v3> &exposureGroupHeaders,
                                       std::vector <royale_rawframesetheader_v3> &rawFrameSetHeaders,
                                       std::vector <royale_processingparameter_v3> &processingParameters,
                                       std::vector<std::pair<std::string, std::vector<uint8_t>>> &additionalData) override;
                RRFACCESSAPI std::vector<royale_versioninformation_v3> getComponentVersions() const override;
                RRFACCESSAPI royale_rrf_platformtype platform() const override;
                RRFACCESSAPI std::string cameraName() const override;
                RRFACCESSAPI std::string imagerType() const override;
                RRFACCESSAPI std::string pseudoDataInterpreterType() const override;
                RRFACCESSAPI uint32_t royaleMajor() const override;
                RRFACCESSAPI uint32_t royaleMinor() const override;
                RRFACCESSAPI uint32_t royalePatch() const override;
                RRFACCESSAPI uint32_t royaleBuild() const override;
                RRFACCESSAPI uint32_t numFrames() const override;
                RRFACCESSAPI uint32_t currentFrame() const override;

            private:

                /**
                 * Builds the offset map that is used internally to skip between frames.
                 * If there is no number of frames in the file header this function will
                 * also try to count the number of frames inside the file.
                 */
                void buildOffsetMap();

                /**
                 * The current frame
                 */
                uint32_t m_currentFrame;

                /**
                 * Offset map typedef
                 */
                typedef std::map<uint32_t, uint64_t> OffsetMap;
                /**
                 * The offset map used to determine the offset of every frame inside the recording
                 */
                OffsetMap m_offsetMap;

                /**
                 * Header of the current file
                 */
                royale_fileheader_v3 m_fileHeader;

                std::vector<royale_versioninformation_v3> m_componentVersions;
            };
        }
    }
}
