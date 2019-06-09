/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <record/v1/FileReader.hpp>

using namespace royale;
using namespace royale::record;


v1::FileReader::FileReader() :
    FileReaderBase(),
    m_currentFrame (0),
    m_fileHeader {}
{
}

v1::FileReader::~FileReader()
{
}

void v1::FileReader::open (const std::string &filename)
{
    close();

    fopen_royale_rrf (m_file, filename.c_str(), "rb");
    if (!m_file)
    {
        throw (std::invalid_argument ("Could not open recording"));
    }

    fseek64_royale_rrf (m_file, 0, SEEK_SET);

    size_t elementsRead = fread (&m_fileHeader, sizeof (royale_fileheader_v1), 1, m_file);

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

    if (m_fileHeader.version != 1)
    {
        close();
        throw (std::logic_error ("Wrong recording version"));
    }

    if (m_fileHeader.numFrames == 0)
    {
        close();
        throw (std::length_error ("No frames in the recording"));
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


    m_imagerSerial = std::string (m_fileHeader.imagerSerial, ROYALE_FILEHEADER_V1_SERIAL_LENGTH);

    buildOffsetMap();
}

void v1::FileReader::close()
{
    m_currentFrame = 0;
    m_maxSensorWidth = 0;
    m_maxSensorHeight = 0;
    m_offsetMap.clear();
    m_calibrationData.clear();
    m_imagerSerial.clear();

    memset (&m_fileHeader, 0, sizeof (royale_fileheader_v1));

    if (m_file)
    {
        fclose (m_file);
        m_file = nullptr;
    }
}

void v1::FileReader::seek (uint32_t frameNumber)
{
    auto offset = m_offsetMap.find (frameNumber);
    if (offset == m_offsetMap.end())
    {
        throw (std::logic_error ("Frame not found!"));
    }

    m_currentFrame = frameNumber;
}

royale_rrf_platformtype v1::FileReader::platform() const
{
    FILE_OPEN_CHECK
    return static_cast<royale_rrf_platformtype> (m_fileHeader.platform);
}

std::string v1::FileReader::cameraName() const
{
    FILE_OPEN_CHECK
    switch (m_fileHeader.cameraType)
    {
        case 1:
            return "PICOS_STANDARD";
        case 2:
            return "PICOFLEXX";
        case 10:
            return "EVALBOARD_LED_STANDARD";
        case 100:
            return "PMD_PLATFORM";
        case 101:
            return "MINICAM";
        case 10000:
            return "CUSTOM";
        case 0x7fffff01:
        default:
            return "UNKNOWN";
    };
}

std::string v1::FileReader::imagerType() const
{
    FILE_OPEN_CHECK
    switch (m_fileHeader.imagerType)
    {
        case 0 :
            return "M2450_A11";
        case 1:
            return "M2450_A12_SR";
        case 2:
            return "M2450_A12_SSC";
        case 3:
            return "M2452_A11_SR";
        case 4:
            return "M2452_A11_SSC";
        case 5:
            return "M2452_B1x_SR";
        case 6:
            return "M2452_B1x_SSC";
        default:
            return "UNKNOWN";
    };
}

std::string v1::FileReader::pseudoDataInterpreterType() const
{
    FILE_OPEN_CHECK
    switch (m_fileHeader.imagerType)
    {
        case 0 : // ImagerType::M2450_A11:
            return "M2450_A11";
            break;
        case 1 : // ImagerType::M2450_A12_SR:
        case 2 : // ImagerType::M2450_A12_SSC:
            return "M2450_A12";
            break;
        case 3 : // ImagerType::M2452_A11_SR:
        case 4 : // ImagerType::M2452_A11_SSC:
        case 5 : // ImagerType::M2452_B1x_SR:
        case 6 : // ImagerType::M2452_B1x_SSC:
            return "M2452";
            break;
        default:
            // should not be here
            return "";
    }
}

uint32_t v1::FileReader::royaleMajor() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.royaleMajor;
}

uint32_t v1::FileReader::royaleMinor() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.royaleMinor;
}

uint32_t v1::FileReader::royalePatch() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.royalePatch;
}

uint32_t v1::FileReader::royaleBuild() const
{
    FILE_OPEN_CHECK
    // Not supported by v1
    return 0u;
}

uint32_t v1::FileReader::numFrames() const
{
    FILE_OPEN_CHECK
    return m_fileHeader.numFrames;
}

uint32_t v1::FileReader::currentFrame() const
{
    FILE_OPEN_CHECK
    return m_currentFrame;
}

void v1::FileReader::buildOffsetMap()
{
    royale_frameheader_v1 frameHeader;
    if (fseek64_royale_rrf (m_file, m_fileHeader.framesOffset, SEEK_SET))
    {
        close();
        throw (std::logic_error ("Frames offset corrupted"));
    }

    for (uint32_t i = 0; i < m_fileHeader.numFrames; ++i)
    {
        m_offsetMap[i] = ftell64_royale_rrf (m_file);
        size_t elementsRead = fread (&frameHeader, sizeof (royale_frameheader_v1), 1, m_file);
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

        if (frameHeader.frameSize < sizeof (royale_frameheader_v1) ||
                fseek64_royale_rrf (m_file, frameHeader.frameSize - sizeof (royale_frameheader_v1), SEEK_CUR))
        {
            close();
            throw (std::out_of_range ("FrameHeader corrupted"));
        }
    }
}

void v1::FileReader::get (std::vector <std::vector<uint16_t>> &imageData,
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

    royale_frameheader_v1 tempFrameHeader;
    fread_checked (&tempFrameHeader, sizeof (royale_frameheader_v1), 1, m_file);

    memset (frameHeader, 0, sizeof (royale_frameheader_v3));

    frameHeader->frameSize = tempFrameHeader.frameSize;
    frameHeader->illuTemperature = tempFrameHeader.illuTemperature;
    frameHeader->numParameters = tempFrameHeader.numParameters;
    frameHeader->numAdditionalData = 0; // Not used for v1
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

    streamHeaders.resize (1); // We have only one stream in v1 files
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

    static_assert (ROYALE_FILEHEADER_V1_USE_CASE_LENGTH == ROYALE_FILEHEADER_V3_USE_CASE_LENGTH, "v1 size != v3 size");
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

    if (frameHeader->numParameters > 0u)
    {
        processingParameters.resize (frameHeader->numParameters);
        for (uint32_t i = 0u; i < frameHeader->numParameters; ++i)
        {
            royale_processingparameter_v1 tempParam;
            fread_checked (&tempParam, sizeof (royale_processingparameter_v1), 1, m_file);

            processingParameters[i].dataType = tempParam.dataType;
            processingParameters[i].processingFlag = tempParam.processingFlag;
            processingParameters[i].value = tempParam.value;
        }
    }

    rawFrameSetHeaders.resize (frameHeader->numRawFrameSets);

    auto currentRawFrame = 0;
    for (uint16_t i = 0u; i < frameHeader->numRawFrameSets; ++i)
    {
        royale_rawframesetheader_v1 tempRFS;
        fread_checked (&tempRFS, sizeof (royale_rawframesetheader_v1), 1, m_file);

        rawFrameSetHeaders[i].modFreq = tempRFS.modFreq;
        rawFrameSetHeaders[i].capturedExpTime = tempRFS.capturedExpTime;

        royale_phasedefinition_v1 ph = static_cast<royale_phasedefinition_v1> (tempRFS.phaseDefinition);
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
            royale_rawframeheader_v1 tmpFrameHeader; // Not used afterwards
            fread_checked (&tmpFrameHeader, sizeof (royale_rawframeheader_v1), 1, m_file);

            fread_checked (&pseudoData[currentRawFrame][0], pseudoData[currentRawFrame].size() * sizeof (uint16_t), 1, m_file);
            fread_checked (&imageData[currentRawFrame][0], imageData[currentRawFrame].size() * sizeof (uint16_t), 1, m_file);
            currentRawFrame++;
        }
    }

    // V1 doesn't support this
    additionalData.clear();
}

std::vector<royale_versioninformation_v3> v1::FileReader::getComponentVersions() const
{
    return std::vector<royale_versioninformation_v3>();
}
