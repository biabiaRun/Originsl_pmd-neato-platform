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
#include "ui_PixelInfoView.h"

class PixelInfoView : public PMDView
{
    Q_OBJECT

public:
    explicit PixelInfoView (QWidget *parent = 0);
    ~PixelInfoView() override;
    void showDepthPointInfo (const QPoint &windowPosition, const QPoint &imagePosition, const royale::DepthPoint &p);
    void updateDepthPointInfo (const QPoint &windowPosition, const QPoint &imagePosition, const royale::DepthPoint &p);
    void hideDepthPointInfo();

protected:
    void paintEvent (QPaintEvent *event) Q_DECL_OVERRIDE;

protected:
    Ui::PixelInfoView ui;
    QTimer *m_hideTimer;
    QSize    m_sizeHint;
    uint32_t m_iterations;
    double   m_stdDevSqrSum;
    double   m_stdDevSum;
};
