/****************************************************************************\
* Copyright (C) 2016 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <record/v3/FileReader.hpp>

using namespace royale;
using namespace royale::record;

v3::FileReader::FileReader() :
    FileReaderBase(),
    m_currentFrame (0),
    m_fileHeader {}
{
}

v3::FileReader::~FileReader()
{
}

void v3::FileReader::open (const std::string &filename)
{
    close();

    fopen_royale_rrf (m_file, filename.c_str(), "rb");
    if (!m_file)
    {
        throw (std::invalid_argument ("Could not open recording"));
    }

    fseek64_royale_rrf (m_file, 0, SEEK_SET);

    size_t elementsRead = fread (&m_fileHeader, sizeof (royale_fileheader_v3), 1, m_file);

    if (elementsRead != 1)
    {
        close();
        throw (std::invalid_argument ("File corrupted"));
    }

    if (memcmp (m_fileHeader.magic, ROYALE_FILEHEADER_MAGIC, ROYALE_FILEHEADER_MAGIC_LENGTH))
    {
        close();
        throw (std::runtime_error ("Magic field corrupted"));
    }

    if (m_fileHeader.version != 3)
    {
        close();
        throw (std::logic_error ("Wrong recording version"));
    }

    // Read the version information block
    fseek64_royale_rrf (m_file, sizeof (royale_fileheader_v3), SEEK_SET);
    m_componentVersions.clear();

    m_componentVersions.resize (m_fileHeader.numVersions);
    for (uint32_t i = 0u; i < m_fileHeader.numVersions; ++i)
    {
        elementsRead = fread (&m_componentVersions[i].componentName, ROYALE_FILEHEADER_V3_COMPONENT_NAME_LENGTH, 1, m_file);
        elementsRead += fread (&m_componentVersions[i].componentType, ROYALE_FILEHEADER_V3_COMPONENT_TYPE_LENGTH, 1, m_file);
        elementsRead += fread (&m_componentVersions[i].componentVersion, ROYALE_FILEHEADER_V3_COMPONENT_VERSION_LENGTH, 1, m_file);

        if (elementsRead != 3)
        {
            close();
            throw (std::runtime_error ("Component version data corrupted"));
        }
    }

    if (m_fileHeader.calibrationSize)
    {
        m_calibrationData.resize (m_fileHeader.calibrationSize);
        if (fseek64_royale_rrf (m_file, m_fileHeader.calibrationOffset, SEEK_SET))
        {
            close();
            throw (std::out_of_range ("Calibration offset corrupted"));
        }

        elementsRead = fread (&m_calibrationData[0], m_fileHeader.calibrationSize, 1, m_file);

        if (elementsRead != 1)
        {
            close();
            throw (std::out_of_range ("Calibration data corrupted"));
        }
    }


    m_imagerSerial = std::string (m_fileHeader.imagerSerial, ROYALE_FILEHEADER_V3_SERIAL_LENGTH);

    buildOffsetMap();

    if (m_fileHeader.numFrames == 0)
    {
        close();
        throw (std::length_error ("No frames in the recording"));
    }
}

void v3::FileReader::close()
{
    m_currentFrame = 0;
    m_maxSensorWidth = 0;
    m_maxSensorHeight = 0;
    m_offsetMap.clear();
    m_calibrationData.clear();
    m_imagerSerial.clear();

    memset (&m_fileHeader, 0, sizeof (royale_fileheader_v3));

    if (m_file)
    {
        fclose (m_file);
        m_file = nullptr;
    }
}

void v3::FileReader::seek (uint32_t frameNumber)
{
    auto offset = m_offsetMap.find (frameNumber);
    if (offset == m_offsetMap.end())
    {
        throw (std::logic_error ("Frame not found!"));
    }

    m_currentFrame = frameNumber;
}

royale_rrf_platformtype v3::FileReader::platform() const
{
    FILE_OPEN_CHECK
    return static_cast<royale_rrf_platformtype> (m_fileHeader.platform);
}

std::string v3::FileReader::cameraName() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.cameraName;
}

std::string v3::FileReader::imagerType() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.imagerType;
}

std::string v3::FileReader::pseudoDataInterpreterType() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.pseudoDataInterpreter;
}

uint32_t v3::FileReader::royaleMajor() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.royaleMajor;
}

uint32_t v3::FileReader::royaleMinor() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.royaleMinor;
}

uint32_t v3::FileReader::royalePatch() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.royalePatch;
}

uint32_t v3::FileReader::royaleBuild() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.royaleBuild;
}

uint32_t v3::FileReader::numFrames() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.numFrames;
}

uint32_t v3::FileReader::currentFrame() const
{
    FILE_OPEN_CHECK
    return m_currentFrame;
}

void v3::FileReader::buildOffsetMap()
{
    royale_frameheader_v3 frameHeader;
    if (fseek64_royale_rrf (m_file, m_fileHeader.framesOffset, SEEK_SET))
    {
        close();
        throw (std::logic_error ("Frames offset corrupted"));
    }

    bool errorDuringRead = false;
    uint32_t currentFrameNumber = 0u;
    while (!errorDuringRead)
    {
        // Save the current offset and check if we have a valid frame
        auto curOffset = ftell64_royale_rrf (m_file);
        size_t elementsRead = fread (&frameHeader, sizeof (royale_frameheader_v3), 1, m_file);
        if (elementsRead != 1)
        {
            errorDuringRead = true;
            continue;
        }
        // Only save the offset if we successfully read a frame
        m_offsetMap[currentFrameNumber] = curOffset;

        if (frameHeader.numColumns > m_maxSensorWidth)
        {
            m_maxSensorWidth = frameHeader.numColumns;
        }

        if (frameHeader.numRows > 0 &&
                frameHeader.numRows > m_maxSensorHeight)
        {
            m_maxSensorHeight = static_cast<uint16_t> (frameHeader.numRows);
        }

        if (frameHeader.frameSize < sizeof (royale_frameheader_v3) ||
                fseek64_royale_rrf (m_file, frameHeader.frameSize - sizeof (royale_frameheader_v3), SEEK_CUR))
        {
            errorDuringRead = true;
            continue;
        }

        ++currentFrameNumber;
    }

    if (!m_fileHeader.numFrames)
    {
        m_fileHeader.numFrames = currentFrameNumber;
    }
    else
    {
        // Sanity check
        if (m_fileHeader.numFrames != currentFrameNumber)
        {
            throw (std::logic_error ("Number of frames in the file header seems to be wrong"));
        }
    }
}

void v3::FileReader::get (std::vector <std::vector<uint16_t>> &imageData,
                          std::vector <std::vector<uint16_t>> &pseudoData,
                          royale_frameheader_v3 *frameHeader,
                          std::vector <royale_streamheader_v3> &streamHeaders,
                          std::vector <royale_framegroupheader_v3> &frameGroupHeaders,
                          std::vector <royale_exposuregroupheader_v3> &exposureGroupHeaders,
                          std::vector <royale_rawframesetheader_v3> &rawFrameSetHeaders,
                          std::vector <royale_processingparameter_v3> &processingParameters,
                          std::vector<std::pair<std::string, std::vector<uint8_t>>> &additionalData)
{
    auto offset = m_offsetMap[m_currentFrame];
    if (fseek64_royale_rrf (m_file, offset, SEEK_SET))
    {
        throw (std::invalid_argument ("Get: OffsetMap corrupted"));
    }

    fread_checked (frameHeader, sizeof (royale_frameheader_v3), 1, m_file);

    imageData.resize (frameHeader->numRawFrames);
    pseudoData.resize (frameHeader->numRawFrames);
    for (uint16_t i = 0; i < frameHeader->numRawFrames; ++i)
    {
        pseudoData.at (i).resize (frameHeader->numColumns);
        imageData.at (i).resize (frameHeader->numColumns * frameHeader->numRows);
    }

    streamHeaders.resize (frameHeader->numStreams);
    for (uint16_t i = 0u; i < frameHeader->numStreams; ++i)
    {
        // Read the header for each stream
        fread_checked (&streamHeaders[i], sizeof (royale_streamheader_v3), 1, m_file);
    }

    frameGroupHeaders.resize (frameHeader->numFrameGroups);
    for (uint16_t i = 0u; i < frameHeader->numFrameGroups; ++i)
    {
        // Read the header for each frame group
        fread_checked (&frameGroupHeaders[i], sizeof (royale_framegroupheader_v3), 1, m_file);
    }

    exposureGroupHeaders.resize (frameHeader->numExposureGroups);
    for (uint16_t i = 0u; i < frameHeader->numExposureGroups; ++i)
    {
        // Read the header for each exposure group
        fread_checked (&exposureGroupHeaders[i], sizeof (royale_exposuregroupheader_v3), 1, m_file);
    }

    rawFrameSetHeaders.resize (frameHeader->numRawFrameSets);

    for (uint16_t i = 0u; i < frameHeader->numRawFrameSets; ++i)
    {
        fread_checked (&rawFrameSetHeaders[i], sizeof (royale_rawframesetheader_v3), 1, m_file);
    }

    for (uint16_t j = 0u; j < frameHeader->numRawFrames; ++j)
    {
        fread_checked (&pseudoData[j][0], pseudoData[j].size() * sizeof (uint16_t), 1, m_file);
        fread_checked (&imageData[j][0], imageData[j].size() * sizeof (uint16_t), 1, m_file);
    }

    if (frameHeader->numParameters > 0u)
    {
        processingParameters.resize (frameHeader->numParameters);
        for (uint32_t i = 0u; i < frameHeader->numParameters; ++i)
        {
            fread_checked (&processingParameters[i], sizeof (royale_processingparameter_v3), 1, m_file);
        }
    }

    if (frameHeader->numAdditionalData > 0u)
    {
        additionalData.resize (frameHeader->numAdditionalData);
        for (uint32_t i = 0u; i < frameHeader->numAdditionalData; ++i)
        {
            auto &curData = additionalData[i];

            curData.first.resize (ROYALE_FILEHEADER_V3_ADDITIONAL_DATA_NAME_LENGTH);
            fread_checked (&curData.first[0], ROYALE_FILEHEADER_V3_ADDITIONAL_DATA_NAME_LENGTH, 1, m_file);
            uint64_t dataSize;
            fread_checked (&dataSize, sizeof (uint64_t), 1, m_file);
            curData.second.resize (static_cast<size_t> (dataSize));
            fread_checked (&curData.second[0], static_cast<size_t> (dataSize), 1, m_file);
        }
    }
    else
    {
        additionalData.clear();
    }
}

std::vector<royale_versioninformation_v3> v3::FileReader::getComponentVersions() const
{
    return m_componentVersions;
}
