/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "DisplaySupport.hpp"
#include <QScreen>
#include <QApplication>
#include <QDebug>

#if defined(Q_OS_ANDROID)
#include <QAndroidJniObject>
#endif

#define ANDROID_HDPI ("android_hdpi")
#define ANDROID_XHDPI ("android_xhdpi")
#define ANDROID_XXHDPI ("android_xxhdpi")
#define IOS ("ios_2x")

static DisplaySupport *s_sharedInstance = NULL;

DisplaySupport *DisplaySupport::sharedInstance()
{
    if (!s_sharedInstance)
    {
        s_sharedInstance = new DisplaySupport();
    }
    return s_sharedInstance;
}

#if defined(Q_OS_IOS)
// Implementation of the constructor for iOS only
DisplaySupport::DisplaySupport() :
    m_imagePostfix ("@2x")
{
    m_pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    m_assetsCategory = IOS;

    //qDebug() << "assets category:" << m_assetsCategory;
    //qDebug() << "pixel ratio:" << m_pixelRatio;
}
#else
// Implementation of the constructor for Android, Linux and Windows
DisplaySupport::DisplaySupport() :
    m_imagePostfix ("")
{
    m_pixelRatio = 1.0f;

#if defined(Q_OS_ANDROID)
    QAndroidJniObject qtActivity = QAndroidJniObject::callStaticObjectMethod ("org/qtproject/qt5/android/QtNative",
                                   "activity",
                                   "()Landroid/app/Activity;");
    QAndroidJniObject resources = qtActivity.callObjectMethod ("getResources", "()Landroid/content/res/Resources;");
    QAndroidJniObject displayMetrics = resources.callObjectMethod ("getDisplayMetrics", "()Landroid/util/DisplayMetrics;");
    int density = displayMetrics.getField<int> ("densityDpi");
    m_pixelRatio = displayMetrics.getField<float> ("density");
#else
    int density = static_cast<int> (qApp->primaryScreen()->physicalDotsPerInch());
    m_pixelRatio = static_cast<float> (density) / 160.0f;
    if (m_pixelRatio > 1.5f)
    {
        m_pixelRatio = 1.5f;
    }
#endif
    if (density <= 160)
    {
        // Unsupported screen density (ldpi, mdpi). Fallback to HDPI.
        m_assetsCategory = ANDROID_HDPI;
    }
    else if (density <= 240)
    {
        m_assetsCategory = ANDROID_HDPI;
    }
    else if (density <= 320)
    {
        m_assetsCategory = ANDROID_XHDPI;
    }
    else if (density <= 480)
    {
        m_assetsCategory = ANDROID_XXHDPI;
    }
    else
    {
        // Unsupported screen density (xxxhdpi). Fallback to XXHDPI.
        m_assetsCategory = ANDROID_XXHDPI;
    }

    if (m_pixelRatio < 1.0f)
    {
        m_pixelRatio = 1.0f;
    }

    //qDebug() << "screen density:" << density;
    //qDebug() << "assets category:" << m_assetsCategory;
    //qDebug() << "pixel ratio:" << m_pixelRatio;
}
#endif

float DisplaySupport::pixelsToPoints (float pixels)
{
    return pixels / m_pixelRatio;
}

float DisplaySupport::pointsToPixels (int points)
{
    return static_cast<float> (points) * m_pixelRatio;
}
