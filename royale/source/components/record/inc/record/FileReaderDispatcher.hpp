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

#include <record/FileReaderBase.hpp>

namespace royale
{
    namespace record
    {
        /**
        * This contains the dispatcher which makes sure that (depending on the file version)
        * every call is transferred to the correct reader.
        */
        class FileReaderDispatcher : public FileReaderBase
        {
        public:
            /**
            * Constructor for the dispatcher.
            */
            RRFACCESSAPI FileReaderDispatcher();

            /**
            * Destructor for the dispatcher.
            */
            RRFACCESSAPI virtual ~FileReaderDispatcher();

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
            RRFACCESSAPI std::string imagerSerial() const override;
            RRFACCESSAPI uint32_t numFrames() const override;
            RRFACCESSAPI uint32_t currentFrame() const override;
            RRFACCESSAPI const std::vector<uint8_t> &getCalibrationData() const override;
            RRFACCESSAPI const bool hasCalibrationData() const override;
            RRFACCESSAPI uint16_t getMaxWidth() const override;
            RRFACCESSAPI uint16_t getMaxHeight() const override;
            RRFACCESSAPI bool isOpen() const override;

            /**
            * Retrieves the file version of the currently opened file.
            */
            RRFACCESSAPI uint16_t getFileVersion();

        private:

            // Pointer to the actual file reader
            FileReaderBase *m_fileReader;

            uint16_t m_fileVersion;
        };
    }
}
