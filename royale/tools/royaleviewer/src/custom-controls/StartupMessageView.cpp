/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "StartupMessageView.hpp"
#include "DisplaySupport.hpp"
#include <QPainter>
#include <QStyle>
#include <QHBoxLayout>

StartupMessageView::StartupMessageView (const QString &message, QWidget *parent)
    : PMDView (parent),
      m_message (message)
{
    ui.setupUi (this);
    layout()->setSizeConstraint (QLayout::SetMaximumSize);
    ui.lblTitle->setText (m_message);

    setGeometry (QStyle::alignedRect (Qt::LeftToRight, Qt::AlignCenter, this->size(), parent->rect()));

    QApplication::setOverrideCursor (Qt::WaitCursor);

    show();
    setWindowModality (Qt::ApplicationModal);

    QApplication::processEvents();
}

StartupMessageView::~StartupMessageView()
{
    QApplication::restoreOverrideCursor();
}

void StartupMessageView::paintEvent (QPaintEvent *)
{
    QPainter painter (this);
    painter.setRenderHint (QPainter::HighQualityAntialiasing);
    const QBrush translucentBlack = QBrush (QColor (0, 0, 0, 190));
    const qreal cornerRadius = 9;
    QPainterPath path;
    path.addRoundedRect (QRectF (0, 0, this->width(), this->height()), cornerRadius, cornerRadius);
    painter.fillPath (path, translucentBlack);
}

