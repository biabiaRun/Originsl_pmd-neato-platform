/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <SPIFlashTool.hpp>
#include <SPIStorage_M2452.hpp>
#include <SPIStorage_M2453.hpp>
#include <SPIStorage_M2455.hpp>

#include <stdint.h>
#include <chrono>
#include <sstream>
#include <iomanip>


#include <FileSystem.hpp>
#include <NarrowCast.hpp>
#include <Crc32.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/StringFunctions.hpp>
#include <imager/ImagerM2452.hpp>

#include <storage/StorageFormatPolar.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::storage;
using namespace spiFlashTool;
using namespace spiFlashTool::storage;

namespace spiFlashTool
{
    enum class ImagerType
    {
        M2452,
        M2453,
        M2455,
        UNDEFINED
    };

    class ProgressListener : public IProgressReportListener
    {
    public:
        explicit ProgressListener (SPIFlashTool &ui) :
            m_ui (ui)
        {
        }

        void reportProgress (const std::string &operation, std::size_t x, std::size_t y) override
        {
            auto s = QString ("%1 %2 of %3").arg (QString::fromStdString (operation)).arg (x).arg (y);
            m_ui.updateStatusBox (std::move (s));
        }

    private:
        SPIFlashTool &m_ui;
    };

    class CLProgressListener : public IProgressReportListener
    {
    public:
        CLProgressListener ()
        {
        }

        void reportProgress (const std::string &operation, std::size_t x, std::size_t y) override
        {
            auto s = QString ("%1 %2 of %3").arg (QString::fromStdString (operation)).arg (x).arg (y);
            std::cout << s.toLocal8Bit().toStdString() << std::endl;
        }
    };
}

SPIFlashTool::SPIFlashTool (const SPIFlashToolParameters &params) :
    m_progressListener (nullptr),
    m_params (params)
{
    setupUi (this);
    m_statusBox.hide();

    if (!params.calibrationFilename.empty())
    {
        QMetaObject::invokeMethod (this, "writeToFlashFromParameters", Qt::QueuedConnection);
        //QTimer::singleShot (0, this, SLOT (writeToFlashFromParameters (void)));
    }
}

SPIFlashTool::~SPIFlashTool()
{
    if (m_cameraDevice != nullptr)
    {
        closeCamera();
    }
}

void SPIFlashTool::on_pbOpen_clicked()
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

void SPIFlashTool::openCamera()
{
    // the camera manager will query for connected cameras
    CameraManager manager (ROYALE_ACCESS_CODE_LEVEL4);

    auto camlist = manager.getConnectedCameraList();

    if (camlist.empty())
    {
        QMessageBox::information (this, this->windowTitle(), "No camera found!");
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
        return;
    }

    m_cameraDevice = manager.createCamera (camName);

    if (m_cameraDevice == nullptr)
    {
        QMessageBox::information (this, this->windowTitle(), "Couldn't create camera!");
        return;
    }

    if (m_cameraDevice->getCameraName (camName) != CameraStatus::SUCCESS)
    {
        QMessageBox::information (this, this->windowTitle(), "Couldn't retrieve camera name!");
        m_cameraDevice.reset ();
        return;
    }

    ImagerType imagerType = ImagerType::UNDEFINED;

    if (stringStartsWith (camName, { "Skylla", "Daedalus", "Charybdis", "Rabenscheid", "Willingen", "Alea" }))
    {
        imagerType = ImagerType::M2452;
    }
    else if (stringStartsWith (camName, { "Salome", "FacePlus", "Equinox" }))
    {
        imagerType = ImagerType::M2453;
    }
    else if (stringStartsWith (camName, { "UnknownPmdModule277x", "Holkin" }))
    {
        imagerType = ImagerType::M2455;
    }

    // Check that this is a module which supports reading out the SPI flash over the imager
    if (imagerType == ImagerType::UNDEFINED)
    {
        QMessageBox::information (this, this->windowTitle(), "This tool is not compatible with this module!");
        m_cameraDevice.reset ();
        return;
    }

    m_storage.reset();
    if (m_params.calibrationFilename.empty())
    {
        m_progressListener = new spiFlashTool::ProgressListener (*this);
    }
    else
    {
        m_progressListener = new spiFlashTool::CLProgressListener ();
    }

    try
    {
        switch (imagerType)
        {
            case ImagerType::M2452:
                m_storage = std::make_shared<SPIStorage_M2452> (m_cameraDevice, m_progressListener);
                break;
            case ImagerType::M2453:
                m_storage = std::make_shared<SPIStorage_M2453> (m_cameraDevice, m_progressListener);
                break;
            case ImagerType::M2455:
                m_storage = std::make_shared<SPIStorage_M2455> (m_cameraDevice, m_progressListener);
                break;
            default:
                throw RuntimeError ("Unsupported imager type");
        }
        auto sfp = StorageFormatPolar (m_storage);

        auto moduleIdentifier = sfp.getModuleIdentifier();
        auto moduleSerial = sfp.getModuleSerialNumber();
        auto moduleSuffix = sfp.getModuleSuffix();

        if (!moduleIdentifier.empty ())
        {
            std::stringstream ss;
            for (auto i = 0u; i < moduleIdentifier.size(); ++i)
            {
                ss << std::setfill ('0') << std::setw (2) << std::hex << static_cast<unsigned> (moduleIdentifier[i]);
            }
            std::string strModuleIdent (ss.str());
            leModuleIdentifier->setText (strModuleIdent.c_str());
            on_leModuleIdentifier_editingFinished();
        }

        leModuleSuffix->setText (moduleSuffix.c_str());
        leModuleSerial->setText (moduleSerial.c_str());
    }
    catch (Exception &e)
    {
        m_storage.reset();
        delete m_progressListener;
        m_progressListener = nullptr;

        QMessageBox::information (this, this->windowTitle(), QString (e.what()));
    }

    readSerial();

    lblImagerSerial->setText (QString::fromStdString (m_serialNumber));
    lblCameraName->setText (QString (camName.c_str()));

    pbOpen->setText ("Close camera");
    gbFlash->setEnabled (true);
    pbReadFromFlash->setEnabled (true);
    pbWriteToFlash->setEnabled (true);
}

void SPIFlashTool::closeCamera()
{
    m_cameraDevice.reset ();
    m_calibrationData.clear();
    lblImagerSerial->setText ("");
    lblCameraName->setText ("");
    m_serialNumber.clear();
    if (pbModuleIdentifier->text () != "HEX")
    {
        on_pbModuleIdentifier_clicked();
    }
    if (m_progressListener)
    {
        delete m_progressListener;
        m_progressListener = nullptr;
    }
    m_moduleIdentifierHex.clear();
    m_moduleIdentifierBin.clear();
    leModuleIdentifier->clear();
    leModuleSuffix->clear();
    leModuleSerial->clear();
    gbFlash->setEnabled (false);

    pbOpen->setText ("Open camera");
}

void SPIFlashTool::on_pbReadFromFlash_clicked()
{
    QString filename = QFileDialog::getSaveFileName (this, "Save calibration to", QString::fromStdString (m_serialNumber) + ".bin", "Calibration files (*.bin);;All files (*.*)");

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

    showStatusBox();

    std::chrono::high_resolution_clock::time_point tStart = std::chrono::high_resolution_clock::now();

    auto flashReadSuccessfully = readFlash();

    std::chrono::high_resolution_clock::time_point tEnd = std::chrono::high_resolution_clock::now();
    float timespan = static_cast<float> (std::chrono::duration_cast<std::chrono::milliseconds> (tEnd - tStart).count()) /
                     1000.0f;

    hideStatusBox();

    if (!flashReadSuccessfully)
    {
        QMessageBox::warning (this, this->windowTitle(), "Problem reading calibration from flash!");
        return;
    }

    uint64_t bytesWritten = writeVectorToFile (String (filename.toStdString()), m_calibrationData);

    if (bytesWritten != m_calibrationData.size ())
    {
        QMessageBox::warning (this, this->windowTitle(), "Problem writing calibration to file!");
        return;
    }

    QMessageBox::information (this, this->windowTitle(), QString ("Flash read out successful : ") +
                              QString::number (timespan, 'f', 2) + QString (" sec"));
}

void SPIFlashTool::on_pbWriteToFlash_clicked()
{
    on_leModuleIdentifier_editingFinished();

    auto useFile = !headerOnly->isChecked();
    Vector<uint8_t> calibData;
    if (useFile)
    {
        if (!readFile (calibData))
        {
            return;
        }
    }
    else
    {
        if (!askHeaderOnly())
        {
            return;
        }
    }

    showStatusBox();

    std::chrono::high_resolution_clock::time_point tStart = std::chrono::high_resolution_clock::now();

    auto flashWrittenSuccessfully = writeFlash (calibData);

    std::chrono::high_resolution_clock::time_point tEnd = std::chrono::high_resolution_clock::now();
    float timespan = static_cast<float> (std::chrono::duration_cast<std::chrono::milliseconds> (tEnd - tStart).count()) /
                     1000.0f;

    hideStatusBox();

    if (!flashWrittenSuccessfully)
    {
        QMessageBox::warning (this, this->windowTitle(), "Problem writing calibration to the flash!");
        return;
    }

    QMessageBox::information (this, this->windowTitle(), QString ("Flash successfully written : ") +
                              QString::number (timespan, 'f', 2) + QString (" sec"));
}

bool SPIFlashTool::readFlash()
{
    if (!m_storage)
    {
        return false;
    }

    try
    {
        auto sfp = StorageFormatPolar (m_storage);
        auto flashContent = sfp.getCalibrationData();
        m_calibrationData.swap (std::move (flashContent));
    }
    catch (Exception &e)
    {
        QMessageBox::information (this, this->windowTitle(), QString (e.what()));
        return false;
    }
    return true;
}

bool SPIFlashTool::writeFlash (const Vector<uint8_t> &calibData)
{
    try
    {
        auto sfp = StorageFormatPolar (m_storage);

        std::stringstream ss;
        ss << std::setw (32) << std::setfill ('0') << m_moduleIdentifierHex;

        auto strIdent = ss.str ();
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

        sfp.writeCalibrationData (calibData, ident, suffix, serial);
    }
    catch (Exception &e)
    {
        QMessageBox::information (this, this->windowTitle(), QString (e.what()));
        return false;
    }
    return true;
}

bool SPIFlashTool::readFile (royale::Vector<uint8_t> &calibData)
{
    QString filename = QFileDialog::getOpenFileName (this, "Calibration that should be flashed", QString::fromStdString (m_serialNumber) + ".bin", "Calibration files (*.bin);;All files (*.*)");

    if (filename.isEmpty())
    {
        return false;
    }

    if (!fileexists (String (filename.toStdString())))
    {
        return false;
    }

    if (QMessageBox::No == QMessageBox::question (this, this->windowTitle(), "Do you really want to flash " + filename + " onto the device?"))
    {
        return false;
    }

    if (!readFileToVector (String (filename.toStdString()), calibData))
    {
        QMessageBox::warning (this, this->windowTitle(), "Problem reading calibration from file!");
        return false;
    }
    return true;
}

bool SPIFlashTool::askHeaderOnly()
{
    return QMessageBox::question (this, this->windowTitle(),
                                  "Do you really want to flash only the header to the device? "
                                  "If the flash already contains a calibration, the calibration "
                                  "is destroyed by writing this header",
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
}

void SPIFlashTool::showStatusBox()
{
    QApplication::setOverrideCursor (Qt::WaitCursor);

    m_statusBox.setStandardButtons (0);
    m_statusBox.setText ("Accessing flash ...\nPlease do not disconnect the camera!");
    m_statusBox.setModal (true);
    m_statusBox.show();
    QCoreApplication::processEvents();
}

void SPIFlashTool::hideStatusBox()
{
    m_statusBox.hide();
    QApplication::restoreOverrideCursor();
}

void SPIFlashTool::updateStatusBox (const QString &message)
{
    m_statusBox.setInformativeText (message);
    QCoreApplication::processEvents();
}

void SPIFlashTool::readSerial()
{
    if (!m_storage)
    {
        return;
    }

    try
    {
        auto eFuseRegisters = m_storage->getEFuseRegisters();

        m_cameraDevice->readRegisters (eFuseRegisters);

        m_serialNumber = royale::imager::ImagerM2452Serial (
                             static_cast<uint16_t> (eFuseRegisters.at (0).second),
                             static_cast<uint16_t> (eFuseRegisters.at (1).second),
                             static_cast<uint16_t> (eFuseRegisters.at (2).second),
                             static_cast<uint16_t> (eFuseRegisters.at (3).second)
                         ).serialNumber;
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

const char *SPIFlashTool::hex_to_bin_char (char c)
{
    switch (toupper (c))
    {
        case '0':
            return "0000";
        case '1':
            return "0001";
        case '2':
            return "0010";
        case '3':
            return "0011";
        case '4':
            return "0100";
        case '5':
            return "0101";
        case '6':
            return "0110";
        case '7':
            return "0111";
        case '8':
            return "1000";
        case '9':
            return "1001";
        case 'A':
            return "1010";
        case 'B':
            return "1011";
        case 'C':
            return "1100";
        case 'D':
            return "1101";
        case 'E':
            return "1110";
        case 'F':
            return "1111";
        default:
            return "";
    }
}

void SPIFlashTool::on_leModuleIdentifier_editingFinished()
{
    if (pbModuleIdentifier->text() == "HEX")
    {
        // Convert from hex to bin
        auto hexString = leModuleIdentifier->text().toStdString();

        std::string binString;
        for (auto i = 0u; i < hexString.length(); ++i)
        {
            binString += hex_to_bin_char (hexString[i]);
        }

        m_moduleIdentifierHex = hexString;
        m_moduleIdentifierBin = binString;
    }
    else
    {
        // Convert from bin to hex
        auto binString = leModuleIdentifier->text().toStdString();

        if (binString.length() % 4u != 0u)
        {
            for (auto i = 0u; i < binString.length() % 4u; ++i)
            {
                binString = std::string ("0") + binString;
            }
        }

        std::stringstream ss;

        for (auto i = 0u; i < binString.length(); i += 4u)
        {
            int result = 0;
            for (auto j = 0; j < 4; ++j)
            {
                result *= 2;
                result += binString[i + j] == '1' ? 1 : 0;
            }

            ss << std::hex << result;
        }

        m_moduleIdentifierHex = ss.str();
        m_moduleIdentifierBin = binString;
    }
}

void SPIFlashTool::on_pbModuleIdentifier_clicked()
{
    if (pbModuleIdentifier->text() == "HEX")
    {
        pbModuleIdentifier->setText ("BIN");
        pbModuleIdentifier->setToolTip ("Switch to hexadecimal representation");
        QString miInputMask;
        miInputMask.fill ('B', 128);
        leModuleIdentifier->setInputMask (miInputMask);
        leModuleIdentifier->setToolTip ("128 Bit Binary");
        leModuleIdentifier->setText (m_moduleIdentifierBin.c_str());
    }
    else
    {
        pbModuleIdentifier->setText ("HEX");
        pbModuleIdentifier->setToolTip ("Switch to binary representation");
        QString miInputMask;
        miInputMask.fill ('H', 32);
        leModuleIdentifier->setInputMask (miInputMask);
        leModuleIdentifier->setToolTip ("16 Byte Hexadecimal");
        leModuleIdentifier->setText (m_moduleIdentifierHex.c_str());
    }
}

void SPIFlashTool::writeToFlashFromParameters ()
{
    QCoreApplication::processEvents();

    // Be careful, there is almost no checking here
    if (!fileexists (String (m_params.calibrationFilename)))
    {
        std::cout << "Calibration file not found!" << std::endl;
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

    Vector<uint8_t> calibData;
    if (!readFileToVector (String (m_params.calibrationFilename), calibData))
    {
        std::cout << "Problem reading calibration from file!" << std::endl;
        QApplication::quit();
        return;
    }

    std::cout << "Trying to flash " << m_params.calibrationFilename << std::endl;

    try
    {
        auto sfp = StorageFormatPolar (m_storage);

        std::stringstream ss;
        ss << std::setw (32) << std::setfill ('0') << m_params.moduleIdentifierHex;

        auto strIdent = ss.str();
        strIdent.resize (32, 0);
        Vector<uint8_t> ident;
        ident.resize (16);
        for (auto i = 0u; i < 16u && i < strIdent.size(); ++i)
        {
            auto tmpStr = strIdent.substr (i * 2, 2);
            ident[i] = narrow_cast<uint8_t> (strtol (tmpStr.c_str(), NULL, 16));
        }

        String suffix (m_params.moduleSuffix);
        String serial (m_params.moduleSerialNumber);

        sfp.writeCalibrationData (calibData, ident, suffix, serial);
    }
    catch (...)
    {
        std::cout << "Problem reading calibration from file!" << std::endl;
        QApplication::quit();
        return;
    }

    std::cout << "Successfully wrote to flash!" << std::endl;
    QApplication::quit();
}
