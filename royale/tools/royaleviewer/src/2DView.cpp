/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "2DView.hpp"
#include <QMutexLocker>
#include "MutexTryLocker.hpp"
#include <float.h>
#include "DisplaySupport.hpp"
#include "custom-controls/PixelInfoView.hpp"
#include <common/MakeUnique.hpp>

#ifdef ANDROID
#define glFrustum glFrustumf
#endif

using namespace royale;

TwoDView::TwoDView (ColorHelper *colorHelper, QMutex *dataMutex, QWidget *parent, Qt::WindowFlags f)
    : IView (colorHelper, dataMutex, parent, f)
{
    setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_backgroundImage = NULL;
    prepareBackgroundImage();
    m_showDistance = true;
    m_uniform = false;
    m_showGrayimage = false;
    m_showOverlayimage = false;
    m_currentDataset = nullptr;
    m_curRect = QRect (0, 0, this->size().width(), this->size().height());
    m_isWindowSizeChanged = false;
    m_pixelInfoManager = new PixelInfoManager();
    m_isHorizontallyFlipped = false;
    m_isVerticallyFlipped = false;

    m_filterMin = 0.0f;
    m_filterMax = 7.5f;

    QObject::connect (this, SIGNAL (newData (const royale::DepthData *, const royale::IntermediateData *)),
                      this, SLOT (onNewData (const royale::DepthData *, const royale::IntermediateData *)), Qt::QueuedConnection);
}

TwoDView::~TwoDView()
{
    delete m_pixelInfoManager;
    delete m_backgroundImage;
}

void TwoDView::fillImage()
{
    if (!m_currentDataset)
    {
        return;
    }

    m_imageBuffer.resize (m_currentDataset->height * m_currentDataset->width * 4);

    if (m_showDistance)
    {
        int idx = 0;
        for (int y = 0; y < m_currentDataset->height; y++)
        {
            for (int x = 0; x < m_currentDataset->width; x++)
            {
                const DepthPoint &currentPoint = m_currentDataset->points[idx];
                const IntermediatePoint &currentInterPoint = m_currentInterDataset->points[idx];

                const RgbColor &curColor = m_colorHelper->getColor (currentPoint.z);

                auto currentIndex = idx * 4;
                if (currentInterPoint.flags & FLAGS_SBI_ON)
                {
                    //note: qt's QImage::Format_ARGB32, i.e. 32-bit ARGB format (0xAARRGGBB).
                    m_imageBuffer[currentIndex++] = 120u;
                    m_imageBuffer[currentIndex++] = 120u;
                    m_imageBuffer[currentIndex++] = 120u;
                    m_imageBuffer[currentIndex] = 255u; // full opaque
                }
                else if (currentPoint.z == 0.f || currentPoint.z < m_filterMin || currentPoint.z > m_filterMax)
                {
                    //note: qt's QImage::Format_ARGB32, i.e. 32-bit ARGB format (0xAARRGGBB).
                    m_imageBuffer[currentIndex++] = 0u;
                    m_imageBuffer[currentIndex++] = 0u;
                    m_imageBuffer[currentIndex++] = 0u;
                    m_imageBuffer[currentIndex] = 255u; // full opaque
                }
                else
                {
                    //note: qt's QImage::Format_ARGB32, i.e. 32-bit ARGB format (0xAARRGGBB).
                    m_imageBuffer[currentIndex++] = static_cast<uint8_t> (curColor.b);
                    m_imageBuffer[currentIndex++] = static_cast<uint8_t> (curColor.g);
                    m_imageBuffer[currentIndex++] = static_cast<uint8_t> (curColor.r);
                    m_imageBuffer[currentIndex] = 255u; // full opaque
                }

                idx += 1;
            }
        }

    }
    else if (m_showGrayimage)
    {
        int idx = 0;
        for (int y = 0; y < m_currentDataset->height; y++)
        {
            for (int x = 0; x < m_currentDataset->width; x++)
            {
                const DepthPoint &currentPoint = m_currentDataset->points[idx];

                const RgbColor &curColor = m_colorHelper->getGrayColor (currentPoint.grayValue);
                auto currentIndex = idx * 4;

                if (currentPoint.z < m_filterMin || currentPoint.z > m_filterMax)
                {
                    //note: qt's QImage::Format_ARGB32, i.e. 32-bit ARGB format (0xAARRGGBB).
                    m_imageBuffer[currentIndex++] = 0u;
                    m_imageBuffer[currentIndex++] = 0u;
                    m_imageBuffer[currentIndex++] = 0u;
                    m_imageBuffer[currentIndex] = 255u; // full opaque
                }
                else
                {
                    //note: qt's QImage::Format_ARGB32, i.e. 32-bit ARGB format (0xAARRGGBB).s
                    m_imageBuffer[currentIndex++] = static_cast<uint8_t> (curColor.b);
                    m_imageBuffer[currentIndex++] = static_cast<uint8_t> (curColor.g);
                    m_imageBuffer[currentIndex++] = static_cast<uint8_t> (curColor.r);
                    m_imageBuffer[currentIndex] = 255; // full opaque
                }

                idx += 1;
            }
        }
    }
    else if (m_showOverlayimage)
    {

        int idx = 0;
        for (int y = 0; y < m_currentDataset->height; y++)
        {
            for (int x = 0; x < m_currentDataset->width; x++)
            {
                const DepthPoint &currentPoint = m_currentDataset->points[idx];

                const RgbColor &curColor = m_colorHelper->getColor (currentPoint.z);

                float grayVal = (float) m_colorHelper->getGrayColor (currentPoint.grayValue).r;
                float scaleTmp = 8.0f * std::pow (currentPoint.z, 1.5f) * grayVal / 255.f;
                float scale = qBound (0.f, scaleTmp, 1.f);

                auto currentIndex = idx * 4;

                if (currentPoint.z == 0.f || currentPoint.z < m_filterMin || currentPoint.z > m_filterMax)
                {
                    //note: qt's QImage::Format_ARGB32, i.e. 32-bit ARGB format (0xAARRGGBB).
                    m_imageBuffer[currentIndex++] = 0u;
                    m_imageBuffer[currentIndex++] = 0u;
                    m_imageBuffer[currentIndex++] = 0u;
                    m_imageBuffer[currentIndex] = 255u; // full opaque
                }
                else
                {
                    //note: qt's QImage::Format_ARGB32, i.e. 32-bit ARGB format (0xAARRGGBB).
                    m_imageBuffer[currentIndex++] = static_cast<uint8_t> (scale * curColor.b);
                    m_imageBuffer[currentIndex++] = static_cast<uint8_t> (scale * curColor.g);
                    m_imageBuffer[currentIndex++] = static_cast<uint8_t> (scale * curColor.r);
                    m_imageBuffer[currentIndex] = 255u; // full opaque
                }

                idx += 1;
            }
        }
    }

    m_image = QImage (&m_imageBuffer[0], m_currentDataset->width, m_currentDataset->height, QImage::Format_ARGB32).mirrored (m_isHorizontallyFlipped, m_isVerticallyFlipped);

    update();
}

void TwoDView::onNewData (const royale::DepthData *data, const royale::IntermediateData *intData)
{
    QMutexLocker locker (m_dataMutex);
    m_currentDataset = data;
    m_currentInterDataset = intData;

    fillImage();
}

const QImage &TwoDView::currentImage()
{
    return m_image;
}

void TwoDView::paintEvent (QPaintEvent *)
{
    QMutexLocker locker (m_dataMutex);
    QPainter p (this);
    p.drawImage (QPoint (0, 0), *m_backgroundImage);

    if (m_currentDataset)
    {
        if (m_isWindowSizeChanged)
        {
            adjustDrawingRectangle();
            m_isWindowSizeChanged = false;
        }
        p.drawImage (m_curRect, m_image);
    }

    if (m_pixelInfoManager->getPixelInfoSize() > 0)
    {
        m_pixelInfoManager->updateValues (m_currentDataset, m_image.width());
    }
}

void TwoDView::showDepthPointAtPosition (const QPoint &imagePosition, Qt::MouseButton pressButton)
{
    if (!m_currentDataset)
    {
        return;
    }

    QMutexLocker locker (m_dataMutex);

    // Convert from 2d canvas to image canvas
    int x = (int) ( (qreal) m_image.width() * (qreal) (imagePosition.x() - m_curRect.x()) / (qreal) m_curRect.width());
    int y = (int) ( (qreal) m_image.height() * (qreal) (imagePosition.y() - m_curRect.y()) / (qreal) m_curRect.height());

    if (m_pixelInfoManager->getPixelInfoSize() <= 5 || pressButton == Qt::RightButton)
    {
        if (x > -1 && x < m_image.width() && y > -1 && y < m_image.height() && pressButton == Qt::LeftButton && m_pixelInfoManager->getPixelInfoSize() < 5)
        {
            m_pixelInfoManager->addPixelInfoView (common::makeUnique<PixelInfoView> (this));

            if (!m_isHorizontallyFlipped && !m_isVerticallyFlipped)
            {
                m_pixelInfoSource = QPoint (x, y);
            }
            else if (!m_isHorizontallyFlipped && m_isVerticallyFlipped)
            {
                m_pixelInfoSource = QPoint (x, m_image.height() - y);
            }
            else if (m_isHorizontallyFlipped && !m_isVerticallyFlipped)
            {
                m_pixelInfoSource = QPoint (m_image.width() - x, y);
            }
            else if (m_isHorizontallyFlipped && m_isVerticallyFlipped)
            {
                m_pixelInfoSource = QPoint (m_image.width() - x, m_image.height() - y);
            }

            m_pixelInfoWindowPosition = imagePosition;

            int rightBorder = this->width() - m_rightBlank;
            if (imagePosition.x() >= (rightBorder - m_pixelInfoManager->getPixelInfoViewWidth()) - 20)
            {
                m_pixelInfoWindowPosition.setX (imagePosition.x() - m_pixelInfoManager->getPixelInfoViewWidth() + 60);
            }

            int offsetY = m_pixelInfoManager->getPixelInfoViewHeight() / 2;
            if (imagePosition.y() <= offsetY)
            {
                m_pixelInfoWindowPosition.setY (imagePosition.y() + offsetY);
            }

            if (imagePosition.y() >= this->size().height() - offsetY)
            {
                m_pixelInfoWindowPosition.setY (imagePosition.y() - offsetY);
            }

            DepthPoint p;
            if (!m_isHorizontallyFlipped && !m_isVerticallyFlipped)
            {
                p = m_currentDataset->points[y * m_image.width() + x];
            }
            else if (!m_isHorizontallyFlipped && m_isVerticallyFlipped)
            {
                p = m_currentDataset->points[ (m_currentDataset->height - y) * m_image.width() + x];
            }
            else if (m_isHorizontallyFlipped && !m_isVerticallyFlipped)
            {
                p = m_currentDataset->points[y * m_image.width() + (m_currentDataset->width - x)];
            }
            else if (m_isHorizontallyFlipped && m_isVerticallyFlipped)
            {
                p = m_currentDataset->points[ (m_currentDataset->height - y) * m_image.width() + (m_currentDataset->width - x)];
            }

            m_pixelInfoManager->addPositions (m_pixelInfoWindowPosition, m_pixelInfoSource, p);
        }

        else if (pressButton == Qt::RightButton && m_pixelInfoManager->getPixelInfoSize() >= 2 && ( (x > -1 && x < m_image.width() && y > -1 && y < m_image.height())))
        {
            m_pixelInfoManager->popLastAdded();
        }

        else if ( (pressButton == Qt::RightButton && m_pixelInfoManager->getPixelInfoSize() == 1) || (! (x > -1 && x < m_image.width() && y > -1 && y < m_image.height())) || m_pixelInfoManager->getPixelInfoSize() == 0)
        {
            m_pixelInfoManager->wipeOut();
            return;
        }

        m_pixelInfoManager->showValues();

    }
}

void TwoDView::mousePressEvent (QMouseEvent *event)
{
    emit closeLicense();

    showDepthPointAtPosition (event->pos(), event->button());
}


void TwoDView::mouseDoubleClickEvent (QMouseEvent *event)
{
}

void TwoDView::mouseMoveEvent (QMouseEvent *event)
{
}

void TwoDView::mouseReleaseEvent (QMouseEvent *event)
{
}

void TwoDView::switchToDistanceBuffer()
{
    QMutexLocker locker (m_dataMutex);
    m_showDistance = true;
    m_uniform = false;
    m_showGrayimage = false;
    m_showOverlayimage = false;
    fillImage();
}

void TwoDView::switchToAmplitudeBuffer()
{
    QMutexLocker locker (m_dataMutex);
    m_showDistance = false;
    fillImage();
}

void TwoDView::switchToGrayBuffer (bool uniform)
{
    QMutexLocker locker (m_dataMutex);
    m_showDistance = false;
    m_uniform = uniform;
    m_showGrayimage = true;
    m_showOverlayimage = false;
    fillImage();
}

void TwoDView::switchToOverlay()
{
    QMutexLocker locker (m_dataMutex);
    m_showDistance = false;
    m_uniform = false;
    m_showGrayimage = false;
    m_showOverlayimage = true;
    fillImage();
}

void TwoDView::colorRangeChanged()
{
    QMutexLocker locker (m_dataMutex);
    fillImage();
}

void TwoDView::prepareBackgroundImage ()
{
    QImage img (DisplaySupport::sharedInstance()->asset ("/background"));
    if (m_backgroundImage)
    {
        delete m_backgroundImage;
    }
    QImage scaledImage = img.scaled (this->size().width(), this->size().height(),
                                     Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_backgroundImage = new QImage (scaledImage);
}

void TwoDView::resizeEvent (QResizeEvent *event)
{
    m_isWindowSizeChanged = true;
    IView::resizeEvent (event);
}

void TwoDView::setImageAlignmentMode (ImageAlignmentMode mode, int unitBlank)
{
    switch (mode)
    {
        case SingleCentered:
            m_leftBlank = unitBlank;
            m_rightBlank = unitBlank;
            break;
        case LeftIndent:
            m_leftBlank = unitBlank;
            m_rightBlank = 0;
            break;
        case RightIndent:
            m_leftBlank = 0;
            m_rightBlank = unitBlank;
            break;
        case Centered:
            m_leftBlank = unitBlank / 2;
            m_rightBlank = unitBlank / 2;
            break;
        default:
            break;
    }

}

void TwoDView::adjustDrawingRectangle()
{
    int width = this->size().width() - m_leftBlank - m_rightBlank;
    int height = this->size().height();
    if ( (float) width / (float) height > (float) m_currentDataset->width / (float) m_currentDataset->height)
    {
        float scaleFactor = (float) height / (float) m_currentDataset->height;
        int scaledImgWidth = (int) ( (float) m_currentDataset->width * scaleFactor);
        int startX = (width - scaledImgWidth) / 2 + m_leftBlank;
        int startY = 0;
        m_curRect = QRect (startX, startY, scaledImgWidth, height);
    }
    else
    {
        float scaleFactor = (float) width / (float) m_currentDataset->width;
        int scaledImgHeight = (int) ( (float) m_currentDataset->height * scaleFactor);
        int startX = m_leftBlank;
        int startY = (height - scaledImgHeight) / 2;
        m_curRect = QRect (startX, startY, width, scaledImgHeight);
    }
}

void TwoDView::setFilterMinMax (float filterMin, float filterMax, bool cameraStart)
{
    QMutexLocker locker (m_dataMutex);
    m_filterMin = filterMin;
    m_filterMax = filterMax;
    if (!cameraStart)
    {
        fillImage();
    }
}

void TwoDView::flipHorizontally (bool enabled)
{
    m_pixelInfoManager->wipeOut();
    m_isHorizontallyFlipped = enabled;
}

void TwoDView::flipVertically (bool enabled)
{
    m_pixelInfoManager->wipeOut();
    m_isVerticallyFlipped = enabled;
}
