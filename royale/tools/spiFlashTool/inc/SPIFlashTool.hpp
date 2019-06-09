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

#include <SPIStorageBase.hpp>
#include <royale.hpp>
#include <ui_mainwindow.h>

#include <memory>

namespace spiFlashTool
{
    class ProgressListener;
}

struct SPIFlashToolParameters
{
    std::string calibrationFilename;
    std::string moduleSerialNumber;
    std::string moduleIdentifierHex;
    std::string moduleSuffix;
};

class SPIFlashTool :
    public QMainWindow,
    public Ui::SPIFlashToolWindow
{
    Q_OBJECT

public:
    explicit SPIFlashTool (const SPIFlashToolParameters &params);
    ~SPIFlashTool();

protected slots :
    void on_pbOpen_clicked();
    void on_pbReadFromFlash_clicked();
    void on_pbWriteToFlash_clicked();

    void on_leModuleIdentifier_editingFinished();
    void on_pbModuleIdentifier_clicked();

    // This will take the parameters, write the calibration to flash
    // and exit the application
    void writeToFlashFromParameters ();

private:
    void openCamera();
    void closeCamera();

    bool readFlash();
    bool writeFlash (const royale::Vector<uint8_t> &calibData);
    bool readFile (royale::Vector<uint8_t> &calibData);
    bool askHeaderOnly();

    void showStatusBox();
    void hideStatusBox();
    void updateStatusBox (const QString &message);
    void readSerial();

    const char *hex_to_bin_char (char c);

private:

    std::shared_ptr<royale::ICameraDevice> m_cameraDevice;
    royale::Vector<uint8_t> m_calibrationData;
    std::string m_serialNumber;

    // As soon as the module identifier is changed and the
    // editing is finished these strings will reflect the
    // hexadecimal and binary versions of the module identifier
    std::string m_moduleIdentifierHex;
    std::string m_moduleIdentifierBin;

    QMessageBox m_statusBox;

    std::shared_ptr<spiFlashTool::storage::SPIStorageBase> m_storage;
    spiFlashTool::storage::IProgressReportListener *m_progressListener;

    friend spiFlashTool::ProgressListener;

    SPIFlashToolParameters m_params;
};
