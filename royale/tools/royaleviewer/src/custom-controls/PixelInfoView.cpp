/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "PixelInfoView.hpp"
#include "DisplaySupport.hpp"
#include <QPainter>
#include <QDebug>
#include <QHBoxLayout>
#include <QToolTip>
#include <QMessageBox>

#include <math.h>


PixelInfoView::PixelInfoView (QWidget *parent)
    : PMDView (parent)
    , m_hideTimer (NULL)
{
    ui.setupUi (this);
    layout()->setSizeConstraint (QLayout::SetMaximumSize);
    ui.gridLayout->setColumnMinimumWidth (1, 80);
    m_hideTimer = new QTimer();
    m_hideTimer->setSingleShot (true);
    QObject::connect (m_hideTimer, SIGNAL (timeout()), this, SLOT (hide()));
    hide();
    m_iterations = 0;
    m_stdDevSqrSum = 0.0;
    m_stdDevSum = 0.0;
    this->setStyleSheet ("");
}

PixelInfoView::~PixelInfoView()
{
    delete m_hideTimer;
}

void PixelInfoView::paintEvent (QPaintEvent *)
{
    QPainter painter (this);
    painter.setRenderHint (QPainter::HighQualityAntialiasing);
    const QBrush translucentBlack = QBrush (QColor (0, 0, 0, 190));
    const qreal cornerRadius = 9;
    QPainterPath path;
    path.addRoundedRect (QRectF (0, 0, this->width(), this->height()), cornerRadius, cornerRadius);
    painter.fillPath (path, translucentBlack);
}

void PixelInfoView::showDepthPointInfo (const QPoint &windowPosition, const QPoint &imagePosition, const royale::DepthPoint &p)
{
    m_iterations = 0;
    m_stdDevSqrSum = 0.0;
    m_stdDevSum = 0.0;
    updateDepthPointInfo (windowPosition, imagePosition, p);
    m_hideTimer->stop();
    show();
}

void PixelInfoView::updateDepthPointInfo (const QPoint &windowPosition, const QPoint &imagePosition, const royale::DepthPoint &p)
{
    double stdDev = 0.0;

    if (p.z != 0.0)
    {
        m_stdDevSum += static_cast<double> (p.z);
        m_stdDevSqrSum += static_cast<double> (p.z * p.z);
        m_iterations++;

        if (m_iterations > 1)
        {
            stdDev = (1.0 / (static_cast<double> (m_iterations) - 1.0)) * (m_stdDevSqrSum - (1.0 / static_cast<double> (m_iterations)) * (m_stdDevSum * m_stdDevSum));
            if (stdDev > 0.0)
            {
                stdDev = sqrt (stdDev);
            }
            else
            {
                stdDev = 0.0;
            }
        }
    }

    QPoint shiftedPosition (windowPosition.x() + 20, windowPosition.y() - 40);
    ui.title->setText ("x,y = (" + QString::number (imagePosition.x()) + ", " + QString::number (imagePosition.y()) + ")");
    ui.depthLabel->setText (QString::number (p.z, 'f', 3) + " m");
    ui.stddevLabel->setText (QString::number (stdDev, 'f', 3) + " m");
    ui.noiseLabel->setText (QString::number (p.noise, 'f', 3) + " m");
    ui.grayLabel->setText (QString::number (p.grayValue));
    ui.confidenceLabel->setText (QString::number (p.depthConfidence));
    move (shiftedPosition);
    adjustSize();
}

void PixelInfoView::hideDepthPointInfo()
{
    m_hideTimer->start (100);
}
