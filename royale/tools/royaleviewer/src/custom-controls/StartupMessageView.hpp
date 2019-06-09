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
#include <QLabel>
#include <royale.hpp>
#include <QTimer>
#include <QPoint>
#include <vector>
#include "PMDView.hpp"
#include "ui_StartupMessageView.h"

class StartupMessageView : public PMDView
{
    Q_OBJECT

public:
    explicit StartupMessageView (const QString &message, QWidget *parent);
    ~StartupMessageView() override;

protected:
    void paintEvent (QPaintEvent *event) Q_DECL_OVERRIDE;

protected:
    Ui::StartupMessageView ui;

    const QString m_message;
};
