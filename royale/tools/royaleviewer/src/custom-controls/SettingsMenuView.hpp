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
    explicit SettingsMenuView (royale::CameraAccessLevel accessLevel, QWidget *parent = 0);
    void setDelegate (ISettingsMenu *obj)
    {
        delegate = obj;
    }

    void deselect();
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

protected slots:
    void settingSelected (QListWidgetItem *item);


protected:
    Ui::SettingsMenuView ui;
    ISettingsMenu *delegate;
    QPixmap *m_background;

private:
    QListWidgetItem *selectedItem;
    std::vector<std::string> m_entries;
    royale::CameraAccessLevel m_accessLevel;

    typedef void (SettingsMenuView::*SignalFunction) (bool);
    struct CheckBoxStruct
    {
        CheckBoxStruct (SettingsMenuView::SignalFunction _func, bool _checkState = false, bool _visible = true) :
            signalFunction (_func),
            checkState (_checkState),
            visible (_visible)
        {}
        CheckBoxStruct() :
            checkState (false),
            visible (false)
        {}

        SettingsMenuView::SignalFunction signalFunction;
        bool checkState;
        bool visible;
    };

    std::map<std::string, struct CheckBoxStruct> m_checkboxes;
};
