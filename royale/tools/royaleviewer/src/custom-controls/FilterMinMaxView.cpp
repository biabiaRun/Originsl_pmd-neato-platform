/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include "FilterMinMaxView.hpp"
#include "DisplaySupport.hpp"
#include <QPainter>

FilterMinMaxView::FilterMinMaxView (QWidget *parent) : PMDStreamView (parent)
{
    ui.setupUi (this);
    layout()->setSizeConstraint (QLayout::SetMaximumSize);

    QObject::connect (ui.filterMinSlider, SIGNAL (valueChanged (int)), this, SLOT (onFilterMinChanged (int)));
    QObject::connect (ui.filterMaxSlider, SIGNAL (valueChanged (int)), this, SLOT (onFilterMaxChanged (int)));

    QObject::connect (ui.cbFilter, SIGNAL (toggled (bool)), this, SLOT (onFilterMinMaxToggled (bool)));

    m_filterMin = .0f;
    m_filterMax = 7.5f;

    setFilterMinMaxSlider ( (int) (m_filterMin * 100.0f), (int) (m_filterMax * 100.0f));
}

void FilterMinMaxView::onFilterMinChanged (int min)
{
    int maxFilterMin = ui.filterMaxSlider->maximum() - 1;
    if (min <= maxFilterMin)
    {
        m_filterMin = static_cast<float> (min) / 100.0f;
        ui.filterMinSlider->setValue ( (int) (m_filterMin * 100));
        ui.filterMinLabel->setText (QString ("Filter Minimum (" + QString::number (m_filterMin, 'f', 2) + " m)"));
        emit filterMinChanged (m_filterMin);
        if (m_filterMin >= m_filterMax)
        {
            onFilterMaxChanged (min + 1);
        }
    }
    else
    {
        onFilterMinChanged (maxFilterMin);
    }
}

void FilterMinMaxView::onFilterMaxChanged (int max)
{
    int minFilterMax = ui.filterMinSlider->minimum() + 1;
    if (max >= minFilterMax)
    {
        m_filterMax = static_cast<float> (max) / 100.0f;
        ui.filterMaxSlider->setValue ( (int) (m_filterMax * 100));
        ui.filterMaxLabel->setText (QString ("Filter Maximum (" + QString::number (m_filterMax, 'f', 2) + " m)"));
        emit filterMaxChanged (m_filterMax);
        if (m_filterMax <= m_filterMin)
        {
            onFilterMinChanged (max - 1);
        }
    }
    else
    {
        onFilterMaxChanged (minFilterMax);
    }
}

void FilterMinMaxView::valuesChanged (int min, int max)
{
    ui.filterMinSlider->blockSignals (true);
    ui.filterMaxSlider->blockSignals (true);
    ui.filterMinSlider->setValue (min);
    ui.filterMaxSlider->setValue (max);
    ui.filterMinLabel->setText (QString ("Filter Minimum (" +
                                         QString::number (static_cast<float> (min) / 100.0f, 'f', 2) + " m)"));
    ui.filterMaxLabel->setText (QString ("Filter Maximum (" +
                                         QString::number (static_cast<float> (max) / 100.0f, 'f', 2) + " m)"));
    ui.filterMinSlider->blockSignals (false);
    ui.filterMaxSlider->blockSignals (false);
}

void FilterMinMaxView::setFilterMinMaxSlider (int min, int max)
{
    ui.filterMinSlider->setMinimum (min);
    ui.filterMinSlider->setMaximum (max);

    ui.filterMaxSlider->setMinimum (min);
    ui.filterMaxSlider->setMaximum (max);

    ui.filterMinSlider->setValue (min);
    ui.filterMaxSlider->setValue (max);
}

void FilterMinMaxView::setVisibility (bool visibility)
{
    ui.filterGroup->setVisible (visibility);
}

void FilterMinMaxView::onFilterMinMaxToggled (bool enabled)
{
    if (enabled)
    {
        emit filterMinChanged (m_filterMin);
        emit filterMaxChanged (m_filterMax);
    }
    else
    {
        emit filterMinChanged (0.0f);
        emit filterMaxChanged (9999.9f);
    }
}