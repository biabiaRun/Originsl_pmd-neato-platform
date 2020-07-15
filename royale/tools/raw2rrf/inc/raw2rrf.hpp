/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <QtWidgets>
#include <vector>

#include <ui_mainwindow.h>

class Raw2RRF :
    public QMainWindow,
    public Ui::Raw2RRFWindow
{
    Q_OBJECT

public:
    Raw2RRF();
    ~Raw2RRF();

protected slots :

    void on_pbSelectRaw_clicked();
    void on_pbSelectCalib_clicked();
    void on_pbSelectOutput_clicked();
    void on_pbProcess_clicked();

private :

    void checkProcessButton();

};
