/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "ColorRangeView.hpp"
#include "DisplaySupport.hpp"
#include <QPainter>

ColorRangeView::ColorRangeView (QWidget *parent) : PMDStreamView (parent)
{
    m_currentDistMinimumValue = 0.1f;
    m_currentDistMaximumValue = 4.5f;
    m_currentGrayMinimumValue = 0;
    m_currentGrayMaximumValue = 4095;
    ui.setupUi (this);
    ui.forceButton->resize (1.0f, 0.7f);
    layout()->setSizeConstraint (QLayout::SetMaximumSize);

    updateDistLabels();
    updateGrayLabels();
    QObject::connect (ui.distMinSlider, SIGNAL (valueChanged (int)), this, SLOT (distMinSliderValueChanged (int)));
    QObject::connect (ui.distMaxSlider, SIGNAL (valueChanged (int)), this, SLOT (distMaxSliderValueChanged (int)));
    QObject::connect (ui.grayMinSlider, SIGNAL (valueChanged (int)), this, SLOT (grayMinSliderValueChanged (int)));
    QObject::connect (ui.grayMaxSlider, SIGNAL (valueChanged (int)), this, SLOT (grayMaxSliderValueChanged (int)));
    QObject::connect (ui.forceButton, SIGNAL (clicked()), this, SIGNAL (autoColorRangeUpdate()));

    QObject::connect (this, SIGNAL (setDistSliderMinText (QString)), ui.distMinLabel, SLOT (setText (QString)));
    QObject::connect (this, SIGNAL (setDistSliderMaxText (QString)), ui.distMaxLabel, SLOT (setText (QString)));
    QObject::connect (this, SIGNAL (setGraySliderMinText (QString)), ui.grayMinLabel, SLOT (setText (QString)));
    QObject::connect (this, SIGNAL (setGraySliderMaxText (QString)), ui.grayMaxLabel, SLOT (setText (QString)));

    setDistSliderMinMax ( (int) (m_currentDistMinimumValue * 100.0f), (int) (m_currentDistMaximumValue * 100.0f));
    setGraySliderMinMax (m_currentGrayMinimumValue, m_currentGrayMaximumValue);
}

void ColorRangeView::updateDistLabels()
{
    QString distMinText = "Distance Minimum Value (" + QString::number (m_currentDistMinimumValue, 'f', 2) + " m)";
    QString distMaxText = "Distance Maximum Value (" + QString::number (m_currentDistMaximumValue, 'f', 2) + " m)";

    emit setDistSliderMinText (distMinText);
    emit setDistSliderMaxText (distMaxText);
}

void ColorRangeView::updateGrayLabels()
{
    QString grayMinText = "Minimum Value (" + QString::number (m_currentGrayMinimumValue) + ")";
    QString grayMaxText = "Maximum Value (" + QString::number (m_currentGrayMaximumValue) + ")";

    emit setGraySliderMinText (grayMinText);
    emit setGraySliderMaxText (grayMaxText);
}

void ColorRangeView::distMinSliderValueChanged (int value)
{
    int maxDistMinimumValue = ui.distMaxSlider->maximum() - 1;
    if (value <= maxDistMinimumValue)
    {
        m_currentDistMinimumValue = (float) value / 100.0f;
        ui.distMinSlider->setValue ( (int) (m_currentDistMinimumValue * 100.0f));
        if (m_currentDistMinimumValue >= m_currentDistMaximumValue)
        {
            m_currentDistMaximumValue = m_currentDistMinimumValue + 0.01f;
            ui.distMaxSlider->setValue ( (int) (m_currentDistMaximumValue * 100.0f));
        }
        updateDistLabels();
        emit distColorRangeChanged (m_currentDistMinimumValue, m_currentDistMaximumValue);
    }
    else
    {
        distMinSliderValueChanged (maxDistMinimumValue);
    }
}

void ColorRangeView::distMaxSliderValueChanged (int value)
{
    int minDistMaximumValue = ui.distMinSlider->minimum() + 1;
    if (value >= minDistMaximumValue)
    {
        m_currentDistMaximumValue = (float) value / 100.0f;
        ui.distMaxSlider->setValue ( (int) (m_currentDistMaximumValue * 100.0f));
        if (m_currentDistMaximumValue <= m_currentDistMinimumValue)
        {
            m_currentDistMinimumValue = m_currentDistMaximumValue - 0.01f;
            ui.distMinSlider->setValue ( (int) (m_currentDistMinimumValue * 100.0f));
        }
        updateDistLabels();
        emit distColorRangeChanged (m_currentDistMinimumValue, m_currentDistMaximumValue);
    }
    else
    {
        distMaxSliderValueChanged (minDistMaximumValue);
    }
}

void ColorRangeView::grayMinSliderValueChanged (int value)
{
    int maxGrayMinimumValue = ui.grayMaxSlider->maximum() - 1;
    if (value <= maxGrayMinimumValue)
    {
        m_currentGrayMinimumValue = value;
        ui.grayMinSlider->setValue (m_currentGrayMinimumValue);
        if (m_currentGrayMinimumValue >= m_currentGrayMaximumValue)
        {
            m_currentGrayMaximumValue = m_currentGrayMinimumValue + 1;
            ui.grayMaxSlider->setValue (m_currentGrayMaximumValue);
        }
        updateGrayLabels();
        emit grayColorRangeChanged (m_currentGrayMinimumValue, m_currentGrayMaximumValue);
    }
    else
    {
        grayMinSliderValueChanged (maxGrayMinimumValue);
    }
}

void ColorRangeView::grayMaxSliderValueChanged (int value)
{
    int minGrayMaximumValue = ui.grayMinSlider->minimum() + 1;
    if (value >= minGrayMaximumValue)
    {
        m_currentGrayMaximumValue = value;
        ui.grayMaxSlider->setValue (m_currentGrayMaximumValue);
        if (m_currentGrayMaximumValue <= m_currentGrayMinimumValue)
        {
            m_currentGrayMinimumValue = m_currentGrayMaximumValue - 1;
            ui.grayMinSlider->setValue (m_currentGrayMinimumValue);
        }
        updateGrayLabels();
        emit grayColorRangeChanged (m_currentGrayMinimumValue, m_currentGrayMaximumValue);
    }
    else
    {
        grayMaxSliderValueChanged (minGrayMaximumValue);
    }
}

void ColorRangeView::setDistRange (float minValue, float maxValue)
{
    int tempMin = (int) (minValue * 100.0f);
    int tempMax = (int) (maxValue * 100.0f);
    if (tempMin > ui.distMinSlider->maximum() ||
            tempMax > ui.distMaxSlider->maximum ())
    {
        ui.distMinSlider->setMaximum (tempMax);
        ui.distMaxSlider->setMaximum (tempMax);
    }
    ui.distMinSlider->setValue (tempMin);
    ui.distMaxSlider->setValue (tempMax);
    m_currentDistMinimumValue = minValue;
    m_currentDistMaximumValue = maxValue;
    updateDistLabels();

    emit distColorRangeChanged (m_currentDistMinimumValue, m_currentDistMaximumValue);
}

void ColorRangeView::setGrayRange (uint16_t minValue, uint16_t maxValue)
{
    ui.grayMinSlider->setValue (minValue);
    ui.grayMaxSlider->setValue (maxValue);
    m_currentGrayMinimumValue = ui.grayMinSlider->value();
    m_currentGrayMaximumValue = ui.grayMaxSlider->value();
    updateGrayLabels();

    emit grayColorRangeChanged (m_currentGrayMinimumValue, m_currentGrayMaximumValue);
}

void ColorRangeView::setDistSliderMinMax (int minValue, int maxValue)
{
    ui.distMinSlider->setMinimum (minValue);
    ui.distMinSlider->setMaximum (maxValue);
    ui.distMaxSlider->setMinimum (minValue);
    ui.distMaxSlider->setMaximum (maxValue);
    ui.distMinSlider->setValue (minValue);
    ui.distMaxSlider->setValue (maxValue);
}

void ColorRangeView::setGraySliderMinMax (int minValue, int maxValue)
{
    ui.grayMinSlider->setMinimum (minValue);
    ui.grayMinSlider->setMaximum (maxValue);
    ui.grayMaxSlider->setMinimum (minValue);
    ui.grayMaxSlider->setMaximum (maxValue);
    ui.grayMinSlider->setValue (minValue);
    ui.grayMaxSlider->setValue (maxValue);
}

void ColorRangeView::showGroups (bool showDist, bool showGray)
{
    ui.distGroup->setVisible (showDist);
    ui.grayGroup->setVisible (showGray);
}

float ColorRangeView::getDistMin()
{
    return m_currentDistMinimumValue;
}

float ColorRangeView::getDistMax()
{
    return m_currentDistMaximumValue;
}

uint16_t ColorRangeView::getGrayMin()
{
    return static_cast<uint16_t> (m_currentGrayMinimumValue);
}

uint16_t ColorRangeView::getGrayMax()
{
    return static_cast<uint16_t> (m_currentGrayMaximumValue);
}
