/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <QWidget>
#include <stdint.h>
#include "ui_FilterLevelView.h"
#include "PMDStreamView.hpp"
#include <royale/FilterLevel.hpp>

class FilterLevelView : public PMDStreamView
{
    Q_OBJECT

public:
    explicit FilterLevelView (QWidget *parent = 0);

    void setFilterLevel (const royale::FilterLevel &level);

public slots:
    void filterLevelSliderChanged (int val);
    void enableToggled (bool val);

signals:

    void filterLevelChanged (const royale::FilterLevel &level,
                             royale::StreamId streamId);

protected:
    Ui::FilterLevelView ui;
};
