/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "ExposureView.hpp"
#include "DisplaySupport.hpp"
#include <QPainter>

ExposureView::ExposureView (QWidget *parent) : PMDStreamView (parent)
{
    ui.setupUi (this);
    ui.exposureSlider->setTracking (false);

    QObject::connect (ui.exposureSlider, SIGNAL (valueChanged (int)), ui.currentExposure, SLOT (setNum (int)));
    QObject::connect (ui.exposureSlider, SIGNAL (valueChanged (int)), this, SIGNAL (exposureValueChanged (int)));
    QObject::connect (ui.cbAutoExposure, SIGNAL (toggled (bool)), this, SIGNAL (exposureModeChanged (bool)));
}

void ExposureView::enableAutoExposure (bool enabled)
{
    ui.cbAutoExposure->setEnabled (enabled);
}

void ExposureView::minMaxChanged (int minValue, int maxValue)
{
    ui.exposureSlider->blockSignals (true);
    ui.exposureSlider->setMinimum (minValue);
    ui.exposureSlider->setMaximum (maxValue);
    ui.exposureSlider->blockSignals (false);
}

void ExposureView::valueChanged (int val)
{
    ui.exposureSlider->blockSignals (true);
    ui.exposureSlider->setValue (val);
    ui.exposureSlider->blockSignals (false);
    ui.currentExposure->setNum (val);
}

void ExposureView::blockSlider (bool val)
{
    ui.exposureSlider->setEnabled (!val);
    ui.exposureSlider->blockSignals (val);
}

void ExposureView::autoExposureChanged (bool enabled)
{
    ui.cbAutoExposure->blockSignals (true);
    ui.cbAutoExposure->setChecked (enabled);
    this->blockSlider (enabled);
    ui.cbAutoExposure->blockSignals (false);
}
