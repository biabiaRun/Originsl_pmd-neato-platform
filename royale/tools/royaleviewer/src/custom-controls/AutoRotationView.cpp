/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include "AutoRotationView.hpp"
#include "DisplaySupport.hpp"
#include <QPainter>

AutoRotationView::AutoRotationView (QWidget *parent) : PMDStreamView (parent)
{
    ui.setupUi (this);
    layout()->setSizeConstraint (QLayout::SetMaximumSize);

    ui.cbAutoRotation->setEnabled (true);
    QObject::connect (ui.cbAutoRotation, SIGNAL (toggled (bool)), this, SLOT (autoRotationChanged (bool)));
    QObject::connect (ui.cbAutoRotation, SIGNAL (toggled (bool)), this, SIGNAL (rotationModeChanged (bool)));

    m_center = 2.f;
    m_speed = 0.01f;

    ui.rCenterSlider->setValue (static_cast<int> (m_center * 100.f));
    ui.rCenterLabel->setText (QString ("Rotation Center (" + QString::number (m_center, 'f', 2) + " m)"));

    ui.rSpeedSlider->setValue (static_cast<int> (m_speed * 1000.f));
    ui.rSpeedLabel->setText (QString ("Rotation Speed (" + QString::number (static_cast<int> (m_speed * 1000.f)) + ")"));

    QObject::connect (ui.rCenterSlider, SIGNAL (valueChanged (int)), this, SLOT (onRCenterChanged (int)));
    QObject::connect (ui.rSpeedSlider, SIGNAL (valueChanged (int)), this, SLOT (onRSpeedChanged (int)));
}

void AutoRotationView::autoRotationChanged (bool enabled)
{
    ui.cbAutoRotation->blockSignals (true);
    ui.cbAutoRotation->setChecked (enabled);
    ui.cbAutoRotation->blockSignals (false);
}

void AutoRotationView::onRCenterChanged (int center)
{
    m_center = static_cast<float> (center) / 100.0f;
    ui.rCenterLabel->setText (QString ("Rotation Center (" + QString::number (m_center, 'f', 2) + " m)"));
    emit rCenterChanged (m_center);
}

void AutoRotationView::setRCenterSlider (int center)
{
    ui.rCenterSlider->blockSignals (true);
    ui.rCenterSlider->setValue (center);
    m_center = static_cast<float> (center) / 100.0f;
    ui.rCenterLabel->setText (QString ("Rotation Center (" + QString::number (m_center, 'f', 2) + " m)"));
    ui.rCenterSlider->blockSignals (false);
}

void AutoRotationView::onRSpeedChanged (int speed)
{
    m_speed = static_cast<float> (speed) / 1000.0f;
    ui.rSpeedLabel->setText (QString ("Rotation Speed (" + QString::number (speed) + ")"));
    emit rSpeedChanged (m_speed);
}

void AutoRotationView::setRSpeedSlider (int speed)
{
    ui.rSpeedSlider->blockSignals (true);
    ui.rSpeedSlider->setValue (speed);
    ui.rSpeedLabel->setText (QString ("Rotation Speed (" + QString::number (speed) + ")"));
    ui.rSpeedSlider->blockSignals (false);
}

void AutoRotationView::setVisibility (bool visibility)
{
    ui.autoRotationGroup->setVisible (visibility);
}
