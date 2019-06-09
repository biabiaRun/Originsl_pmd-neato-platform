/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag
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
#include "ui_FilterMinMaxView.h"
#include "PMDStreamView.hpp"

class FilterMinMaxView : public PMDStreamView
{
public:
    Q_OBJECT

public:
    explicit FilterMinMaxView (QWidget *parent = 0);

    void setFilterMinMaxSlider (int min, int max);
    void setVisibility (bool visibility);

    void valuesChanged (int min, int max);

public slots:
    void onFilterMinChanged (int min);
    void onFilterMaxChanged (int max);
    void onFilterMinMaxToggled (bool enabled);

signals:
    void filterMinChanged (float min);
    void filterMaxChanged (float max);

protected:
    Ui::FilterMinMaxView ui;

    float m_filterMin;
    float m_filterMax;
};
