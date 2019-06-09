/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <QString>

class DisplaySupport
{
public:
    static DisplaySupport *sharedInstance();

    void setAssetsPrefix (const QString &prefix)
    {
        m_assetsPrefix = prefix;
    }

    const QString &assetsCategory()
    {
        return m_assetsCategory;
    }

    const QString &imagePostfix()
    {
        return m_imagePostfix;
    }

    float pixelRatio()
    {
        return m_pixelRatio;
    }

    const QString &assetsPrefix()
    {
        return m_assetsPrefix;
    }

    const QString asset (const QString &name)
    {
        return QString (m_assetsPrefix + m_assetsCategory + name + m_imagePostfix);
    }

    float pixelsToPoints (float pixels);
    float pointsToPixels (int points);

protected:
    DisplaySupport();

private:
    QString m_assetsCategory;
    QString m_imagePostfix;
    float   m_pixelRatio;
    QString m_assetsPrefix;
};
