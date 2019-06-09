/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "DataSelectorView.hpp"
#include "DisplaySupport.hpp"

DataSelectorView::DataSelectorView (QWidget *parent) : PMDView (parent)
{
    ui.setupUi (this);
    ui.distanceButton->resize (1.0f, 1.0f);
    ui.grayButton->resize (1.0f, 1.0f);
    ui.uniformButton->resize (1.0f, 1.0f);
    ui.overlayButton->resize (1.0f, 1.0f);
    layout()->setSizeConstraint (QLayout::SetMaximumSize);

    QObject::connect (ui.distanceButton, SIGNAL (clicked()), this, SLOT (distanceButtonAction()));
    QObject::connect (ui.grayButton, SIGNAL (clicked()), this, SLOT (grayButtonAction()));
    QObject::connect (ui.uniformButton, SIGNAL (clicked()), this, SLOT (uniButtonAction()));
    QObject::connect (ui.overlayButton, SIGNAL (clicked()), this, SLOT (overlayButtonAction()));
}

void DataSelectorView::distanceButtonAction()
{
    emit dataSelectorSwitched (DataSelector_Distance);
}

void DataSelectorView::amplitudeButtonAction()
{
    emit dataSelectorSwitched (DataSelector_Amplitude);
}

void DataSelectorView::grayButtonAction()
{
    emit dataSelectorSwitched (DataSelector_Gray);
}

void DataSelectorView::uniButtonAction()
{
    emit dataSelectorSwitched (DataSelector_GrayUni);
}

void DataSelectorView::overlayButtonAction()
{
    emit dataSelectorSwitched (DataSelector_Overlay);
}


void DataSelectorView::displayUniformMode (bool visible)
{
    // Switch between different display modes of this control.
    // If the 2D widget is active we should not display the uniform
    // option
    ui.uniformButton->setVisible (visible);
}
