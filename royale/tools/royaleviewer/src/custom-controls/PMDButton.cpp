/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "PMDButton.hpp"
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include "DisplaySupport.hpp"
#include <assert.h>

PMDButton::PMDButton (QWidget *parent) : QPushButton (parent), m_defaultImage (NULL), m_pressedImage (NULL), m_hideLabel (false)
{
    setStyleSheet ("background-color:transparent;color:white");
    setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-default"));
    setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-pressed"));
    setAutoRepeat (false);
}

PMDButton::~PMDButton()
{
}

QSize PMDButton::sizeHint() const
{
    if (m_defaultImage)
    {
        return QSize ( (m_defaultImage->size().width()),
                       (m_defaultImage->size().height()));
    }
    // This leads to a (visually) broken UI on purpose.
    return QSize (0, 0);
}


void PMDButton::paintEvent (QPaintEvent *ev)
{
    QPainter painter (this);

    std::shared_ptr<QImage> image = m_defaultImage;

    bool flag;
    if (isCheckable())
    {
        flag = isChecked();
    }
    else
    {
        flag = isDown();
    }

    if (flag)
    {
        image = m_pressedImage;
    }
    if (image)
    {
        QPixmap pix;
        pix = pix.fromImage (*image);
        painter.drawPixmap (0, 0, this->width(), this->height(), pix);
    }
    if (!m_hideLabel)
    {
        QPushButton::paintEvent (ev);
    }
}

void PMDButton::setDefaultImage (const QString &path)
{
    m_defaultImage.reset (new QImage (path));
}

void PMDButton::setPressedImage (const QString &path)
{
    m_pressedImage.reset (new QImage (path));
}

void PMDButton::resize (float widthRatio, float heightRatio)
{
    if (m_defaultImage)
    {
        if (widthRatio > 0.0f)
        {
            int width = static_cast<int> (161.f * widthRatio);
            int newWidth = static_cast<int> (DisplaySupport::sharedInstance()->pointsToPixels (width));
            setFixedWidth (newWidth);
        }
        if (heightRatio > 0.0f)
        {
            int height = static_cast<int> (45.f * heightRatio);
            int newHeight = static_cast<int> (DisplaySupport::sharedInstance()->pointsToPixels (height));
            setFixedHeight (newHeight);
        }
    }
}