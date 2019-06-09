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

#include <memory>
#include <QPushButton>

class QImage;

class PMDButton : public QPushButton
{
public:
    explicit PMDButton (QWidget *parent = 0);
    ~PMDButton();

    void setDefaultImage (const QString &path);
    void setPressedImage (const QString &path);
    void hideLabel (bool hideLabel)
    {
        m_hideLabel = hideLabel;
    }

    /**
    *  Called to zoom the button, when the other controls and font size automatically zoom
    *  due to the changes of resolution.
    *  When the widthRatio and heightRatio are equal to 1,
    *  the scaling ratio of the button is equal to the scaling ratio of other controls.
    *  When the widthRatio or heightRatio is less than or equal to 0, the width or height does not zoom
    *
    *  @param widthRatio The scaling ratio of the button's width
    *  @param heightRatio The scaling ratio of the button's height
    */
    void resize (float widthRatio, float heightRatio);

protected:
    void paintEvent (QPaintEvent *) Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;

signals:

public slots:
protected:
    std::shared_ptr<QImage> m_defaultImage;
    std::shared_ptr<QImage> m_pressedImage;
    bool    m_hideLabel;
};
