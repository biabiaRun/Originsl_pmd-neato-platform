/****************************************************************************\
* Copyright (C) 2015 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <record/v2/FileReader.hpp>

using namespace royale;
using namespace royale::record;

v2::FileReader::FileReader() :
    FileReaderBase(),
    m_currentFrame (0),
    m_fileHeader {}
{
}

v2::FileReader::~FileReader()
{
}

void v2::FileReader::open (const std::string &filename)
{
    close();

    fopen_royale_rrf (m_file, filename.c_str(), "rb");
    if (!m_file)
    {
        throw (std::invalid_argument ("Could not open recording"));
    }

    fseek64_royale_rrf (m_file, 0, SEEK_SET);

    size_t elementsRead = fread (&m_fileHeader, sizeof (royale_fileheader_v2), 1, m_file);

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

    if (m_fileHeader.version != 2)
    {
        close();
        throw (std::logic_error ("Wrong recording version"));
    }

    if (m_fileHeader.numFrames == 0)
    {
        close();
        throw (std::length_error ("No frames in the recording"));
    }

    // Read the version information block
    fseek64_royale_rrf (m_file, sizeof (royale_fileheader_v2), SEEK_SET);
    m_componentVersions.clear();

    static_assert (ROYALE_FILEHEADER_V2_COMPONENT_NAME_LENGTH == ROYALE_FILEHEADER_V3_COMPONENT_NAME_LENGTH, "v2 size != v3 size");
    static_assert (ROYALE_FILEHEADER_V2_COMPONENT_VERSION_LENGTH == ROYALE_FILEHEADER_V3_COMPONENT_VERSION_LENGTH, "v2 size != v3 size");
    m_componentVersions.resize (m_fileHeader.numVersions);
    for (uint32_t i = 0u; i < m_fileHeader.numVersions; ++i)
    {
        elementsRead = fread (&m_componentVersions[i].componentName, ROYALE_FILEHEADER_V3_COMPONENT_NAME_LENGTH, 1, m_file);
        elementsRead += fread (&m_componentVersions[i].componentVersion, ROYALE_FILEHEADER_V3_COMPONENT_VERSION_LENGTH, 1, m_file);

        memset (&m_componentVersions[i].componentType[0], 0, ROYALE_FILEHEADER_V3_COMPONENT_TYPE_LENGTH);

        if (elementsRead != 2)
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


    m_imagerSerial = std::string (m_fileHeader.imagerSerial, ROYALE_FILEHEADER_V2_SERIAL_LENGTH);

    buildOffsetMap();
}

void v2::FileReader::close()
{
    m_currentFrame = 0;
    m_maxSensorWidth = 0;
    m_maxSensorHeight = 0;
    m_offsetMap.clear();
    m_calibrationData.clear();
    m_imagerSerial.clear();

    memset (&m_fileHeader, 0, sizeof (royale_fileheader_v2));

    if (m_file)
    {
        fclose (m_file);
        m_file = nullptr;
    }
}

void v2::FileReader::seek (uint32_t frameNumber)
{
    auto offset = m_offsetMap.find (frameNumber);
    if (offset == m_offsetMap.end())
    {
        throw (std::logic_error ("Frame not found!"));
    }

    m_currentFrame = frameNumber;
}

royale_rrf_platformtype v2::FileReader::platform() const
{
    FILE_OPEN_CHECK
    return static_cast<royale_rrf_platformtype> (m_fileHeader.platform);
}

std::string v2::FileReader::cameraName() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.cameraName;
}

std::string v2::FileReader::imagerType() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.imagerType;
}

std::string v2::FileReader::pseudoDataInterpreterType() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.pseudoDataInterpreter;
}

uint32_t v2::FileReader::royaleMajor() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.royaleMajor;
}

uint32_t v2::FileReader::royaleMinor() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.royaleMinor;
}

uint32_t v2::FileReader::royalePatch() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.royalePatch;
}

uint32_t v2::FileReader::royaleBuild() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.royaleBuild;
}

uint32_t v2::FileReader::numFrames() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.numFrames;
}

uint32_t v2::FileReader::currentFrame() const
{
    FILE_OPEN_CHECK
    return m_currentFrame;
}

void v2::FileReader::buildOffsetMap()
{
    royale_frameheader_v2 frameHeader;
    if (fseek64_royale_rrf (m_file, m_fileHeader.framesOffset, SEEK_SET))
    {
        close();
        throw (std::logic_error ("Frames offset corrupted"));
    }

    for (uint32_t i = 0; i < m_fileHeader.numFrames; ++i)
    {
        m_offsetMap[i] = ftell64_royale_rrf (m_file);
        size_t elementsRead = fread (&frameHeader, sizeof (royale_frameheader_v2), 1, m_file);
        if (elementsRead != 1)
        {
            close();
            throw (std::runtime_error ("Frame corrupted"));
        }

        if (frameHeader.numColumns > m_maxSensorWidth)
        {
            m_maxSensorWidth = frameHeader.numColumns;
        }

        if (frameHeader.numRows > 0 &&
                frameHeader.numRows - 1 > m_maxSensorHeight)
        {
            m_maxSensorHeight = static_cast<uint16_t> (frameHeader.numRows - 1);
        }

        if (frameHeader.frameSize < sizeof (royale_frameheader_v2) ||
                fseek64_royale_rrf (m_file, frameHeader.frameSize - sizeof (royale_frameheader_v2), SEEK_CUR))
        {
            close();
            throw (std::out_of_range ("FrameHeader corrupted"));
        }
    }
}

void v2::FileReader::get (std::vector <std::vector<uint16_t>> &imageData,
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

    royale_frameheader_v2 tempFrameHeader;
    fread_checked (&tempFrameHeader, sizeof (royale_frameheader_v2), 1, m_file);

    memset (frameHeader, 0, sizeof (royale_frameheader_v3));

    frameHeader->frameSize = tempFrameHeader.frameSize;
    frameHeader->illuTemperature = tempFrameHeader.illuTemperature;
    frameHeader->numParameters = tempFrameHeader.numParameters;
    frameHeader->numAdditionalData = tempFrameHeader.numAdditionalData;
    frameHeader->numColumns = tempFrameHeader.numColumns;
    frameHeader->numRawFrames = tempFrameHeader.numRawFrames;
    frameHeader->numRawFrameSets = tempFrameHeader.numRawFrameSets;
    frameHeader->numRows = tempFrameHeader.numRows;
    frameHeader->targetFrameRate = tempFrameHeader.targetFrameRate;
    frameHeader->timestamp = tempFrameHeader.timestamp;
    frameHeader->curStreamId = 0xdefa; // Default stream id
    frameHeader->numStreams = 1;
    frameHeader->numFrameGroups = 1;
    frameHeader->numExposureGroups = frameHeader->numRawFrameSets;

    streamHeaders.resize (1); // We have only one stream in v2 files
    streamHeaders[0].numFrameGroups = 1;
    streamHeaders[0].frameGroupIdxs[0] = 0;
    streamHeaders[0].streamId = 0xdefa;

    frameGroupHeaders.resize (1);
    frameGroupHeaders[0].numRawFrameSets = frameHeader->numRawFrameSets;
    for (uint16_t i = 0u; i < frameHeader->numRawFrameSets; ++i)
    {
        frameGroupHeaders[0].rawFrameSetIdxs[i] = i;
    }

    exposureGroupHeaders.resize (frameHeader->numRawFrameSets);
    for (uint16_t i = 0u; i < frameHeader->numRawFrameSets; ++i)
    {
        std::stringstream outStream;
        outStream << "Group" << i;
        std::string outString = outStream.str();
        outString.resize (ROYALE_FILEHEADER_V3_EXPOSUREGROUP_NAME_LENGTH);

        memcpy (&exposureGroupHeaders[i].exposureGroupName[0], outString.c_str(), ROYALE_FILEHEADER_V3_EXPOSUREGROUP_NAME_LENGTH);
    }

    static_assert (ROYALE_FILEHEADER_V2_USE_CASE_LENGTH == ROYALE_FILEHEADER_V3_USE_CASE_LENGTH, "v2 size != v3 size");
    memcpy (frameHeader->useCaseName, tempFrameHeader.useCaseName, ROYALE_FILEHEADER_V3_USE_CASE_LENGTH);

    // Remove the pseudo data line
    frameHeader->numRows--;

    imageData.resize (frameHeader->numRawFrames);
    pseudoData.resize (frameHeader->numRawFrames);
    for (uint16_t i = 0; i < frameHeader->numRawFrames; ++i)
    {
        pseudoData.at (i).resize (frameHeader->numColumns);
        imageData.at (i).resize (frameHeader->numColumns * frameHeader->numRows);
    }

    rawFrameSetHeaders.resize (frameHeader->numRawFrameSets);

    auto currentRawFrame = 0;
    for (uint16_t i = 0u; i < frameHeader->numRawFrameSets; ++i)
    {
        royale_rawframesetheader_v2 tempRFS;
        fread_checked (&tempRFS, sizeof (royale_rawframesetheader_v2), 1, m_file);

        rawFrameSetHeaders[i].modFreq = tempRFS.modFreq;
        rawFrameSetHeaders[i].capturedExpTime = tempRFS.capturedExpTime;

        royale_phasedefinition_v2 ph = static_cast<royale_phasedefinition_v2> (tempRFS.phaseDefinition);
        switch (ph)
        {
            case GREYSCALE:
                rawFrameSetHeaders[i].phaseDefinition = static_cast<uint8_t> (royale_phasedefinition_v3::GRAYSCALE);
                break;
            case MODULATED:
                rawFrameSetHeaders[i].phaseDefinition = static_cast<uint8_t> (royale_phasedefinition_v3::MODULATED_4PH_CW);
                break;
            default:
                // this shouldn't happen
                throw (std::logic_error ("Unknown phase definition"));
        }

        rawFrameSetHeaders[i].dutyCycle = 0; // DC_AUTO
        rawFrameSetHeaders[i].numRawFrames = tempRFS.numRawFrames;
        rawFrameSetHeaders[i].alignment = 1; // START_ALIGNED
        rawFrameSetHeaders[i].exposureGroupIdx = i;

        exposureGroupHeaders[i].exposureMax = tempFrameHeader.exposureMax;
        exposureGroupHeaders[i].exposureMin = tempFrameHeader.exposureMin;
        exposureGroupHeaders[i].exposureTime = tempRFS.useCaseExpTime;

        for (uint16_t j = 0u; j < rawFrameSetHeaders[i].numRawFrames; ++j)
        {
            royale_rawframeheader_v2 tmpFrameHeader; // Not used afterwards
            fread_checked (&tmpFrameHeader, sizeof (royale_rawframeheader_v2), 1, m_file);

            fread_checked (&pseudoData[currentRawFrame][0], pseudoData[currentRawFrame].size () * sizeof (uint16_t), 1, m_file);
            fread_checked (&imageData[currentRawFrame][0], imageData[currentRawFrame].size () * sizeof (uint16_t), 1, m_file);
            currentRawFrame++;
        }
    }

    if (frameHeader->numParameters > 0u)
    {
        processingParameters.resize (frameHeader->numParameters);
        for (uint32_t i = 0u; i < frameHeader->numParameters; ++i)
        {
            royale_processingparameter_v2 tempParam;
            fread_checked (&tempParam, sizeof (royale_processingparameter_v2), 1, m_file);

            processingParameters[i].dataType = tempParam.dataType;
            processingParameters[i].processingFlag = tempParam.processingFlag;
            processingParameters[i].value = tempParam.value;
        }
    }

    static_assert (ROYALE_FILEHEADER_V2_ADDITIONAL_DATA_NAME_LENGTH == ROYALE_FILEHEADER_V3_ADDITIONAL_DATA_NAME_LENGTH, "v2 size != v3 size");

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

std::vector<royale_versioninformation_v3> v2::FileReader::getComponentVersions() const
{
    return m_componentVersions;
}
