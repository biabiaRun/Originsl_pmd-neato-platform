/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <QtWidgets>

#include <factory/BridgeController.hpp>
#include <factory/NonVolatileStorageFactory.hpp>
#include <royale/String.hpp>
#include <pal/IStorageReadRandom.hpp>

#include <memory>
#include <stdint.h>
#include <vector>

#include <ui_mainwindow.h>

class MonstarFirmwareUpdate :
    public QMainWindow,
    public Ui::UpdateFirmwareWindow
{
    Q_OBJECT

public:
    MonstarFirmwareUpdate();
    ~MonstarFirmwareUpdate();

protected slots :

    void on_pbUpdateFirmware_clicked();

private:

    // Reads a file from the Qt resources and returns a vector with the content
    std::vector<uint8_t> readFromResource (QString filename);

    // Creates an IStorageReadRandom object to read out the old firmware image.
    // This will later also be used to write the new image.
    std::shared_ptr<royale::pal::IStorageReadRandom> createStorage (royale::factory::IBridgeFactory &bridgeFactory);

    // maxx and monstar devices using the UVC firmware might not have a module
    // identifier yet. If we don't write a module identifier for these devices
    // they will only open as Level 3 devices afterwards (see firmware/arctic/bin/README.md)
    void fixFlashHeader (royale::factory::IBridgeFactory &bridgeFactory);

    void showStatusBox();
    void hideStatusBox();

    std::vector<uint8_t> m_firmwareImage;
    QMessageBox m_statusBox;
    royale::String m_cameraName;
};
