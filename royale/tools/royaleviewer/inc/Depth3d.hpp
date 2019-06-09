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

#include "Renderable.hpp"
#include "ColorHelper.hpp"

#include <royale/LensParameters.hpp>
#include <royale/DepthData.hpp>
#include <royale/IntermediateData.hpp>

QT_FORWARD_DECLARE_STRUCT (VertexData);

class Depth3d : public Renderable
{
public:
    Depth3d (ColorHelper *colorHelper);
    virtual ~Depth3d();

    virtual bool initGL() override;
    virtual void render (const QMatrix4x4 &mvMatrix, const QMatrix4x4 &projectionMatrix, const QMatrix4x4 &modelTransform) override;
    virtual void updateLensParameters (royale::LensParameters &lensParam);
    virtual void setData (const royale::DepthData *data, const royale::IntermediateData *intData);

    void switchToDistanceBuffer();
    void switchToAmplitudeBuffer(); // obsolete - to remove!
    void switchToGrayBuffer (bool uniform);
    void switchToOverlay();
    void setPointScaleFactor (const float &val);
    void setScreenResYHalf (const float &val);
    void colorRangeChanged();

    /**
    *  Set the the distance range
    *
    *  @param filterMin The minimum value of the distance range
    *  @param filterMax The maximum value of the distance range
    *  @param cameraStart The status of the camera, open or closed
    */
    void setFilterMinMax (float filterMin, float filterMax, bool cameraStart);

protected:
    virtual void initGeometry () override;


    const royale::DepthData *m_currentDataset;
    const royale::IntermediateData *m_currentInterDataset;
    royale::LensParameters m_lensParam;

    VertexData *m_vertices;
    GLuint *m_indices;
    bool m_isGLInit;
    bool m_isDataAvailable;
    bool m_isNewDataAvailable;
    bool m_forceColorRangeUpdate;
    bool m_showDistance;
    bool m_uniform;
    bool m_showGrayimage;
    bool m_showOverlayimage;
    bool m_modeSwitched;
    float m_pointScaleFactor;
    bool m_lensParamsChanged;
    //float m_pxSizeX;
    float m_focalLengthInPxSizeY;
    float m_screenResYHalf;
    ColorHelper *m_colorHelper;
    float m_filterMin;
    float m_filterMax;
};
