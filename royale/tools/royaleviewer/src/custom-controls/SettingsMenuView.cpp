/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "SettingsMenuView.hpp"
#include <QApplication>
#include <QPainter>
#include <QPixmap>
#include <QDebug>
#include "DisplaySupport.hpp"
#include <QScroller>

namespace
{
    const char *active_btn_style =
        "color:    rgba(255, 255, 255, 255);"
        "border:   0px none rgba(0, 0, 0, 0);"
#ifndef TARGET_PLATFORM_ANDROID
        "border-bottom: 1px solid qlineargradient("
        "spread: pad, x1 : 0, y1 : 0, x2 : 1, y2 : 0,"
        "stop:0.1 rgba(255, 255, 255, 63),"
        "stop:0.5 rgba(255, 255, 255, 255),"
        "stop:0.9 rgba(255, 255, 255, 63)"
        ")"
#endif
        "";

    const char *settings_menu_style =
        "QPushButton"
        "{"
        "  margin:     0;"
        "  padding:    10;"
        "  text-align: left;"

        "  border:        0px none  rgba(0, 0, 0, 0);"
#ifndef TARGET_PLATFORM_ANDROID
        "  border-bottom: 1px solid rgba(0, 0, 0, 63);"
#endif
        "}"
#ifndef TARGET_PLATFORM_ANDROID
        "QPushButton:hover"
        "{"
        "  color: rgba(255, 255, 255, 255);"
        "  border: 0px none rgba(0, 0, 0, 0);"

        "border-bottom: 1px solid qlineargradient("
        "spread:pad, x1:0, y1:0, x2:1, y2:0,"
        "stop:0 rgba(255, 255, 255, 63),"
        "stop:0.5 rgba(255, 255, 255, 255),"
        "stop:1 rgba(255, 255, 255, 63)"
        ");"
        "}"
#endif
        "QCheckBox"
        "{"
        "  background: rgba(0, 0, 0, 0);"
        "}";
}

SettingsMenuView::SettingsMenuView (royale::CameraAccessLevel accessLevel, QWidget *parent)
    : PMDView (parent)
    , delegate (nullptr)
    , m_background (nullptr)
    , m_accessLevel (accessLevel)
{
    ui.setupUi (this);
    setStyleSheet (settings_menu_style);

    resetCheckboxes();

    QObject::connect (ui.btn_colorRange, SIGNAL (clicked()), this, SLOT (showColorRangeView()));
    QObject::connect (ui.btn_useCase, SIGNAL (clicked()), this, SLOT (showUseCaseView()));
    QObject::connect (ui.btn_exposureTime, SIGNAL (clicked()), this, SLOT (showExposureView()));
    QObject::connect (ui.btn_filterLevel, SIGNAL (clicked()), this, SLOT (showFiterLevelView()));
    QObject::connect (ui.btn_parameters, SIGNAL (clicked()), this, SLOT (showParametersView()));
    QObject::connect (ui.btn_cameraPresets, SIGNAL (clicked()), this, SLOT (showCameraPresetsView()));
    QObject::connect (ui.btn_autoRotation, SIGNAL (clicked()), this, SLOT (showAutoRotationView()));
    QObject::connect (ui.btn_filterMinMax, SIGNAL (clicked()), this, SLOT (showFilterMinMaxView()));
    QObject::connect (ui.btn_data, SIGNAL (clicked()), this, SLOT (showDataView()));
    QObject::connect (ui.btn_loadFile, SIGNAL (clicked()), this, SLOT (showLoadFileView()));
    QObject::connect (ui.btn_reset, SIGNAL (clicked()), parent, SLOT (resetGUI()));

    QObject::connect (ui.cb_showFrustrum, SIGNAL (stateChanged (int)), this, SLOT (updateShowFrustrum (int)));
    QObject::connect (ui.cb_lockView, SIGNAL (stateChanged (int)), this, SLOT (updateLockView (int)));
    QObject::connect (ui.cb_flipVertical, SIGNAL (stateChanged (int)), this, SLOT (updateFlipVertical (int)));
    QObject::connect (ui.cb_flipHorizontal, SIGNAL (stateChanged (int)), this, SLOT (updateFlipHorizontal (int)));
    QObject::connect (ui.cb_showFPS, SIGNAL (stateChanged (int)), this, SLOT (updateShowFPS (int)));
    QObject::connect (ui.cb_showStreamID, SIGNAL (stateChanged (int)), this, SLOT (updateShowStreamID (int)));
    QObject::connect (ui.cb_showNumberOfValidPixels, SIGNAL (stateChanged (int)), this, SLOT (updateShowNumberOfValidPixels (int)));
    QObject::connect (ui.cb_singleFrameRecording, SIGNAL (stateChanged (int)), this, SLOT (updateSingleFrameRecording (int)));
    QObject::connect (ui.cb_showRangeSpecifier, SIGNAL (stateChanged (int)), this, SLOT (updateShowRangeSpecifier (int)));

#ifdef ROYALE_PLAYBACK_ONLY
    updateToolEntries (false, true, false, true);
#else
    updateToolEntries (false, true, false, false);
#endif
}

void SettingsMenuView::deselect()
{
    ui.btn_colorRange->setStyleSheet ("");
    ui.btn_useCase->setStyleSheet ("");
    ui.btn_exposureTime->setStyleSheet ("");
    ui.btn_filterLevel->setStyleSheet ("");
    ui.btn_parameters->setStyleSheet ("");
    ui.btn_cameraPresets->setStyleSheet ("");
    ui.btn_autoRotation->setStyleSheet ("");
    ui.btn_filterMinMax->setStyleSheet ("");
    ui.btn_data->setStyleSheet ("");
    ui.btn_loadFile->setStyleSheet ("");
}

void SettingsMenuView::resetCheckboxes()
{
    ui.cb_showFrustrum->setChecked (true);
    ui.cb_lockView->setChecked (false);
    ui.cb_flipVertical->setChecked (false);
    ui.cb_flipHorizontal->setChecked (false);
    ui.cb_showFPS->setChecked (false);
    ui.cb_showStreamID->setChecked (false);
    ui.cb_showNumberOfValidPixels->setChecked (false);
    ui.cb_singleFrameRecording->setChecked (false);
    ui.cb_showRangeSpecifier->setChecked (false);
}

bool SettingsMenuView::singleFrameRecordingEnabled (bool playBack)
{
    if (playBack)
    {
        return true;
    }

    return ui.cb_singleFrameRecording->isChecked();
}

void SettingsMenuView::updateToolEntries (bool cameraAvailable, bool mode2D, bool modeMixed, bool playBack)
{
    ui.btn_colorRange->setVisible (true);

    ui.btn_useCase->setVisible (cameraAvailable);
    ui.btn_exposureTime->setVisible (cameraAvailable);
    ui.btn_filterLevel->setVisible (cameraAvailable);

    if (m_accessLevel >= royale::CameraAccessLevel::L2)
    {
        ui.btn_parameters->setVisible (true);
    }
    else
    {
        ui.btn_parameters->setVisible (false);
    }

    ui.btn_cameraPresets->setVisible (!mode2D);
    ui.btn_autoRotation->setVisible (!mode2D);

    ui.btn_filterMinMax->setVisible (true);
    ui.btn_data->setVisible (true);
    ui.btn_loadFile->setVisible (true);

    if (!mode2D)
    {
        ui.cb_showFrustrum->setVisible (true);
        ui.cb_lockView->setVisible (modeMixed);

    }
    else
    {
        ui.cb_showFrustrum->setVisible (false);
        ui.cb_lockView->setVisible (false);
    }

    ui.cb_flipVertical->setVisible (cameraAvailable);
    ui.cb_flipHorizontal->setVisible (cameraAvailable);

    ui.cb_showFPS->setVisible (true);
    ui.cb_showStreamID->setVisible (true);

    ui.cb_showNumberOfValidPixels->setVisible (m_accessLevel >= royale::CameraAccessLevel::L2);

    ui.cb_singleFrameRecording->setVisible (true);

    if (playBack)
    {
        ui.cb_singleFrameRecording->setVisible (false);
#ifdef TARGET_PLATFORM_ANDROID
        ui.cb_showRangeSpecifier->setVisible (false);
#else
        ui.cb_showRangeSpecifier->setVisible (true);
#endif
    }
    else
    {
        ui.cb_singleFrameRecording->setVisible (true);
        ui.cb_showRangeSpecifier->setVisible (false);
    }
}

void SettingsMenuView::showView (QPushButton *btn, const char *settingsView)
{
    deselect();
    delegate->settingSelected (settingsView);
    btn->setStyleSheet (active_btn_style);
}

void SettingsMenuView::showColorRangeView()
{
    showView (ui.btn_colorRange, "Color Range");
}

void SettingsMenuView::showUseCaseView()
{
    showView (ui.btn_useCase, "Use Case (FPS)");
}

void SettingsMenuView::showExposureView()
{
    showView (ui.btn_exposureTime, "Exposure Time");
}

void SettingsMenuView::showFiterLevelView()
{
    showView (ui.btn_filterLevel, "Filter level");
}

void SettingsMenuView::showParametersView()
{
    showView (ui.btn_parameters, "Parameters");
}

void SettingsMenuView::showCameraPresetsView()
{
    showView (ui.btn_cameraPresets, "Camera Preset");
}

void SettingsMenuView::showAutoRotationView()
{
    showView (ui.btn_autoRotation, "Auto Rotation");
}

void SettingsMenuView::showFilterMinMaxView()
{
    showView (ui.btn_filterMinMax, "Filter (Min/Max)");
}

void SettingsMenuView::showDataView()
{
    showView (ui.btn_data, "Data");
}

void SettingsMenuView::showLoadFileView()
{
    showView (ui.btn_loadFile, "Load File");
}

void SettingsMenuView::updateShowFrustrum (int state)
{
    emit enableFrustumDisplay (state == Qt::CheckState::Checked);
}

void SettingsMenuView::updateLockView (int state)
{
    emit enableLockView (state == Qt::CheckState::Checked);
}

void SettingsMenuView::updateFlipVertical (int state)
{
    emit enableFlipVertically (state == Qt::CheckState::Checked);
}

void SettingsMenuView::updateFlipHorizontal (int state)
{
    emit enableFlipHorizontally (state == Qt::CheckState::Checked);
}

void SettingsMenuView::updateShowFPS (int state)
{
    emit enableShowFPS (state == Qt::CheckState::Checked);
}

void SettingsMenuView::updateShowStreamID (int state)
{
    emit enableShowStreamId (state == Qt::CheckState::Checked);
}

void SettingsMenuView::updateShowNumberOfValidPixels (int state)
{
    emit enableShowValidPixelsNumber (state == Qt::CheckState::Checked);
}

void SettingsMenuView::updateSingleFrameRecording (int state)
{
    emit enableSingleFrameRecording (state == Qt::CheckState::Checked);
}

void SettingsMenuView::updateShowRangeSpecifier (int state)
{
    emit enableRangeSpecifier (state == Qt::CheckState::Checked);
}

void SettingsMenuView::disableRRFRecording (bool val)
{
    if (ui.cb_singleFrameRecording->isVisible())
    {
        return;
    }

    ui.cb_singleFrameRecording->setChecked (val);
    ui.cb_singleFrameRecording->setVisible (val);
}
