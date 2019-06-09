/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
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
#include <ui_mainwindow.h>
#include <FlashDataBase.hpp>

class FlashTool :
    public QMainWindow,
    public Ui::FlashToolWindow
{
    Q_OBJECT

public:
    FlashTool();
    ~FlashTool();

protected slots :
    void on_pbOpen_clicked();
    void on_pbSaveFromFlash_clicked();
    void on_pbSaveToFlash_clicked();

private:
    void openCamera();
    void closeCamera();
    void retrieveFlashInformation();

private:
    royale::String m_serialNumber;
    std::map<std::string, std::string> m_camInfoMap;
    std::unique_ptr<FlashDataBase> m_flashData;
};
