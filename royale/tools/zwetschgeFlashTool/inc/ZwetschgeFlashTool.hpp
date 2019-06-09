/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <QtWidgets>

#include <royale.hpp>
#include <SPIStorageHardwareSpi.hpp>

#include <config/IImagerExternalConfig.hpp>

#include <RegisterMapTable.hpp>

#include <ui_mainwindow.h>

#include <memory>

struct ZwetschgeFlashToolParameters
{
    /**
     * If true, assume the imager is an M2455 even if the name isn't recognised as an M2455-based
     * device. For example, this would allow the Animator firmware to be used, even if Royale's
     * UsbProbeDataListRoyale.cpp expects that to be an M2453 device.
     */
    bool forceM2455 = false;
    std::string zwetschgeFilename;
    std::string calibFilename;
};

class ZwetschgeFlashTool :
    public QMainWindow,
    public Ui::ZwetschgeFlashToolWindow,
    public spiFlashTool::storage::IProgressReportListener
{
    Q_OBJECT

public:
    explicit ZwetschgeFlashTool (const ZwetschgeFlashToolParameters &params);
    ~ZwetschgeFlashTool();

    void reportProgress (const std::string &operation, std::size_t x, std::size_t y) override;

protected slots :
    void on_pbOpen_clicked();

    void on_pbLoadFile_clicked();
    void on_pbWriteToFlash_clicked();

    void on_pbShowDetails_clicked();
    void on_pbSaveCalibration_clicked();

    // This will take the parameters, write the calibration to flash
    // and exit the application
    void writeToFlashFromParameters ();
    // This will take the parameters, read the calib from the given
    // zwetschge and write it to disk.
    void extractCalibFromZwetschge ();

    void lstSelectionChanged();

private:
    void openCamera();
    void closeCamera();

    void clearFields (bool keepFlashData);

    void showStatusBox();
    void hideStatusBox();
    void updateStatusBox (const QString &message);

    void readSerial();

    void loadZwetschgeData (std::shared_ptr<royale::pal::IStorageReadRandom> storageAccess);
    void loadProtectedArea (std::shared_ptr<royale::pal::IStorageReadRandom> storageAccess);

    void writeZwetschge();
    void readZwetschge(QString filename);

private:

    std::shared_ptr<royale::ICameraDevice> m_cameraDevice;

    QMessageBox m_statusBox;
    QString m_zwetschgeFilename;
    std::string m_serialNumber;
    std::string m_moduleSerial;

    std::shared_ptr<spiFlashTool::storage::SPIStorageHardwareSpi> m_storage;

    ZwetschgeFlashToolParameters m_params;

    bool m_clVersion;

    std::vector<royale::imager::IImagerExternalConfig::UseCaseData> m_useCaseList;
    royale::Vector<uint8_t> m_calibrationData;

    RegisterMapTable *m_regMapInit;
    RegisterMapTable *m_regMapFw1;
    RegisterMapTable *m_regMapFw2;
    RegisterMapTable *m_regMapFwStart;
    RegisterMapTable *m_regMapStart;
    RegisterMapTable *m_regMapStop;
};
