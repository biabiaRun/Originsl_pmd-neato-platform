/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <storage/NonVolatileStoragePersistent.hpp>
#include <common/FileSystem.hpp>
#include <common/exceptions/CalibrationDataNotFound.hpp>
#include <common/exceptions/NotImplemented.hpp>

using namespace royale::storage;
using namespace royale::common;

NonVolatileStoragePersistent::NonVolatileStoragePersistent (const royale::String &fileName) :
    m_fileName (fileName)
{
}

royale::Vector<uint8_t> NonVolatileStoragePersistent::getCalibrationData()
{
    if (! fileexists (m_fileName))
    {
        throw CalibrationDataNotFound ("File for the calibration data does not exist");
    }

    auto fileSize = getFileSize (m_fileName);
    royale::Vector<uint8_t> data;
    if (readFileToVector (m_fileName, data) != fileSize)
    {
        throw CalibrationDataNotFound ("Failed to read complete calibration data");
    }
    return data;
}

royale::Vector<uint8_t> NonVolatileStoragePersistent::getModuleIdentifier()
{
    if (! fileexists (m_fileName))
    {
        throw CalibrationDataNotFound ("File for the calibration data does not exist");
    }

    return {};
}

royale::String NonVolatileStoragePersistent::getModuleSuffix()
{
    if (! fileexists (m_fileName))
    {
        throw CalibrationDataNotFound ("File for the calibration data does not exist");
    }

    return "";
}

royale::String NonVolatileStoragePersistent::getModuleSerialNumber()
{
    if (! fileexists (m_fileName))
    {
        throw CalibrationDataNotFound ("File for the calibration data does not exist");
    }

    return "";
}

uint32_t NonVolatileStoragePersistent::getCalibrationDataChecksum()
{
    if (!fileexists (m_fileName))
    {
        throw CalibrationDataNotFound ("File for the calibration data does not exist");
    }

    return 0;
}

void NonVolatileStoragePersistent::writeCalibrationData (const royale::Vector<uint8_t> &data)
{
    throw NotImplemented();
}

void NonVolatileStoragePersistent::writeCalibrationData (const royale::Vector<uint8_t> &data,
        const royale::Vector<uint8_t> &identifier,
        const royale::String &suffix,
        const royale::String &serialNumber)
{
    throw NotImplemented();
}
