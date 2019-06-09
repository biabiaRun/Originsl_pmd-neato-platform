/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "CameraPositionView.hpp"
#include "DisplaySupport.hpp"
#include <QLabel>

CameraPositionView::CameraPositionView (QWidget *parent) : PMDView (parent)
{
    ui.setupUi (this);
    ui.frontViewButton->resize (1.0f, 1.0f);
    ui.sideViewButton->resize (1.0f, 1.0f);
    ui.birdViewButton->resize (1.0f, 1.0f);
    QObject::connect (ui.frontViewButton, SIGNAL (clicked()), this, SLOT (frontButtonClicked()));
    QObject::connect (ui.sideViewButton, SIGNAL (clicked()), this, SLOT (sideButtonClicked()));
    QObject::connect (ui.birdViewButton, SIGNAL (clicked()), this, SLOT (birdButtonClicked()));
    layout()->setSizeConstraint (QLayout::SetMaximumSize);
}

void CameraPositionView::frontButtonClicked()
{
    emit cameraPositionPreset (CameraPositionPreset_Front);
}

void CameraPositionView::sideButtonClicked()
{
    emit cameraPositionPreset (CameraPositionPreset_Side);
}

void CameraPositionView::birdButtonClicked()
{
    emit cameraPositionPreset (CameraPositionPreset_Bird);
}
