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

#include <cstdint>
#include <stdio.h>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <chrono>
#include <exception>
#include <stdexcept>
#include <string.h>

#include <record/CommonHeader.h>

#include <record/v3/FileHeader.h>

#define FILE_OPEN_CHECK \
    if (!m_file) \
                    { \
        throw std::invalid_argument("No file opened"); \
                    }

#define fread_checked(ptr, size, count, stream ) \
                                                {size_t fread_ret = fread (ptr, size, count, stream); \
    if (fread_ret != count) \
                                    { \
        std::stringstream sstr; \
        sstr << "Read error " << "File : " << __FILE__ << " Line : " << __LINE__; \
        throw (std::runtime_error (sstr.str ())); \
                                }}

namespace royale
{
    namespace record
    {
        /**
        * This contains the base for the different file reader classes.
        */
        class FileReaderBase
        {
        public:
            /**
            * Constructor for the file reader.
            */
            RRFACCESSAPI FileReaderBase();

            /**
            * Destructor for the file reader.
            */
            RRFACCESSAPI virtual ~FileReaderBase();

            /**
            * Opens a previously recorded file.
            * @param filename Filename which should be opened
            */
            RRFACCESSAPI virtual void open (const std::string &filename) = 0;

            /**
            * Closes the current file. If no file is opened it only resets the internal variables.
            */
            RRFACCESSAPI virtual void close() = 0;

            /**
            * Sets the current frame to frameNumber. The next call to \sa get will retrieve this frame.
            * @param frameNumber Desired frame number
            */
            RRFACCESSAPI virtual void seek (uint32_t frameNumber) = 0;

            /**
            * Retrieves the current frame from the file.
            * @param imageData Image data of the frames (size = numColumns * sizeof (uint16_t))
            * @param pseudoData Pseudo data of the frames (size = numColumns * numRows * sizeof (uint16_t))
            * @param frameHeader Header of the current frame
            * @param streamHeaders Stream information
            * @param frameGroupHeaders Frame group information
            * @param exposureGroupHeaders Exposure group information
            * @param rawFrameSetHeaders RawFrameSet specific information
            * @param processingParameters Parameters used for processing
            * @param additionalData Additional data for this frame
            */
            RRFACCESSAPI virtual void get (std::vector <std::vector<uint16_t>> &imageData,
                                           std::vector <std::vector<uint16_t>> &pseudoData,
                                           royale_frameheader_v3 *frameHeader,
                                           std::vector <royale_streamheader_v3> &streamHeaders,
                                           std::vector <royale_framegroupheader_v3> &frameGroupHeaders,
                                           std::vector <royale_exposuregroupheader_v3> &exposureGroupHeaders,
                                           std::vector <royale_rawframesetheader_v3> &rawFrameSetHeaders,
                                           std::vector <royale_processingparameter_v3> &processingParameters,
                                           std::vector<std::pair<std::string, std::vector<uint8_t>>> &additionalData) = 0;

            /**
            * Retrieve the versions of all the components used for the recording
            */
            RRFACCESSAPI virtual std::vector<royale_versioninformation_v3> getComponentVersions() const = 0;

            /**
            * Retrieve the platform that was used for the recording. \sa royale_rrf_platformtype
            */
            RRFACCESSAPI virtual royale_rrf_platformtype platform() const = 0;

            /**
            * Retrieves the camera type used for the recording.
            */
            RRFACCESSAPI virtual std::string cameraName() const = 0;

            /**
            * Retrieves the imager type used for the recording
            */
            RRFACCESSAPI virtual std::string imagerType() const = 0;

            /**
            * Retrieves the imager type used for the recording
            */
            RRFACCESSAPI virtual std::string pseudoDataInterpreterType() const = 0;

            /**
            * The major version of Royale used for the recording.
            */
            RRFACCESSAPI virtual uint32_t royaleMajor() const = 0;

            /**
            * The minor version of Royale used for the recording.
            */
            RRFACCESSAPI virtual uint32_t royaleMinor() const = 0;

            /**
            * The patch version of Royale used for the recording.
            */
            RRFACCESSAPI virtual uint32_t royalePatch() const = 0;

            /**
            * The build version of Royale used for the recording.
            */
            RRFACCESSAPI virtual uint32_t royaleBuild() const = 0;

            /**
            * The serial number of the imager used for the recording.
            */
            RRFACCESSAPI virtual std::string imagerSerial() const;

            /**
            * Retrieves the number of frames inside the recording.
            */
            RRFACCESSAPI virtual uint32_t numFrames() const = 0;

            /**
            * Retrieves the current raw frame number.
            */
            RRFACCESSAPI virtual uint32_t currentFrame() const = 0;

            /**
            * Retrieves the calibration data that was used during the recording.
            */
            RRFACCESSAPI virtual const std::vector<uint8_t> &getCalibrationData() const;

            /**
            * Check if there is calibration data
            */
            RRFACCESSAPI virtual const bool hasCalibrationData() const;

            /**
            *  Returns the maximal width in the recording.
            */
            RRFACCESSAPI virtual uint16_t getMaxWidth() const;

            /**
            *  Returns the maximal width in the recording.
            */
            RRFACCESSAPI virtual uint16_t getMaxHeight() const;

            /**
            *  Returns if a file is opened.
            */
            RRFACCESSAPI virtual bool isOpen() const;

        protected:

            /**
            * Calibration data
            */
            std::vector<uint8_t> m_calibrationData;

            /**
            * The current file
            */
            FILE *m_file;

            std::string m_imagerSerial;

            uint16_t m_maxSensorWidth;
            uint16_t m_maxSensorHeight;
        };
    }
}
