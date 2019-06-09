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
#include "ui_ExposureView.h"
#include "PMDStreamView.hpp"

class PushButton;

class ExposureView : public PMDStreamView
{
    Q_OBJECT
public:
    explicit ExposureView (QWidget *parent = 0);
    void enableAutoExposure (bool enabled);
public slots:
    void minMaxChanged (int minValue, int maxValue);
    void valueChanged (int val);
    void blockSlider (bool val);
    void autoExposureChanged (bool enabled);
signals:
    void exposureValueChanged (int exposureVal);
    void exposureModeChanged (bool enabled);
protected:
    Ui::ExposureView ui;
};
