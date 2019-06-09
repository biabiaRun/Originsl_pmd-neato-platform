/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <QWidget>
#include "ui_FramerateView.h"
#include "PMDView.hpp"

class FramerateView : public PMDView
{
    Q_OBJECT
public:
    explicit FramerateView (QWidget *parent = 0);
public slots:
    void minMaxChanged (int minValue, int maxValue);
    void valueChanged (int val);
signals:
    void framerateValueChanged (int framerateVal);
protected:
    Ui::FramerateView ui;
};
