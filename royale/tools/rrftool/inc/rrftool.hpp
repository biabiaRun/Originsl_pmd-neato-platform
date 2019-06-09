/****************************************************************************\
 * Copyright (C) 2016 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <QtWidgets>

#include <ui_mainwindow.h>

#include <RRFReader.h>
#include <RRFWriter.h>

class RRFTool :
    public QMainWindow,
    public Ui::RRFToolWindow
{
    Q_OBJECT

public:
    RRFTool();
    ~RRFTool();

protected slots :

    void on_actionOpenFile_triggered();
    void on_actionSaveAs_triggered();
    void on_actionExit_triggered();
    void on_actionCloseFile_triggered();

private :

    void clearInfoBox();
    void fillInfoBox();
    bool saveFrames (const royale_rrf_handle &outHandle);

private:

    royale_rrf_handle m_fileReadHandle;
};
