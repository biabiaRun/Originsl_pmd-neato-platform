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
#include "ui_CameraPositionView.h"
#include "PMDView.hpp"

#define CameraPositionPreset_Front (0)
#define CameraPositionPreset_Side (1)
#define CameraPositionPreset_Bird (2)

class CameraPositionView : public PMDView
{
    Q_OBJECT
public:
    explicit CameraPositionView (QWidget *parent = 0);

protected slots:
    void frontButtonClicked();
    void sideButtonClicked();
    void birdButtonClicked();

signals:
    void cameraPositionPreset (int preset);

protected:
    Ui::CameraPositionView ui;
};
