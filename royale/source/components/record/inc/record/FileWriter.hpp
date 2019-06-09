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

#include <stdio.h>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>

#include <record/v3/FileHeader.h>

namespace royale
{
    namespace record
    {
        /**
        * This contains the file writer class.
        */
        class FileWriter
        {
        public:
            /**
            * \brief Constructor.
            *
            * Constructor for the FileWriter class.
            */
            RRFACCESSAPI FileWriter();

            /**
            * \brief Destructor.
            *
            * Destructor for the FileWriter class.
            */
            RRFACCESSAPI virtual ~FileWriter();

            /**
            * \brief Open a file for recording.
            *
            * Opens a file for recording.
            * @param filename Filename which should be used
            * @param calibrationData Current calibration data
            * @param imagerSerial Serial number of the imager used
            * @param cameraName Camera type used (is the mapping is not known to royalecore)
            * @param imagerType Imager type used
            * @param pseudoDataInter Pseudo data interpreter used
            * @param royaleMajor Royale major version used for the recording
            * @param royaleMinor Royale minor version used for the recording
            * @param royalePatch Royale patch version used for the recording
            * @param royaleBuild Royale build version used for the recording
            * @param platform Platform used for the recording
            * @param componentNames Version information (names of the components)
            * @param componentTypes Version information (types of the components)
            * @param componentVersions Version information (versions of the components)
            */
            RRFACCESSAPI void open (const std::string &filename, const std::vector<uint8_t> &calibrationData,
                                    const std::string &imagerSerial, const std::string &cameraName, const std::string &imagerType,
                                    const std::string &pseudoDataInter, uint32_t royaleMajor, uint32_t royaleMinor,
                                    uint32_t royalePatch, uint32_t royaleBuild, royale_rrf_platformtype platform,
                                    const std::vector<std::string> &componentNames, const std::vector<std::string> &componentTypes,
                                    const std::vector<std::string> &componentVersions);

            /**
            * \brief Close a recording.
            *
            * Closes the current recording and writes the final FileHeader.
            */
            RRFACCESSAPI void close();

            /**
            * \brief Write a frame to file.
            *
            * Writes a frame to the currently opened file.
            * @param imageData Vector of pointers to the image data
            * @param pseudoData Vector of pointers to the pseudo data
            * @param frameHeader Header of the current frame
            * @param streamHeaders Stream information
            * @param frameGroupHeaders Frame group information
            * @param exposureGroupHeaders Exposure group information
            * @param rawFrameSetHeaders RawFrameSet specific information
            * @param processingParameters Parameters used for processing
            * @param additionalData Additional data for this frame
            */
            RRFACCESSAPI void put (const std::vector <const uint16_t *> &imageData,
                                   const std::vector <const uint16_t *> &pseudoData,
                                   const royale_frameheader_v3 *frameHeader,
                                   const std::vector <royale_streamheader_v3> &streamHeaders,
                                   const std::vector <royale_framegroupheader_v3> &frameGroupHeaders,
                                   const std::vector <royale_exposuregroupheader_v3> &exposureGroupHeaders,
                                   const std::vector <royale_rawframesetheader_v3> &rawFrameSetHeaders,
                                   const std::vector <royale_processingparameter_v3> &processingParameters,
                                   const std::vector<royale_additionaldata_v3> &additionalData);

        private:

            /**
            * The current file
            */
            FILE *m_file;

            /**
            * The current file header
            */
            royale_fileheader_v3 m_fileHeader;

            /**
            * Imager serial
            */
            std::string m_imagerSerial;

            /**
            * Camera type
            */
            std::string m_cameraName;

            /**
            * Imager type
            */
            std::string m_imagerType;

            /**
            * Pseudo data interpreter type
            */
            std::string m_pseudoDataInter;
        };
    }
}
