/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "FilterLevelView.hpp"
#include "DisplaySupport.hpp"
#include <QPainter>

FilterLevelView::FilterLevelView (QWidget *parent) :
    PMDStreamView (parent)
{
    ui.setupUi (this);

    QObject::connect (ui.filterLevelSlider, SIGNAL (valueChanged (int)), this, SLOT (filterLevelSliderChanged (int)));
    QObject::connect (ui.cbEnableLevels, SIGNAL (toggled (bool)), this, SLOT (enableToggled (bool)));
}

void FilterLevelView::filterLevelSliderChanged (int val)
{
    royale::FilterLevel filterLevel;
    switch (val)
    {
        case 0:
            filterLevel = royale::FilterLevel::Off;
            break;
        case 1:
            filterLevel = royale::FilterLevel::Legacy;
            break;
        case 2:
            filterLevel = royale::FilterLevel::Full;
            break;
    }
    emit filterLevelChanged (filterLevel, getCurrentStreamId());
}

void FilterLevelView::setFilterLevel (const royale::FilterLevel &level)
{
    ui.cbEnableLevels->blockSignals (true);
    switch (level)
    {
        case royale::FilterLevel::Off:
            ui.filterLevelSlider->setEnabled (true);
            ui.cbEnableLevels->setChecked (true);
            ui.filterLevelSlider->setValue (0);
            break;
        case royale::FilterLevel::Legacy:
            ui.filterLevelSlider->setEnabled (true);
            ui.cbEnableLevels->setChecked (true);
            ui.filterLevelSlider->setValue (1);
            break;
        case royale::FilterLevel::Full:
            ui.filterLevelSlider->setEnabled (true);
            ui.cbEnableLevels->setChecked (true);
            ui.filterLevelSlider->setValue (2);
            break;
        case royale::FilterLevel::Custom:
            ui.filterLevelSlider->setEnabled (false);
            ui.cbEnableLevels->setChecked (false);
            break;
        default:
            // This shouldn't happen!
            break;
    }
    ui.cbEnableLevels->blockSignals (false);
}

void FilterLevelView::enableToggled (bool val)
{
    if (!val)
    {
        ui.filterLevelSlider->setValue (0);
    }
    filterLevelSliderChanged (ui.filterLevelSlider->value());
}
