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

#include <QWidget>
#include "ui_OpenFileView.h"
#include "PMDView.hpp"

#define DataSelector_Distance  (0)
#define DataSelector_Amplitude (1)
#define DataSelector_Gray      (2)

class OpenFileView : public PMDView
{
    Q_OBJECT

public:
    explicit OpenFileView (QWidget *parent = 0);

signals:
    void fileSelected (const std::string &);

protected slots:
    void okClicked();

protected:
    void showEvent (QShowEvent *event) override;

protected:
    Ui::OpenFileView ui;
};
