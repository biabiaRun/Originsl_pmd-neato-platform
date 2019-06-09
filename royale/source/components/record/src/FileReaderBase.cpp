/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <record/FileReaderBase.hpp>

using namespace royale;
using namespace royale::record;

FileReaderBase::FileReaderBase() :
    m_file (nullptr),
    m_maxSensorWidth (0),
    m_maxSensorHeight (0)
{
}

FileReaderBase::~FileReaderBase()
{
    m_file = nullptr;
}

std::string FileReaderBase::imagerSerial() const
{
    FILE_OPEN_CHECK
    return m_imagerSerial;
}

const std::vector<uint8_t> &FileReaderBase::getCalibrationData() const
{
    FILE_OPEN_CHECK
    return m_calibrationData;
}

const bool FileReaderBase::hasCalibrationData() const
{
    FILE_OPEN_CHECK
    return !m_calibrationData.empty();
}

uint16_t FileReaderBase::getMaxWidth() const
{
    FILE_OPEN_CHECK
    return m_maxSensorWidth;
}

uint16_t FileReaderBase::getMaxHeight() const
{
    FILE_OPEN_CHECK
    return m_maxSensorHeight;
}

bool FileReaderBase::isOpen() const
{
    if (m_file)
    {
        return true;
    }
    else
    {
        return false;
    }
}
