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
#include <stdint.h>
#include "ui_ColorRangeView.h"
#include "PMDStreamView.hpp"

class ColorRangeView : public PMDStreamView
{
    Q_OBJECT

public:
    explicit ColorRangeView (QWidget *parent = 0);
    void setDistRange (float minValue, float maxValue);
    void setGrayRange (uint16_t minValue, uint16_t maxValue);
    void setDistSliderMinMax (int minValue, int maxValue);
    void setGraySliderMinMax (int minValue, int maxValue);
    void updateDistLabels();
    void updateGrayLabels();

    void showGroups (bool showDist, bool showGray);

    float getDistMin();
    float getDistMax();
    uint16_t getGrayMin();
    uint16_t getGrayMax();

public slots:
    void distMinSliderValueChanged (int value);
    void distMaxSliderValueChanged (int value);
    void grayMinSliderValueChanged (int value);
    void grayMaxSliderValueChanged (int value);

signals:
    void distColorRangeChanged (float minValue, float maxValue);
    void grayColorRangeChanged (int minValue, int maxValue);
    void autoColorRangeUpdate();
    void setDistSliderMinText (QString text);
    void setDistSliderMaxText (QString text);
    void setGraySliderMinText (QString text);
    void setGraySliderMaxText (QString text);

    void forceButtonClicked();


protected:
    Ui::ColorRangeView ui;
    float   m_currentDistMinimumValue;
    float   m_currentDistMaximumValue;
    int   m_currentGrayMinimumValue;
    int   m_currentGrayMaximumValue;
};
