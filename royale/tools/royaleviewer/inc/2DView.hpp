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

#include "IView.hpp"
#include "ColorHelper.hpp"
#include <royale/DepthData.hpp>
#include "PixelInfoManager.hpp"

/**
* the alignment mode for the image in 2dView, incl. mixed mode
*/
enum ImageAlignmentMode
{
    SingleCentered,
    LeftIndent,
    RightIndent,
    Centered
};

class PixelInfoView;
class PixelInfoManager;

class TwoDView : public IView
{
    Q_OBJECT

public:
    TwoDView (ColorHelper *colorHelper, QMutex *dataMutex, QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~TwoDView();
    void switchToDistanceBuffer();
    void switchToAmplitudeBuffer(); // obsolete - to remove!
    void switchToGrayBuffer (bool uniform);
    void switchToOverlay();
    void colorRangeChanged() override;
    void prepareBackgroundImage ();
    const QImage &currentImage();

    /**
    *  Set the alignment mode of the image in 2dView
    *  Determine the sizes of the left and right blank region for the 2dView
    *
    *  @param mode The alignment mode of the image
    *  @param unitBlank The unit size of blank region
    */
    void setImageAlignmentMode (ImageAlignmentMode mode, int unitBlank);

    /**
    *  Set the the distance range
    *
    *  @param filterMin The minimum value of the distance range
    *  @param filterMax The maximum value of the distance range
    *  @param cameraStart The status of the camera, open or closed
    */
    void setFilterMinMax (float filterMin, float filterMax, bool cameraStart);

signals:
    void closeLicense();

public slots:
    void onNewData (const royale::DepthData *data, const royale::IntermediateData *intData) override;
    void flipVertically (bool enabled);
    void flipHorizontally (bool enabled);
protected:
    void paintEvent (QPaintEvent *) Q_DECL_OVERRIDE;
    void mousePressEvent (QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent (QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent (QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent (QMouseEvent *event) Q_DECL_OVERRIDE;
    void fillImage();
    void showDepthPointAtPosition (const QPoint &imagePosition, Qt::MouseButton pressButton);
    void resizeEvent (QResizeEvent *event) Q_DECL_OVERRIDE;
private:
    void setTooltipText (QMouseEvent *event);
    void adjustDrawingRectangle();
private:
    std::vector<uint8_t>      m_imageBuffer;
    QImage                    m_image;
    QString                   m_toolTipText;
    const royale::DepthData  *m_currentDataset;
    const royale::IntermediateData  *m_currentInterDataset;
    bool                      m_showDistance;
    bool                      m_uniform;
    bool                      m_showGrayimage;
    bool                      m_showOverlayimage;
    PixelInfoView            *m_pixelInfoView;
    PixelInfoManager         *m_pixelInfoManager;
    QImage                   *m_backgroundImage;
    QRect                     m_curRect;
    bool                      m_isWindowSizeChanged;
    QPoint                    m_pixelInfoSource;
    QPoint                    m_pixelInfoWindowPosition;
    bool                      m_isHorizontallyFlipped;
    bool                      m_isVerticallyFlipped;
    int                       m_rightBlank;
    int                       m_leftBlank;
    float                     m_filterMin;
    float                     m_filterMax;
};
