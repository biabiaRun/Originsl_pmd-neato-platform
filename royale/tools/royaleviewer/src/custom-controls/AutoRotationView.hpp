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
#include "ui_AutoRotationView.h"
#include "PMDStreamView.hpp"

class AutoRotationView : public PMDStreamView
{
public:
    Q_OBJECT

public:
    explicit AutoRotationView (QWidget *parent = 0);
    void setRCenterSlider (int center);
    void setRSpeedSlider (int speed);
    void setVisibility (bool visibility);

public slots:
    void autoRotationChanged (bool enabled);
    void onRCenterChanged (int center);
    void onRSpeedChanged (int speed);

signals:
    void rotationModeChanged (bool enabled);
    void rCenterChanged (float center);
    void rSpeedChanged (float speed);

protected:
    Ui::AutoRotationView ui;

    float m_center;
    float m_speed;
};
