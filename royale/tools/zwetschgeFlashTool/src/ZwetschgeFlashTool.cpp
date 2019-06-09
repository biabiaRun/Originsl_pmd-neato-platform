/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <ZwetschgeFlashTool.hpp>
#include <SPIStorageHelper.hpp>

#include <chrono>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdint.h>
#include <thread>

#include <FileSystem.hpp>
#include <NarrowCast.hpp>
#include <Crc32.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/StringFunctions.hpp>
#include <config/FlashMemoryConfig.hpp>
#include <imager/ImagerSimpleHexSerialNumber.hpp>

#include <storage/StorageFormatZwetschge.hpp>
#include <storage/StorageBuffer.hpp>

#include "UseCaseDetails.hpp"

using namespace royale;
using namespace royale::common;
using namespace royale::config;
using namespace royale::storage;
using namespace spiFlashTool::storage;

namespace
{
    const auto ZWETSCHGE_TOC_MAGIC = std::array<uint8_t, 9>
                                     {
                                         {
                                             'Z', 'W', 'E', 'T', 'S', 'C', 'H', 'G', 'E'
                                         }
                                     };
}

ZwetschgeFlashTool::ZwetschgeFlashTool (const ZwetschgeFlashToolParameters &params) :
    m_params (params),
    m_clVersion (false)
{
    setupUi (this);
    m_statusBox.hide();

    auto regMapLayout = dynamic_cast<QGridLayout *> (scrollAreaWidgetContents->layout());
    m_regMapInit = new RegisterMapTable();
    regMapLayout->addWidget (m_regMapInit, 0, 1);
    m_regMapFw1 = new RegisterMapTable();
    regMapLayout->addWidget (m_regMapFw1, 1, 1);
    m_regMapFw2 = new RegisterMapTable();
    regMapLayout->addWidget (m_regMapFw2, 2, 1);
    m_regMapFwStart = new RegisterMapTable();
    regMapLayout->addWidget (m_regMapFwStart, 3, 1);
    m_regMapStart = new RegisterMapTable();
    regMapLayout->addWidget (m_regMapStart, 4, 1);
    m_regMapStop = new RegisterMapTable();
    regMapLayout->addWidget (m_regMapStop, 5, 1);


    lbProgress->setVisible (false);
    flashProgress->setVisible (false);

    QObject::connect (lstUseCases, SIGNAL (itemSelectionChanged (void)), this, SLOT (lstSelectionChanged (void)));

    if (!params.calibFilename.empty())
    {
        m_clVersion = true;
        QMetaObject::invokeMethod (this, "extractCalibFromZwetschge", Qt::QueuedConnection);
    }
    if ( (!params.zwetschgeFilename.empty()) && params.calibFilename.empty())
    {
        m_clVersion = true;
        QMetaObject::invokeMethod (this, "writeToFlashFromParameters", Qt::QueuedConnection);
    }
}

ZwetschgeFlashTool::~ZwetschgeFlashTool()
{
    if (m_cameraDevice != nullptr)
    {
        closeCamera();
    }
}

void ZwetschgeFlashTool::on_pbOpen_clicked()
{
    if (m_cameraDevice == nullptr)
    {
        openCamera();
    }
    else
    {
        closeCamera();
    }
}

void ZwetschgeFlashTool::on_pbLoadFile_clicked()
{
    QSettings myAppSettings;

    QString filename = QFileDialog::getOpenFileName (this, "Zwetschge file to load", myAppSettings.value ("last_dir").toString(), "Zwetschge (*.zwetschge);;All files (*.*)");

    if (filename.isEmpty())
    {
        return;
    }

    QDir currentDir;
    myAppSettings.setValue ("last_dir", currentDir.absoluteFilePath (filename));

    clearFields (true);

    try
    {
        readZwetschge (filename);
    }
    catch (Exception &e)
    {
        QMessageBox::information (this, this->windowTitle(), QString (e.what()));
    }

    if (m_cameraDevice)
    {
        pbWriteToFlash->setEnabled (true);
    }
    m_zwetschgeFilename = filename;
}

void ZwetschgeFlashTool::openCamera()
{
    clearFields (false);

    showStatusBox();

    QCoreApplication::processEvents();

    // the camera manager will query for connected cameras
    CameraManager manager (ROYALE_ACCESS_CODE_LEVEL4);

    auto camlist = manager.getConnectedCameraList();

    if (camlist.empty())
    {
        QMessageBox::information (this, this->windowTitle(), "No camera found!");
        hideStatusBox();
        return;
    }

    String camName;

    if (camlist.size() == 1)
    {
        camName = camlist[0];
    }
    else
    {
        QMessageBox::information (this, this->windowTitle(), "Please connect only one camera!");
        hideStatusBox();
        return;
    }

    m_cameraDevice = manager.createCamera (camName);

    if (m_cameraDevice == nullptr)
    {
        QMessageBox::information (this, this->windowTitle(), "Couldn't create camera!");
        hideStatusBox();
        return;
    }

    if (m_cameraDevice->getCameraName (camName) != CameraStatus::SUCCESS)
    {
        QMessageBox::information (this, this->windowTitle(), "Couldn't retrieve camera name!");
        m_cameraDevice.reset ();
        hideStatusBox();
        return;
    }

    flashProgress->setValue (0);
    lbProgress->setVisible (true);
    flashProgress->setVisible (true);

    m_storage.reset();
    try
    {
        m_storage = createSPIStorage (m_cameraDevice, this, 0);
        loadZwetschgeData (m_storage);
        loadProtectedArea (m_storage);
    }
    catch (Exception &e)
    {
        QMessageBox::information (this, this->windowTitle(), QString (e.what()));
    }

    lbProgress->setVisible (false);
    flashProgress->setVisible (false);

    readSerial();

    lblCameraName->setText (camName.c_str());
    lblImagerSerial->setText (m_serialNumber.c_str());

    pbOpen->setText ("Close camera");
    pbLoadFile->setEnabled (true);
    pbWriteToFlash->setEnabled (false);
    m_zwetschgeFilename.clear();

    hideStatusBox();
}

void ZwetschgeFlashTool::closeCamera()
{
    clearFields (false);

    lblCameraName->clear();
    lblImagerSerial->clear();

    m_storage.reset();
    m_cameraDevice.reset ();

    m_cameraDevice = nullptr;

    pbWriteToFlash->setEnabled (false);

    pbOpen->setText ("Open camera");
}

void ZwetschgeFlashTool::showStatusBox()
{
    QApplication::setOverrideCursor (Qt::WaitCursor);

    m_statusBox.setStandardButtons (0);
    m_statusBox.setText ("Accessing flash ...\nPlease do not disconnect the camera!");
    m_statusBox.setModal (true);
    m_statusBox.show();
    QCoreApplication::processEvents();
}

void ZwetschgeFlashTool::hideStatusBox()
{
    m_statusBox.hide();
    QApplication::restoreOverrideCursor();
}

void ZwetschgeFlashTool::updateStatusBox (const QString &message)
{
    m_statusBox.setInformativeText (message);
    QCoreApplication::processEvents();
}

void ZwetschgeFlashTool::writeToFlashFromParameters()
{
    QCoreApplication::processEvents();

    // Be careful, there is almost no checking here
    if (!fileexists (String (m_params.zwetschgeFilename)))
    {
        std::cout << "Zwetschge file not found!" << std::endl;
        QApplication::quit();
        return;
    }

    openCamera();

    if (!m_storage)
    {
        std::cout << "No camera found!" << std::endl;
        QApplication::quit();
        return;
    }

    m_zwetschgeFilename = QString::fromStdString (m_params.zwetschgeFilename);

    std::cout << "Trying to flash " << m_params.zwetschgeFilename << std::endl;

    try
    {
        writeZwetschge();
    }
    catch (...)
    {
        std::cout << "Problem flashing Zwetschge file!" << std::endl;
        QApplication::quit();
        return;
    }

    std::cout << "Successfully wrote to flash!" << std::endl;
    QApplication::quit();
}

void ZwetschgeFlashTool::extractCalibFromZwetschge()
{
    QCoreApplication::processEvents();

    // Be careful, there is almost no checking here
    if (!fileexists (String (m_params.zwetschgeFilename)))
    {
        std::cout << "Zwetschge file not found!" << std::endl;
        QApplication::quit();
        return;
    }
    try
    {
        readZwetschge (QString::fromStdString (m_params.zwetschgeFilename));
    }
    catch (Exception &e)
    {
        std::cout << "Could not load zwetschge " << m_params.zwetschgeFilename << " Exception: " << e.what() << std::endl;
        QApplication::quit();
        return;
    }
    m_zwetschgeFilename = QString::fromStdString (m_params.zwetschgeFilename);
    if (m_calibrationData.empty())
    {
        return;
    }

    QString filename = QString::fromStdString (m_params.calibFilename);

    if (filename.isEmpty())
    {
        return;
    }

    auto bytesWritten = static_cast<std::size_t> (writeVectorToFile (String (filename.toStdString()), m_calibrationData));
    if (bytesWritten != m_calibrationData.size())
    {
        std::cout << "Error writing calibration file " << filename.toStdString() << " to the hard disk!" << std::endl;
    }

    std::cout << "Successfully extracted calib" << std::endl;
    QApplication::quit();

}

void ZwetschgeFlashTool::loadZwetschgeData (std::shared_ptr<royale::pal::IStorageReadRandom> storageAccess)
{
    auto sfz = StorageFormatZwetschge (storageAccess);

    try
    {
        auto externalConfig = sfz.getExternalConfig (true, true);
        m_useCaseList = externalConfig.imagerExternalConfig->getUseCaseList();
        for (auto curUseCase : m_useCaseList)
        {
            lstUseCases->addItem (curUseCase.name.c_str());
        }
        lblZwetschgeChecksum->setText (QString::number (sfz.getZwetschgeCrc()));

        m_calibrationData.clear();
        pbSaveCalibration->setEnabled (false);
        try
        {
            m_calibrationData = externalConfig.calibration->getCalibrationData();
            lblCalibSize->setText (QString::number (m_calibrationData.size()));
            lblCalibDataChksum->setText ("0x" + QString::number (externalConfig.calibration->getCalibrationDataChecksum(), 16)
                                         .rightJustified (8, '0'));
            pbSaveCalibration->setEnabled (true);
        }
        catch (...)
        {
            lblCalibSize->setText ("0");
        }

        auto moduleIdentifier = externalConfig.calibration->getModuleIdentifier();
        m_moduleSerial = externalConfig.calibration->getModuleSerialNumber().toStdString();
        auto moduleSuffix = externalConfig.calibration->getModuleSuffix();

        if (!moduleIdentifier.empty())
        {
            std::stringstream ss;
            for (auto i = 0u; i < moduleIdentifier.size(); ++i)
            {
                ss << std::setfill ('0') << std::setw (2) << std::hex << static_cast<unsigned> (moduleIdentifier[i]);
            }
            std::string strModuleIdent (ss.str());
            lblModuleIdentifier->setText (strModuleIdent.c_str());
        }

        lblModuleSerial->setText (m_moduleSerial.c_str());
        lblModuleSuffix->setText (moduleSuffix.c_str());

        m_regMapInit->addRegisterMapList (externalConfig.imagerExternalConfig->getInitializationMap());
        m_regMapFw1->addRegisterMapList (externalConfig.imagerExternalConfig->getFirmwarePage1());
        m_regMapFw2->addRegisterMapList (externalConfig.imagerExternalConfig->getFirmwarePage2());
        m_regMapFwStart->addRegisterMapList (externalConfig.imagerExternalConfig->getFirmwareStartMap());
        m_regMapStart->addRegisterMapList (externalConfig.imagerExternalConfig->getStartMap());
        m_regMapStop->addRegisterMapList (externalConfig.imagerExternalConfig->getStopMap());
    }
    catch (...)
    {
        QMessageBox::information (this, this->windowTitle(), "Problem reading Zwetschge file from flash!");
    }
}

void ZwetschgeFlashTool::on_pbWriteToFlash_clicked()
{
    lbProgress->setText ("");
    flashProgress->setValue (0);
    lbProgress->setVisible (true);
    flashProgress->setVisible (true);

    showStatusBox();

    try
    {
        writeZwetschge();
    }
    catch (...)
    {
        QMessageBox::information (this, this->windowTitle(), "Problem flashing Zwetschge file!");
    }

    lbProgress->setVisible (false);
    flashProgress->setVisible (false);

    hideStatusBox();
}

void ZwetschgeFlashTool::clearFields (bool keepFlashData)
{
    lblModuleIdentifier->clear();
    lblModuleSerial->clear();
    lblCalibSize->clear();
    lblCalibDataChksum->clear();
    lblZwetschgeChecksum->clear();

    lstUseCases->clear();

    m_useCaseList.clear();

    if (!keepFlashData)
    {
        lblModuleIdentifierProt->clear();
        lblADCGain->clear();
        lblADCVirtualOffset->clear();
        lblADCOffset->clear();
        lblRCOTrim->clear();
        lblProtCRC->clear();
    }
}

void ZwetschgeFlashTool::readSerial()
{
    if (!m_storage)
    {
        return;
    }

    try
    {
        auto eFuseRegisters = m_storage->getEFuseRegisters();
        m_cameraDevice->readRegisters (eFuseRegisters);

        std::vector<uint16_t> regs;
        for (const auto &x : eFuseRegisters)
        {
            regs.emplace_back (static_cast<uint16_t> (x.second));
        }

        m_serialNumber = royale::imager::ImagerSimpleHexSerialNumber (regs).toString();
    }
    catch (const std::exception &e)
    {
        auto message = QStringLiteral ("Error reading out serial number: %1").arg (e.what());
        QMessageBox::information (this, this->windowTitle(), message);
        m_serialNumber = "";
        return;
    }
    catch (...)
    {
        QMessageBox::information (this, this->windowTitle(), "Error reading out serial number!");
        m_serialNumber = "";
        return;
    }
}

void ZwetschgeFlashTool::writeZwetschge()
{
    std::vector<uint8_t> zwetschgeData;
    readFileToStdVector (m_zwetschgeFilename.toStdString(), zwetschgeData);

    const auto readOnlyOffset = 0x2000;

    if (zwetschgeData.size() < readOnlyOffset &&
            !std::equal (ZWETSCHGE_TOC_MAGIC.begin(), ZWETSCHGE_TOC_MAGIC.end(), zwetschgeData.begin()))
    {
        throw RuntimeError ("Zwetschge file too small");
    }

    if (!std::equal (ZWETSCHGE_TOC_MAGIC.begin(), ZWETSCHGE_TOC_MAGIC.end(), zwetschgeData.begin()) &&
            std::equal (ZWETSCHGE_TOC_MAGIC.begin(), ZWETSCHGE_TOC_MAGIC.end(), zwetschgeData.begin() + readOnlyOffset))
    {
        zwetschgeData.erase (zwetschgeData.begin(), zwetschgeData.begin() + readOnlyOffset);
    }
    else if (!std::equal (ZWETSCHGE_TOC_MAGIC.begin(), ZWETSCHGE_TOC_MAGIC.end(), zwetschgeData.begin()) &&
             !std::equal (ZWETSCHGE_TOC_MAGIC.begin(), ZWETSCHGE_TOC_MAGIC.end(), zwetschgeData.begin() + readOnlyOffset))
    {
        throw RuntimeError ("No Zwetschge file");
    }

    m_storage->writeStorage (readOnlyOffset, zwetschgeData);
}

void ZwetschgeFlashTool::readZwetschge (QString filename)
{
    FlashMemoryConfig flashMemoryConfig;

    std::vector<uint8_t> zwetschgeData;
    readFileToStdVector (filename.toStdString(), zwetschgeData);

    const auto readOnlyOffset = 0x2000;

    if (zwetschgeData.size() > ZWETSCHGE_TOC_MAGIC.size() &&
            std::equal (ZWETSCHGE_TOC_MAGIC.begin(), ZWETSCHGE_TOC_MAGIC.end(), zwetschgeData.begin()))
    {
        // This seems to be a Zwetschge without reserved area
        zwetschgeData.insert (zwetschgeData.begin(), readOnlyOffset, 0u);
    }

    auto storage = std::make_shared<StorageBuffer> (flashMemoryConfig, zwetschgeData);
    loadZwetschgeData (storage);
}

void ZwetschgeFlashTool::reportProgress (const std::string &operation, std::size_t x, std::size_t y)
{
    if (m_clVersion)
    {
        auto s = QString ("%1 %2 of %3").arg (QString::fromStdString (operation)).arg (x).arg (y);
        std::cout << s.toLocal8Bit().toStdString() << std::endl;
    }
    else
    {
        lbProgress->setText (QString::fromStdString (operation));
        flashProgress->setMaximum (static_cast<int> (y));
        flashProgress->setValue (static_cast<int> (x));
        QCoreApplication::processEvents();
    }
}

void ZwetschgeFlashTool::on_pbShowDetails_clicked()
{
    if (lstUseCases->selectedItems().empty())
    {
        return;
    }

    auto selectedItemName = lstUseCases->selectedItems().at (0)->text().toStdString();

    bool found = false;
    royale::imager::IImagerExternalConfig::UseCaseData foundUseCase;
    for (auto &curUseCase : m_useCaseList)
    {
        if (curUseCase.name == selectedItemName)
        {
            foundUseCase = curUseCase;
            found = true;
            break;
        }
    }

    if (!found)
    {
        return;
    }

    UseCaseDetails *useCaseDetails = new UseCaseDetails ();

    useCaseDetails->lblName->setText (foundUseCase.name.c_str());
    useCaseDetails->lblWaitTime->setText (QString::number (foundUseCase.waitTime.count()));

    QString blockSizeStr = "";
    for (auto blockSize : foundUseCase.imageStreamBlockSizes)
    {
        blockSizeStr += QString::number (blockSize) + ", ";
    }
    if (blockSizeStr.size() > 2)
    {
        blockSizeStr = blockSizeStr.left (blockSizeStr.size() - 2);
    }
    useCaseDetails->lblBlockSizes->setText (blockSizeStr);

    QString modFreqStr = "";
    for (auto modFreq : foundUseCase.modulationFrequencies)
    {
        modFreqStr += QString::number (modFreq) + ", ";
    }
    if (modFreqStr.size() > 2)
    {
        modFreqStr = modFreqStr.left (modFreqStr.size() - 2);
    }
    useCaseDetails->lblModFreqs->setText (modFreqStr);

    std::stringstream guid;
    for (auto i = 0u; i < foundUseCase.guid.data().size(); ++i)
    {
        guid << std::hex << static_cast<int> (foundUseCase.guid.data() [i]);
        if (i == 3 || i == 5 || i == 7 || i == 9)
        {
            guid << "-";
        }
    }
    useCaseDetails->lblGuid->setText (guid.str().c_str());

    if (foundUseCase.sequentialRegisterHeader.empty())
    {
        useCaseDetails->lblType->setText ("Timed register map");

        useCaseDetails->gbSequential->setVisible (false);
        for (auto curValue : foundUseCase.registerMap)
        {
            useCaseDetails->m_registerMapTable->addEntry (curValue);
        }
    }
    else
    {
        useCaseDetails->lblType->setText ("Sequential register map");
        useCaseDetails->lblFlashAddress->setText (QString ("0x%1").arg
                (foundUseCase.sequentialRegisterHeader.flashConfigAddress, 4, 16, QChar ('0')));
        useCaseDetails->lblFlashSize->setText (QString::number (foundUseCase.sequentialRegisterHeader.flashConfigSize));
        useCaseDetails->lblImagerAddress->setText (QString ("0x%1").arg
                (foundUseCase.sequentialRegisterHeader.imagerAddress, 4, 16, QChar ('0')));

        for (auto curValue : foundUseCase.registerMap)
        {
            useCaseDetails->m_registerMapTable->addEntry (curValue);
        }
    }

    useCaseDetails->exec();

    delete useCaseDetails;
}

void ZwetschgeFlashTool::lstSelectionChanged()
{
    if (lstUseCases->selectedItems().empty())
    {
        pbShowDetails->setEnabled (false);
    }
    else
    {
        pbShowDetails->setEnabled (true);
    }
}

void ZwetschgeFlashTool::on_pbSaveCalibration_clicked()
{
    if (m_calibrationData.empty())
    {
        return;
    }

    QString filename = QFileDialog::getSaveFileName (this, "Save calibration to", QString::fromStdString (m_moduleSerial) + ".jgf", "Calibration files (*.jgf);;All files (*.*)");

    if (filename.isEmpty())
    {
        return;
    }

    if (fileexists (String (filename.toStdString())))
    {
        if (QMessageBox::No == QMessageBox::question (this, this->windowTitle(), "Do you want to overwrite the file?"))
        {
            return;
        }
    }

    std::size_t bytesWritten = static_cast<std::size_t> (writeVectorToFile (String (filename.toStdString()), m_calibrationData));
    if (bytesWritten != m_calibrationData.size())
    {
        QMessageBox::warning (this, this->windowTitle(), "Error writing calibration file to the hard disk!");
    }
}

void ZwetschgeFlashTool::loadProtectedArea (std::shared_ptr<royale::pal::IStorageReadRandom> storageAccess)
{
    std::vector<uint8_t> bufProtectedArea (32);
    storageAccess->readStorage (0u, bufProtectedArea);

    std::stringstream ss;
    for (auto i = 0u; i < 16; ++i)
    {
        ss << std::setfill ('0') << std::setw (2) << std::hex << static_cast<unsigned> (bufProtectedArea[i]);
    }
    std::string strModuleIdent (ss.str());
    lblModuleIdentifierProt->setText (strModuleIdent.c_str());

    uint8_t adcGain;
    uint8_t adcVOffset;
    uint8_t adcOffset;
    uint16_t rcoTrim;
    uint16_t crc;

    adcGain = bufProtectedArea[0x10];
    adcVOffset = (bufProtectedArea[0x11] & 0xC0) >> 6;
    adcOffset = bufProtectedArea[0x11] & 0x3F;
    rcoTrim = static_cast<uint16_t> ( (bufProtectedArea[0x12] << 8) + bufProtectedArea[0x13]);
    crc = static_cast<uint16_t> ( (bufProtectedArea[0x14] << 8) + bufProtectedArea[0x15]);

    lblADCGain->setText (QString::number (adcGain));
    lblADCVirtualOffset->setText (QString::number (adcVOffset));
    lblADCOffset->setText (QString::number (adcOffset));
    lblRCOTrim->setText (QString::number (rcoTrim));
    lblProtCRC->setText (QString::number (crc));
}
