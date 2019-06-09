/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <flashtool.hpp>

#include <stdint.h>

#include <FileSystem.hpp>
#include <FlashDatav7.hpp>
#include <FlashDatav100.hpp>

using namespace royale;
using namespace royale::common;

FlashTool::FlashTool()
{
    setupUi (this);
}

FlashTool::~FlashTool()
{
    if (m_flashData != nullptr)
    {
        closeCamera();
    }
}

void FlashTool::on_pbOpen_clicked()
{
    if (m_flashData == nullptr)
    {
        openCamera();
    }
    else
    {
        closeCamera();
    }
}

void FlashTool::openCamera()
{
    // the camera manager will query for connected cameras
    CameraManager manager (ROYALE_ACCESS_CODE_LEVEL3);

    auto camlist = manager.getConnectedCameraList();

    String camName;

    if (camlist.empty())
    {

    }
    else if (camlist.size() == 1)
    {
        camName = camlist[0];
    }
    else
    {
        // Let the user choose the camera
    }

    std::unique_ptr<royale::ICameraDevice> cameraDevice = manager.createCamera (camName);

    if (cameraDevice == nullptr)
    {
        QMessageBox::information (this, this->windowTitle(), "Couldn't create camera!");
        return;
    }

    cameraDevice->setCallbackData (CallbackData::Raw);

    CameraStatus ret = cameraDevice->initialize();
    if (ret != CameraStatus::SUCCESS)
    {
        QMessageBox::information (this, this->windowTitle(), "Couldn't initialize the camera!");
        return;
    }

    ret = cameraDevice->getId (m_serialNumber);
    if (ret != CameraStatus::SUCCESS)
    {
        cameraDevice.reset (nullptr);
        QMessageBox::information (this, this->windowTitle(), "Couldn't read camera serial number!");
        return;
    }

    String cameraName;
    ret = cameraDevice->getCameraName (cameraName);
    if (ret != CameraStatus::SUCCESS)
    {
        cameraDevice.reset (nullptr);
        QMessageBox::information (this, this->windowTitle(), "Couldn't read camera name!");
        return;
    }

    Vector<Pair<String, String>> camInfo;
    cameraDevice->getCameraInfo (camInfo);

    QGridLayout *layout = new QGridLayout (flashDataWidget);
    flashDataWidget->setLayout (layout);
    QWidget *flashWidget = new QWidget();
    flashWidget->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    layout->addWidget (flashWidget, 0, 0);

    // Create a map to make the parsing easier
    m_camInfoMap.clear();
    for (auto infoEntry : camInfo)
    {
        m_camInfoMap[infoEntry.first.toStdString()] = infoEntry.second.toStdString();
    }

    if (m_camInfoMap["BRIDGE_TYPE"] == "Enclustra")
    {
        m_flashData.reset (new FlashDataV100 (std::move (cameraDevice), flashWidget));
    }
    else if (m_camInfoMap["BRIDGE_TYPE"] == "UVC" ||
             m_camInfoMap["BRIDGE_TYPE"] == "Amundsen")
    {
        m_flashData.reset (new FlashDataV7 (std::move (cameraDevice), flashWidget));
    }
    else
    {
        cameraDevice.reset (nullptr);
        QMessageBox::information (this, this->windowTitle(), "Bridgetype not supported!");
        return;
    }

    flashDataWidget->show();

    retrieveFlashInformation();
    pbOpen->setText ("Close camera");

    lbImagerSerial->setText (m_serialNumber.c_str());
    lbCameraName->setText (cameraName.c_str());
}

void FlashTool::closeCamera()
{
    m_flashData.reset (nullptr);
    gbFlash->setEnabled (false);
    cbCalibAvailable->setChecked (false);
    flashDataWidget->hide();
    lbCalibSize->setText ("");
    lbImagerSerial->clear();
    lbCameraName->clear();
    pbOpen->setText ("Open camera");
}

void FlashTool::on_pbSaveFromFlash_clicked()
{
    QString filename = QFileDialog::getSaveFileName (this, "Save calibration to",
                       QString::fromStdString (m_serialNumber.toStdString()) + ".bin",
                       "Calibration files (*.bin *.cal *.spc *.jgf);;All files (*.*)");

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

    auto calibData = m_flashData->getCalibrationData();
    if (calibData.empty())
    {
        QMessageBox::warning (this, this->windowTitle(), "Calibration data empty!");
        return;
    }

    uint64_t bytesWritten = writeVectorToFile (String (filename.toStdString()), calibData);

    if (bytesWritten != calibData.size())
    {
        QMessageBox::warning (this, this->windowTitle(), "Problem writing calibration to file!");
    }
}

void FlashTool::on_pbSaveToFlash_clicked()
{
    QString filename = QFileDialog::getOpenFileName (this, "Calibration that should be flashed",
                       QString::fromStdString (m_serialNumber.toStdString()) + ".bin",
                       "Calibration files (*.bin *.cal *.spc *.jgf);;All files (*.*)");

    if (filename.isEmpty())
    {
        return;
    }

    if (!fileexists (String (filename.toStdString())))
    {
        return;
    }

    if (QMessageBox::No == QMessageBox::question (this, this->windowTitle(), "Do you really want to flash " + filename + " onto the device?"))
    {
        return;
    }

    Vector<uint8_t> data;
    readFileToVector (String (filename.toStdString()), data);
    m_flashData->setCalibrationData (data);
    m_flashData->writeData();

    retrieveFlashInformation();
}

void FlashTool::retrieveFlashInformation()
{
    m_flashData->readData();

    auto calibData = m_flashData->getCalibrationData();

    gbFlash->setEnabled (true);

    if (calibData.empty())
    {
        cbCalibAvailable->setChecked (false);
        lbCalibSize->setText ("0");
        pbSaveFromFlash->setEnabled (false);
    }
    else
    {
        cbCalibAvailable->setChecked (true);
        lbCalibSize->setText (QString::number (calibData.size()));
        pbSaveFromFlash->setEnabled (true);
    }
    pbSaveToFlash->setEnabled (true);
}
