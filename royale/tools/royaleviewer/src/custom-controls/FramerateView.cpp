/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "FramerateView.hpp"
#include "DisplaySupport.hpp"
#include <QPainter>

FramerateView::FramerateView (QWidget *parent) : PMDView (parent)
{
    ui.setupUi (this);
    ui.framerateSlider->setTracking (false);

    QObject::connect (ui.framerateSlider, SIGNAL (valueChanged (int)), ui.currentFramerate, SLOT (setNum (int)));
    QObject::connect (ui.framerateSlider, SIGNAL (valueChanged (int)), this, SIGNAL (framerateValueChanged (int)));
}

void FramerateView::minMaxChanged (int minValue, int maxValue)
{
    ui.framerateSlider->blockSignals (true);
    ui.framerateSlider->setMinimum (minValue);
    ui.framerateSlider->setMaximum (maxValue);
    ui.framerateSlider->blockSignals (false);
}

void FramerateView::valueChanged (int val)
{
    ui.framerateSlider->blockSignals (true);
    ui.framerateSlider->setValue (val);
    ui.framerateSlider->blockSignals (false);
    ui.currentFramerate->setNum (val);
}
