/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <record/FileWriter.hpp>

#include <sstream>
#include <string.h>
#include <exception>

#ifndef ROYALE_ENABLE_PLATFORM_CODE
#include <royale-version.h>
#endif

using namespace royale;
using namespace royale::record;

#define fwrite_checked(ptr, size, count, stream ) \
    {size_t fwrite_ret = fwrite (ptr, size, count, stream); \
    if (fwrite_ret != count) \
            { \
        std::stringstream sstr; \
        sstr << "Write error " << "File : " << __FILE__ << " Line : " << __LINE__; \
        throw (std::runtime_error (sstr.str ())); \
        }}

FileWriter::FileWriter()
{
    m_file = 0;
    close();
}

FileWriter::~FileWriter()
{
    close();
}

void FileWriter::open (const std::string &filename, const std::vector<uint8_t> &calibrationData,
                       const std::string &imagerSerial, const std::string &cameraName, const std::string &imagerType,
                       const std::string &pseudoDataInter, uint32_t royaleMajor, uint32_t royaleMinor,
                       uint32_t royalePatch, uint32_t royaleBuild, royale_rrf_platformtype platform,
                       const std::vector<std::string> &componentNames, const std::vector<std::string> &componentTypes,
                       const std::vector<std::string> &componentVersions)
{
    close();

    fopen_royale_rrf (m_file, filename.c_str(), "wb");

    if (!m_file)
    {
        throw (std::invalid_argument ("Could not open file"));
    }

    // Fill the file header
    m_imagerSerial = imagerSerial;
    m_cameraName = cameraName;
    m_imagerType = imagerType;
    m_pseudoDataInter = pseudoDataInter;

    m_fileHeader.royaleMajor = royaleMajor;
    m_fileHeader.royaleMinor = royaleMinor;
    m_fileHeader.royalePatch = royalePatch;
    m_fileHeader.royaleBuild = royaleBuild;
    m_fileHeader.platform = static_cast<uint8_t> (platform);

    if (componentNames.size() != componentVersions.size())
    {
        throw (std::runtime_error ("Component names and versions do not match"));
    }
    m_fileHeader.numVersions = static_cast<uint32_t> (componentNames.size());

    m_fileHeader.compressionType = static_cast<uint8_t> (royale_rrf_compressiontype::NONE);

    memcpy (m_fileHeader.magic, ROYALE_FILEHEADER_MAGIC, ROYALE_FILEHEADER_MAGIC_LENGTH);
    m_fileHeader.version = ROYALE_FILEHEADER_VERSION;

    m_cameraName.resize (ROYALE_FILEHEADER_V3_CAMERA_NAME_LENGTH);
    memcpy (&m_fileHeader.cameraName, m_cameraName.c_str(), ROYALE_FILEHEADER_V3_CAMERA_NAME_LENGTH);

    m_imagerType.resize (ROYALE_FILEHEADER_V3_IMAGER_TYPE_LENGTH);
    memcpy (&m_fileHeader.imagerType, m_imagerType.c_str(), ROYALE_FILEHEADER_V3_IMAGER_TYPE_LENGTH);

    m_pseudoDataInter.resize (ROYALE_FILEHEADER_V3_PSEUDODATA_INTER_LENGTH);
    memcpy (&m_fileHeader.pseudoDataInterpreter, m_pseudoDataInter.c_str(), ROYALE_FILEHEADER_V3_PSEUDODATA_INTER_LENGTH);

    m_imagerSerial.resize (ROYALE_FILEHEADER_V3_SERIAL_LENGTH);
    memcpy (&m_fileHeader.imagerSerial, m_imagerSerial.c_str(), ROYALE_FILEHEADER_V3_SERIAL_LENGTH);

    fwrite_checked (&m_fileHeader, sizeof (royale_fileheader_v3), 1, m_file);

    // Write the version information block
    for (auto i = 0u; i < m_fileHeader.numVersions; ++i)
    {
        std::string componentName (componentNames[i]);
        std::string componentType (componentTypes[i]);
        std::string componentVersion (componentVersions[i]);

        componentName.resize (ROYALE_FILEHEADER_V3_COMPONENT_NAME_LENGTH);
        componentType.resize (ROYALE_FILEHEADER_V3_COMPONENT_TYPE_LENGTH);
        componentVersion.resize (ROYALE_FILEHEADER_V3_COMPONENT_VERSION_LENGTH);

        fwrite_checked (componentName.c_str(), ROYALE_FILEHEADER_V3_COMPONENT_NAME_LENGTH, 1, m_file);
        fwrite_checked (componentType.c_str(), ROYALE_FILEHEADER_V3_COMPONENT_TYPE_LENGTH, 1, m_file);
        fwrite_checked (componentVersion.c_str(), ROYALE_FILEHEADER_V3_COMPONENT_VERSION_LENGTH, 1, m_file);
    }

    // Write the calibration data block
    m_fileHeader.calibrationOffset = ftell64_royale_rrf (m_file);
    m_fileHeader.calibrationSize = static_cast<uint32_t> (calibrationData.size());

    if (!calibrationData.empty ())
    {
        fwrite_checked (&calibrationData[0], calibrationData.size(), 1, m_file);
    }

    m_fileHeader.framesOffset = ftell64_royale_rrf (m_file);

    if (fseek64_royale_rrf (m_file, 0, SEEK_SET))
    {
        throw (std::runtime_error ("Error writing header"));
    }

    fwrite_checked (&m_fileHeader, sizeof (royale_fileheader_v3), 1, m_file);

    if (fseek64_royale_rrf (m_file, m_fileHeader.framesOffset, SEEK_SET))
    {
        throw (std::runtime_error ("Error jumping to frame start"));
    }
}

void FileWriter::close()
{
    if (m_file)
    {
        m_fileHeader.framesSize = ftell64_royale_rrf (m_file) - m_fileHeader.framesOffset;

        if (fseek64_royale_rrf (m_file, 0, SEEK_SET))
        {
            throw (std::runtime_error ("Error writing header before closing"));
        }

        fwrite_checked (&m_fileHeader, sizeof (royale_fileheader_v3), 1, m_file);

        fclose (m_file);
        m_file = 0;
    }

    memset (&m_fileHeader, 0, sizeof (royale_fileheader_v3));
}

void FileWriter::put (const std::vector <const uint16_t *> &imageData,
                      const std::vector <const uint16_t *> &pseudoData,
                      const royale_frameheader_v3 *frameHeader,
                      const std::vector <royale_streamheader_v3> &streamHeaders,
                      const std::vector <royale_framegroupheader_v3> &frameGroupHeaders,
                      const std::vector <royale_exposuregroupheader_v3> &exposureGroupHeaders,
                      const std::vector <royale_rawframesetheader_v3> &rawFrameSetHeaders,
                      const std::vector <royale_processingparameter_v3> &processingParameters,
                      const std::vector<royale_additionaldata_v3> &additionalData)
{
    if (!m_file)
    {
        // Seems we haven't opened a file yet
        return;
    }

    uint64_t frameStart = ftell64_royale_rrf (m_file);

    // Write frame header
    fwrite_checked (frameHeader, sizeof (royale_frameheader_v3), 1, m_file);

    royale_frameheader_v3 newFrameHeader;
    memcpy (&newFrameHeader, frameHeader, sizeof (royale_frameheader_v3));

    for (auto streamHeader : streamHeaders)
    {
        // Write the header for each stream
        fwrite_checked (&streamHeader, sizeof (royale_streamheader_v3), 1, m_file);
    }

    for (auto frameGroupHeader : frameGroupHeaders)
    {
        // Write the header for each frame group
        fwrite_checked (&frameGroupHeader, sizeof (royale_framegroupheader_v3), 1, m_file);
    }

    for (auto exposureGroupHeader : exposureGroupHeaders)
    {
        // Write the header for each frame group
        fwrite_checked (&exposureGroupHeader, sizeof (royale_exposuregroupheader_v3), 1, m_file);
    }

    for (auto i = 0; i < frameHeader->numRawFrameSets; ++i)
    {
        // Write raw frame set header
        fwrite_checked (&rawFrameSetHeaders[i], sizeof (royale_rawframesetheader_v3), 1, m_file);
    }

    for (uint16_t j = 0; j < frameHeader->numRawFrames; ++j)
    {
        // Write raw data
        fwrite_checked (pseudoData[j], frameHeader->numColumns * sizeof (uint16_t), 1, m_file);
        fwrite_checked (imageData[j], frameHeader->numRows * frameHeader->numColumns * sizeof (uint16_t), 1, m_file);
    }

    // Write parameters
    for (auto i = 0u; i < frameHeader->numParameters; ++i)
    {
        // Write processing parameter header
        fwrite_checked (&processingParameters[i], sizeof (royale_processingparameter_v3), 1, m_file);
    }

    // Write additional data blocks
    for (auto adBlock = additionalData.begin(); adBlock != additionalData.end(); ++adBlock)
    {
        fwrite_checked (adBlock->dataName, ROYALE_FILEHEADER_V3_ADDITIONAL_DATA_NAME_LENGTH, 1, m_file);
        fwrite_checked (&adBlock->dataSize, sizeof (uint64_t), 1, m_file);
        fwrite_checked (adBlock->data, static_cast<size_t> (adBlock->dataSize), 1, m_file);
    }

    // Update frame size in frame header
    uint64_t frameEnd = ftell64_royale_rrf (m_file);
    newFrameHeader.frameSize = static_cast<uint32_t> (frameEnd - frameStart);
    fseek64_royale_rrf (m_file, frameStart, SEEK_SET);
    fwrite_checked (&newFrameHeader, sizeof (royale_frameheader_v3), 1, m_file);
    fseek64_royale_rrf (m_file, frameEnd, SEEK_SET);

    if (fflush (m_file))
    {
        // There was a problem flushing the file stream
        std::stringstream sstr;
        sstr << "Error during flushing of the stream : Error " << ferror (m_file);
        throw std::runtime_error (sstr.str());
    }

    m_fileHeader.numFrames++;
}
