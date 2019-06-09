/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <record/FileReaderDispatcher.hpp>

#include <sys/stat.h>

#include <record/v1/FileReader.hpp>
#include <record/v2/FileReader.hpp>
#include <record/v3/FileReader.hpp>

using namespace royale;
using namespace royale::record;

#define READER_OPEN_CHECK \
do {\
if (!m_fileReader) \
{ \
throw std::logic_error("No file opened"); \
} \
} while (0)

namespace
{
    inline bool fileexists (const std::string &filename)
    {
        if (filename.empty())
        {
            return false;
        }

        struct stat_royale_rrf info;
        return stat_royale_rrf (filename.c_str(), &info) == 0;
    }
}

FileReaderDispatcher::FileReaderDispatcher() :
    FileReaderBase(),
    m_fileReader (nullptr),
    m_fileVersion (0)
{
}

FileReaderDispatcher::~FileReaderDispatcher()
{
    close();
}

void FileReaderDispatcher::open (const std::string &filename)
{
    if (!fileexists (filename))
    {
        throw (std::invalid_argument ("Could not find recording"));
    }

    fopen_royale_rrf (m_file, filename.c_str(), "rb");
    if (!m_file)
    {
        throw (std::logic_error ("Could not open recording"));
    }

    fseek64_royale_rrf (m_file, 0, SEEK_SET);



    char magic[ROYALE_FILEHEADER_MAGIC_LENGTH];
    size_t bytesRead = fread (&magic, ROYALE_FILEHEADER_MAGIC_LENGTH, 1, m_file);

    if (bytesRead != 1)
    {
        close();
        throw (std::runtime_error ("File corrupted"));
    }

    if (memcmp (magic, ROYALE_FILEHEADER_MAGIC, ROYALE_FILEHEADER_MAGIC_LENGTH))
    {
        close();
        throw (std::runtime_error ("Magic field corrupted"));
    }

    uint8_t fileVersion;
    bytesRead = fread (&fileVersion, 1, 1, m_file);

    m_fileVersion = fileVersion;

    fclose (m_file);
    m_file = nullptr;

    switch (fileVersion)
    {
        case 1:
            m_fileReader = new v1::FileReader();
            break;
        case 2:
            m_fileReader = new v2::FileReader();
            break;
        case 3:
            m_fileReader = new v3::FileReader();
            break;
        default:
            throw (std::logic_error ("Version not supported"));
    }

    m_fileReader->open (filename);
}

void FileReaderDispatcher::close()
{
    if (m_fileReader)
    {
        m_fileReader->close();
        delete m_fileReader;
        m_fileReader = nullptr;
        m_fileVersion = 0;
    }
}

void FileReaderDispatcher::seek (uint32_t frameNumber)
{
    READER_OPEN_CHECK;
    return m_fileReader->seek (frameNumber);
}

royale_rrf_platformtype FileReaderDispatcher::platform() const
{
    READER_OPEN_CHECK;
    return m_fileReader->platform();
}

std::string FileReaderDispatcher::cameraName() const
{
    READER_OPEN_CHECK;
    return m_fileReader->cameraName();
}

std::string FileReaderDispatcher::imagerType() const
{
    READER_OPEN_CHECK;
    return m_fileReader->imagerType();
}

std::string FileReaderDispatcher::pseudoDataInterpreterType() const
{
    READER_OPEN_CHECK;
    return m_fileReader->pseudoDataInterpreterType();
}

uint32_t FileReaderDispatcher::royaleMajor() const
{
    READER_OPEN_CHECK;
    return m_fileReader->royaleMajor();
}

uint32_t FileReaderDispatcher::royaleMinor() const
{
    READER_OPEN_CHECK;
    return m_fileReader->royaleMinor();
}

uint32_t FileReaderDispatcher::royalePatch() const
{
    READER_OPEN_CHECK;
    return m_fileReader->royalePatch();
}

uint32_t FileReaderDispatcher::royaleBuild() const
{
    READER_OPEN_CHECK;
    return m_fileReader->royaleBuild();
}

std::string FileReaderDispatcher::imagerSerial() const
{
    READER_OPEN_CHECK;
    return m_fileReader->imagerSerial();
}

uint32_t FileReaderDispatcher::numFrames() const
{
    READER_OPEN_CHECK;
    return m_fileReader->numFrames();
}

uint32_t FileReaderDispatcher::currentFrame() const
{
    READER_OPEN_CHECK;
    return m_fileReader->currentFrame();
}

const std::vector<uint8_t> &FileReaderDispatcher::getCalibrationData() const
{
    READER_OPEN_CHECK;
    return m_fileReader->getCalibrationData();
}

const bool FileReaderDispatcher::hasCalibrationData() const
{
    READER_OPEN_CHECK;
    return m_fileReader->hasCalibrationData();
}

void FileReaderDispatcher::get (std::vector <std::vector<uint16_t>> &imageData,
                                std::vector <std::vector<uint16_t>> &pseudoData,
                                royale_frameheader_v3 *frameHeader,
                                std::vector <royale_streamheader_v3> &streamHeaders,
                                std::vector <royale_framegroupheader_v3> &frameGroupHeaders,
                                std::vector <royale_exposuregroupheader_v3> &exposureGroupHeaders,
                                std::vector <royale_rawframesetheader_v3> &rawFrameSetHeaders,
                                std::vector <royale_processingparameter_v3> &processingParameters,
                                std::vector<std::pair<std::string, std::vector<uint8_t>>> &additionalData)
{
    READER_OPEN_CHECK;
    return m_fileReader->get (imageData, pseudoData, frameHeader, streamHeaders,
                              frameGroupHeaders, exposureGroupHeaders, rawFrameSetHeaders, processingParameters, additionalData);
}

std::vector<royale_versioninformation_v3> FileReaderDispatcher::getComponentVersions() const
{
    READER_OPEN_CHECK;
    return m_fileReader->getComponentVersions();
}

uint16_t FileReaderDispatcher::getMaxWidth() const
{
    READER_OPEN_CHECK;
    return m_fileReader->getMaxWidth();
}

uint16_t FileReaderDispatcher::getMaxHeight() const
{
    READER_OPEN_CHECK;
    return m_fileReader->getMaxHeight();
}

bool FileReaderDispatcher::isOpen() const
{
    READER_OPEN_CHECK;
    return m_fileReader->isOpen();
}

uint16_t FileReaderDispatcher::getFileVersion()
{
    return m_fileVersion;
}
