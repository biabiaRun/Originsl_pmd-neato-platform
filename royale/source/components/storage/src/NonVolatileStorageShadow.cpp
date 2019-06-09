/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <storage/NonVolatileStorageShadow.hpp>
#include <common/Crc32.hpp>
#include <common/FileSystem.hpp>
#include <common/exceptions/CalibrationDataNotFound.hpp>
#include <common/exceptions/NotImplemented.hpp>

using namespace royale::storage;
using namespace royale::common;

NonVolatileStorageShadow::NonVolatileStorageShadow (const royale::Vector<uint8_t> &calibrationData,
        const royale::Vector<uint8_t> &identifier,
        const royale::String &suffix,
        const royale::String &serialNumber) :
    m_calibrationData (calibrationData),
    m_identifier (identifier),
    m_suffix (suffix),
    m_serialNumber (serialNumber),
    m_calibrationDataChecksum (calculateCRC32 (calibrationData.data(), calibrationData.size()))
{
}

NonVolatileStorageShadow::NonVolatileStorageShadow (royale::hal::INonVolatileStorage &src,
        bool useCaching) :
    m_identifier (src.getModuleIdentifier()),
    m_suffix (src.getModuleSuffix()),
    m_serialNumber (src.getModuleSerialNumber()),
    m_calibrationDataChecksum (src.getCalibrationDataChecksum())
{
    bool caching = useCaching;

    // Check if there is a serial number on the storage.
    // If not caching will be disabled
    if (m_serialNumber.empty())
    {
        caching = false;
    }

    m_calibrationData.clear();

    if (!caching)
    {
        m_calibrationData = src.getCalibrationData();
    }
    else
    {
        String externalStoragePath = getExternalStoragePath();

        if (!externalStoragePath.empty())
        {
            externalStoragePath = externalStoragePath + "/";
        }

        // Check if we already created a cached version of
        // the calibration
        royale::String filename = externalStoragePath + m_serialNumber + ".cal";

        if (fileexists (filename))
        {
            LOG (DEBUG) << "Using cached calibration : " << filename;
            royale::Vector<uint8_t> calData;
            readFileToVector (filename, calData);

            auto checkSum = calculateCRC32 (calData.data(), calData.size());

            // Check if the file we loaded has the same checksum
            // as the calibration data from the storage
            if (m_calibrationDataChecksum != 0 &&
                    checkSum == m_calibrationDataChecksum)
            {
                m_calibrationData = calData;
            }
        }

        if (m_calibrationData.empty())
        {
            m_calibrationData = src.getCalibrationData();

            // We didn't cache it before, so save a copy now
            writeVectorToFile (filename, m_calibrationData);
        }
    }
}

royale::Vector<uint8_t> NonVolatileStorageShadow::getModuleIdentifier()
{
    return m_identifier;
}

royale::String NonVolatileStorageShadow::getModuleSuffix()
{
    return m_suffix;
}

royale::String NonVolatileStorageShadow::getModuleSerialNumber()
{
    return m_serialNumber;
}

royale::Vector<uint8_t> NonVolatileStorageShadow::getCalibrationData()
{
    return m_calibrationData;
}

uint32_t NonVolatileStorageShadow::getCalibrationDataChecksum()
{
    return m_calibrationDataChecksum;
}

void NonVolatileStorageShadow::writeCalibrationData (const royale::Vector<uint8_t> &data)
{
    throw NotImplemented ("Writing calibration data is not supported for StorageShadow");
}

void NonVolatileStorageShadow::writeCalibrationData (const royale::Vector<uint8_t> &data,
        const royale::Vector<uint8_t> &identifier,
        const royale::String &suffix,
        const royale::String &serialNumber)
{
    throw NotImplemented ("Writing calibration data is not supported for StorageShadow");
}
