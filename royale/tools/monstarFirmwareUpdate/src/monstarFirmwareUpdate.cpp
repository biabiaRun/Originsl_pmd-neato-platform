/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <monstarFirmwareUpdate.hpp>

#include <common/Crc32.hpp>
#include <common/StringFunctions.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <modules/ModuleConfigPicoMonstarCommon.hpp>
#include <pal/IStorageWriteFullOverwrite.hpp>
#include <storage/StorageFormatPolar.hpp>

#include <G8PIDs.hpp>

using namespace royale::common;
using namespace royale::config;
using namespace royale::factory;
using namespace royale::pal;
using namespace royale::storage;

MonstarFirmwareUpdate::MonstarFirmwareUpdate()
{
    setupUi (this);
    m_statusBox.hide();
}

MonstarFirmwareUpdate::~MonstarFirmwareUpdate()
{
}

void MonstarFirmwareUpdate::on_pbUpdateFirmware_clicked()
{
    // Check for connected devices

    royale::factory::BridgeController bridgeController (getUsbProbeDataPlusG8 (false, true),
            getProcessingParameterMapFactoryRoyale());

    try
    {
        auto devices = bridgeController.probeDevices();

        if (devices.empty())
        {
            QMessageBox::warning (this, this->windowTitle(), "No suitable devices found!");
            return;
        }

        if (devices.size() > 1)
        {
            QMessageBox::warning (this, this->windowTitle(), "Please make sure that only one module is connected!");
            return;
        }

        auto &builder = *devices.front();
        auto coreConfig = builder.getICoreConfig();
        m_cameraName = coreConfig->getCameraName();

        auto &bridgeFactory = builder.getBridgeFactory();

        auto storage = createStorage (bridgeFactory);

        if (storage == nullptr)
        {
            return;
        }

        // Check that we were able to load the right firmware image from the resources
        if (m_firmwareImage.empty())
        {
            QMessageBox::warning (this, this->windowTitle(), "No firmware image!");
            return;
        }

        showStatusBox();

        // Read the current firmware from the flash.
        // As we don't know the size of the firmware we will just read
        // the same amount of data that the new image uses and compare them.
        std::vector<uint8_t> oldImage (m_firmwareImage.size());
        try
        {
            storage->readStorage (0u, oldImage);
        }
        catch (...)
        {
            hideStatusBox();
            QMessageBox::information (this, this->windowTitle(), "There was a problem reading out the flash!");
            return;
        }

        auto oldCrc = calculateCRC32 (&oldImage[0], oldImage.size());
        auto newCrc = calculateCRC32 (&m_firmwareImage[0], m_firmwareImage.size());

        // Check if the new firmware is different from the old one
        if (oldCrc == newCrc)
        {
            hideStatusBox();
            QMessageBox::information (this, this->windowTitle(), "The firmware is already up-to-date.");
            return;
        }

        auto storageWrite = std::dynamic_pointer_cast<IStorageWriteFullOverwrite> (storage);
        if (storageWrite == nullptr)
        {
            hideStatusBox();
            QMessageBox::warning (this, this->windowTitle(), "Cannot create storage for writing!");
            return;
        }

        // Write the new firmware
        try
        {
            storageWrite->writeStorage (m_firmwareImage);
        }
        catch (...)
        {
            hideStatusBox();
            QMessageBox::information (this, this->windowTitle(), "There was a problem writing the new firmware image!");
            return;
        }

        try
        {
            fixFlashHeader (bridgeFactory);
        }
        catch (...)
        {
            hideStatusBox();
            QMessageBox::information (this, this->windowTitle(), "There was a problem writing the new header!");
            return;
        }

    }
    catch (...)
    {
        hideStatusBox();
        QMessageBox::information (this, this->windowTitle(), "There was a problem opening the device!");
        return;
    }

    hideStatusBox();

    QMessageBox::information (this, this->windowTitle(), "The new firmware was successfully written to the flash.\n"
                              "Please reconnect the camera to activate it.");
}

std::vector<uint8_t> MonstarFirmwareUpdate::readFromResource (QString filename)
{
    QFile resFile (":/" + filename);

    if (!resFile.open (QFile::ReadOnly))
    {
        return{};
    }

    QByteArray fileContent = resFile.readAll();

    return std::vector<uint8_t> (fileContent.begin(), fileContent.end());
}

std::shared_ptr<royale::pal::IStorageReadRandom> MonstarFirmwareUpdate::createStorage (royale::factory::IBridgeFactory &bridgeFactory)
{
    std::shared_ptr<royale::pal::IStorageReadRandom> storage;

    try
    {
        std::shared_ptr<ISensorRoutingConfig> flashRoute;
        struct royale::config::FlashMemoryConfig flashConfig;

        // Check if one of the connected devices is a pico monstar
        // and create the appropriate storage and firmware file
        if (stringStartsWith (m_cameraName, { "PICOMONSTAR" }))
        {
            flashRoute = findSensorRoute (royale::picomonstar::sensorMap, SensorRole::STORAGE_CALIBRATION);
            flashConfig = royale::picomonstar::flashConfig;

            m_firmwareImage = readFromResource ("monstar_firmware");
        }
        else
        {
            QMessageBox::warning (this, this->windowTitle(), "This update tool only works for pico monstar modules!");
            return nullptr;
        }

        // Reset the access offset so that we are able to read out the firmware image
        flashConfig.imageSize = flashConfig.accessOffset;
        flashConfig.accessOffset = 0u;
        storage = NonVolatileStorageFactory::createStorageReadRandom (bridgeFactory,
                  flashConfig,
                  flashRoute.get());
    }
    catch (...)
    {
        QMessageBox::warning (this, this->windowTitle(), "Cannot create storage!");
        return nullptr;
    }

    return storage;
}

void MonstarFirmwareUpdate::showStatusBox()
{
    QApplication::setOverrideCursor (Qt::WaitCursor);

    m_statusBox.setStandardButtons (0);
    m_statusBox.setText ("Accessing flash ...\nPlease do not disconnect the camera!");
    m_statusBox.setModal (true);
    m_statusBox.show();
    QCoreApplication::processEvents();
}

void MonstarFirmwareUpdate::hideStatusBox()
{
    m_statusBox.hide();
    QApplication::restoreOverrideCursor();
}

void MonstarFirmwareUpdate::fixFlashHeader (royale::factory::IBridgeFactory &bridgeFactory)
{
    std::shared_ptr<royale::pal::IStorageReadRandom> storage;

    std::shared_ptr<ISensorRoutingConfig> flashRoute;
    struct royale::config::FlashMemoryConfig flashConfig;

    // Check if one of the connected devices is a pico monstar
    // and create the appropriate storage and firmware file
    if (stringStartsWith (m_cameraName, { "PICOMONSTAR" }))
    {
        flashRoute = findSensorRoute (royale::picomonstar::sensorMap, SensorRole::STORAGE_CALIBRATION);
        flashConfig = royale::picomonstar::flashConfig;
    }

    storage = NonVolatileStorageFactory::createStorageReadRandom (bridgeFactory,
              flashConfig,
              flashRoute.get());

    auto flashMemory = StorageFormatPolar (storage);

    auto moduleSerial = flashMemory.getModuleSerialNumber();
    auto moduleId = flashMemory.getModuleIdentifier();
    auto moduleSuffix = flashMemory.getModuleSuffix();

    if (!moduleId.empty())
    {
        // We already have a module identifier
        return;
    }

    auto calibData = flashMemory.getCalibrationData();

    if (m_cameraName == "PICOMONSTAR1" ||
            m_cameraName == "PICOMONSTAR2" ||
            m_cameraName == "PICOMONSTARDEFAULT")
    {
        // Use the product code for the monstar 2, as the module configs don't differ besides the camera name
        moduleId = royale::Vector<uint8_t> ({ 0x01, 0x02, 0x30, 0x01, 0x00, 0x50, 0x51, 0x00, 0x00, 0x30, 0x04, 0x02, 0x06, 0x00, 0x00, 0x00 });
    }
    else if (m_cameraName == "PICOMONSTAR850GLASS")
    {
        moduleId = royale::Vector<uint8_t> ({ 0x01, 0x02, 0x30, 0x01, 0x00, 0x80, 0x54, 0x00, 0x00, 0x30, 0x04, 0x02, 0x06, 0x00, 0x00, 0x00 });
    }

    flashMemory.writeCalibrationData (calibData, moduleId, moduleSuffix, moduleSerial);
}
