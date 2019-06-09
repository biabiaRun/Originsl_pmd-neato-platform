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
#include "ui_DataSelectorView.h"
#include "PMDView.hpp"

#define DataSelector_Distance  (0)
#define DataSelector_Amplitude (1)
#define DataSelector_Gray      (2)
#define DataSelector_GrayUni   (3)
#define DataSelector_Overlay   (4)

class PushButton;

class DataSelectorView : public PMDView
{
    Q_OBJECT

public:
    explicit DataSelectorView (QWidget *parent = 0);

    void displayUniformMode (bool visible);

protected slots:
    void distanceButtonAction();
    void amplitudeButtonAction();
    void grayButtonAction();
    void uniButtonAction();
    void overlayButtonAction();
signals:
    void dataSelectorSwitched (int mode);
protected:
    Ui::DataSelectorView ui;
    PushButton *m_grayButton;
    PushButton *m_distButton;
    PushButton *m_uniButton;
    PushButton *m_overlayButton;
};
