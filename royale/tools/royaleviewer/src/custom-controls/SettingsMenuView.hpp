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
#include <royale.hpp>
#include "ui_SettingsMenuView.h"
#include "PMDView.hpp"

class ISettingsMenu
{
public:
    virtual void settingSelected (const QString &name) = 0;
};

class SettingsMenuView : public PMDView
{
    Q_OBJECT

public:
    explicit SettingsMenuView (royale::CameraAccessLevel accessLevel, QWidget *parent = nullptr);
    void setDelegate (ISettingsMenu *obj)
    {
        delegate = obj;
    }

    void deselect();
    void resetCheckboxes();
    void updateToolEntries (bool cameraAvailable, bool mode2D, bool modeMixed, bool playBack);
    void disableRRFRecording (bool val);
    bool singleFrameRecordingEnabled (bool playBack);

signals:
    void enableFrustumDisplay (bool enabled);
    void enableLockView (bool enabled);
    void enableFlipVertically (bool enabled);
    void enableFlipHorizontally (bool enabled);
    void enableShowFPS (bool enabled);
    void enableShowStreamId (bool enabled);
    void enableShowValidPixelsNumber (bool enabled);
    void enableSingleFrameRecording (bool enabled);
    void enableRangeSpecifier (bool enabled);

protected slots:

    void showView (QPushButton *btn, const char *settingsView);

    void showColorRangeView();
    void showUseCaseView();
    void showExposureView();
    void showFiterLevelView();
    void showParametersView();
    void showCameraPresetsView();
    void showAutoRotationView();
    void showFilterMinMaxView();
    void showDataView();
    void showLoadFileView();

    void updateShowFrustrum (int);
    void updateLockView (int);
    void updateFlipVertical (int);
    void updateFlipHorizontal (int);
    void updateShowFPS (int);
    void updateShowStreamID (int);
    void updateShowNumberOfValidPixels (int);
    void updateSingleFrameRecording (int);
    void updateShowRangeSpecifier (int);

protected:
    Ui::SettingsMenuView ui;
    ISettingsMenu *delegate;

    QPixmap *m_background;

private:
    royale::CameraAccessLevel m_accessLevel;
};
