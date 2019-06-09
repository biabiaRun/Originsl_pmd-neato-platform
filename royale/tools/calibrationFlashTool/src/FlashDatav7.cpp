/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <FlashDatav7.hpp>

#include <common/NarrowCast.hpp>
#include <storage/StorageFormatPolar.hpp>

#include <cstdint>
#include <iomanip>
#include <sstream>

using namespace royale;
using namespace royale::common;
using namespace royale::storage;

namespace
{
    class DummyBridge :
        public royale::pal::IStorageReadRandom,
        public royale::pal::IStorageWriteFullOverwrite
    {
    public:

        ~DummyBridge() override = default;

        void readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer) override
        {
            // Do nothing here
        }

        void writeStorage (const std::vector<uint8_t> &buffer) override
        {
            m_flashData = buffer;
        }

        std::vector<uint8_t> m_flashData;
    };
}


FlashDataV7::FlashDataV7 (std::unique_ptr<royale::ICameraDevice> cameraDevice, QWidget *parent) :
    FlashDataBase (std::move (cameraDevice), parent)
{
    setupUi (parent);
}

FlashDataV7::~FlashDataV7()
{
}

void FlashDataV7::writeFlashData()
{
    if (cbAdditionalInfo->isChecked())
    {
        writeHeaderAndData();
    }
    else
    {
        m_cameraDevice->writeDataToFlash (m_calibrationData);
    }
}

void FlashDataV7::readFlashData()
{
    leModuleIdentifier->clear();
    leModuleSerial->clear();
    leModuleSuffix->clear();

    Vector<Pair<String, String>> camInfos;
    m_cameraDevice->getCameraInfo (camInfos);

    for (auto curInfo : camInfos)
    {
        if (curInfo.first == "MODULE_IDENTIFIER")
        {
            if (!curInfo.second.empty())
            {
                leModuleIdentifier->setText (curInfo.second.c_str());
            }
        }
        else if (curInfo.first == "MODULE_SERIAL")
        {
            leModuleSerial->setText (curInfo.second.c_str());
        }
        else if (curInfo.first == "MODULE_SUFFIX")
        {
            leModuleSuffix->setText (curInfo.second.c_str());
        }
    }

    m_cameraDevice->getCalibrationData (m_calibrationData);
}

void FlashDataV7::writeHeaderAndData()
{
    auto dummyBridge = std::make_shared<DummyBridge> (DummyBridge());
    StorageFormatPolar flashMemory (dummyBridge);

    std::stringstream ss;
    ss << std::setw (32) << std::setfill ('0') << leModuleIdentifier->text().toStdString();

    auto strIdent = ss.str();
    strIdent.resize (32, 0);
    Vector<uint8_t> ident;
    ident.resize (16);
    for (auto i = 0u; i < 16u && i < strIdent.size(); ++i)
    {
        auto tmpStr = strIdent.substr (i * 2, 2);
        ident[i] = narrow_cast<uint8_t> (strtol (tmpStr.c_str(), NULL, 16));
    }

    String suffix (leModuleSuffix->text().toStdString());
    String serial (leModuleSerial->text().toStdString());

    flashMemory.writeCalibrationData (m_calibrationData, ident, suffix, serial);

    m_cameraDevice->writeDataToFlash (Vector<uint8_t>::fromStdVector (dummyBridge->m_flashData));
}
