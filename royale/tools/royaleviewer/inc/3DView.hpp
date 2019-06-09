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
#include <royale/LensParameters.hpp>

#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QMutex>

QT_FORWARD_DECLARE_CLASS (QOpenGLShaderProgram);
QT_FORWARD_DECLARE_CLASS (QOpenGLTexture);
QT_FORWARD_DECLARE_STRUCT (ThreeDViewProperties);


class ThreeDView : public IView
{
    Q_OBJECT

public:
    ThreeDView (ColorHelper *colorHelper, QMutex *dataMutex, QWidget *parent = 0, Qt::WindowFlags f = 0);
    enum class ViewType {FRONT, BIRD, SIDE};

    ~ThreeDView();

    virtual QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    virtual QSize sizeHint() const Q_DECL_OVERRIDE;
    virtual void updateLensParameters (royale::LensParameters lensParam, uint16_t width, uint16_t height);
    virtual void translate (float x, float y, float z);

    void resetLensParameters();

    void switchToDistanceBuffer();
    void switchToAmplitudeBuffer(); // obsolete - to remove!
    void switchToGrayBuffer (bool uniform);
    void switchToOverlay();
    void toggleFrustumDisplay();
    void colorRangeChanged() override;
    const QImage &currentImage();

    /**
    *  Update the current 3dView by the view type.
    *  This method is used to set new view of current 3dView.
    *
    *  @param viewType The option of view type
    */
    void setCameraPosition (ViewType viewType);

    /**
    *  Assign the values of matrices to the ModelViewMatrix and RotationMatrix of the current 3dView
    *  and update the view.
    *  This method is used to set new view of current 3dView.
    *
    *  @param modelViewMatrix The value of ModelViewMatrix from other 3dView
    *  @param rotationMatrix The value of RotationMatrix from other 3dView
    */
    void setCameraPosition (const QMatrix4x4 &modelViewMatrix, const QMatrix4x4 &rotationMatrix);

    /**
    *  Set the rotation status of 3dViews
    *
    *  @param isRotating The 3dView is/isn't rotating.
    */
    void setAutoRotationStatus (bool isRotating);

    /**
    *  This method is only used during "Lock View" to set the rotation status of secondary 3dViews.
    *
    *  @param isFollowing The 3dView is a secondary 3dViews of auto rotation and is/isn't rotating follow the main 3dView during "Lock View".
    */
    void setFollowRotationStatus (bool isFollowing);

    /*
    *  Set new center of auto rotation for the current 3dView
    *
    *  @param center The center of auto rotation in the current 3dView
    */
    void setRotatingCenter (float center);

    /*
    *  Set new speed of auto rotation and synchronize the rotating speed of the other 3dViews during "Lock View"
    *
    *  @param speed The speed of auto rotation
    *  @param sync Enable or disable the synchronization of speeds
    */
    void setRotatingSpeed (float speed, bool sync);

    /**
    *  Set the the distance range
    *
    *  @param filterMin The minimum value of the distance range
    *  @param filterMax The maximum value of the distance range
    *  @param cameraStart The status of the camera, open or closed
    */
    void setFilterMinMax (float filterMin, float filterMax, bool cameraStart);

signals:
    void clicked();

    /**
    *  This signal will be sent when current view has been changed
    *  and "Lock View" has been activated in Mode Mixed (3D).
    *
    *  @param modelViewMatrix The value of ModelViewMatrix in the current 3dView
    *  @param rotationMatrix The value of RotationMatrix in the current 3dView
    */
    void cameraPositionChanged (const QMatrix4x4 &modelViewMatrix, const QMatrix4x4 &rotationMatrix);

    /**
    *  This signal will be sent when the status of auto rotation has changed,
    *  e.g. to start/stop auto rotation, to pause auto rotation during mouse event, to change the main 3dView during "Lock View".
    *
    *  @param isRotating The current 3dView is/isn't rotating
    *  @param lockView "Lock View" has/hasn't been activated in Mode Mixed (3D)
    *  @param pause The auto rotation has/hasn't been paused
    */
    void autoRotationStatusChanged (bool isRotating, bool lockView, bool pause);

    /**
    *  This signal will be sent when the status of auto rotation has changed,
    *  e.g. to start/stop auto rotation, to pause auto rotation during mouse event, to change the main 3dView during "Lock View".
    *
    *  @param speed The speed of auto rotation
    */
    void rotatingSpeedChanged (float speed);

public slots:
    virtual void onNewData (const royale::DepthData *data, const royale::IntermediateData *intData) Q_DECL_OVERRIDE;
    void frustumDisplayToggled (bool enabled);

    /**
    *  Change the status of "Lock View" in the current 3dView
    *  This method will be called immediately after "Lock View" has been selected in Mode Mixed (3D).
    *
    *  @param enabled status of "Lock View"
    */
    void lockViewToggled (bool enabled);

    /**
    *  Emit the signal method cameraPositionChanged()
    *  This method can be called by immediately after "Lock View"
    *  to initialize the views in other windows with current view.
    */
    void onLockViewEnabled (bool enabled);

    /**
    *  This method is called every 10 milliseconds to achieve auto rotation
    */
    void onAutoRotation();

protected:
    virtual void initializeGL() Q_DECL_OVERRIDE;
    virtual void paintGL() Q_DECL_OVERRIDE;
    virtual void resizeGL (int width, int height) Q_DECL_OVERRIDE;
    virtual void mouseDoubleClickEvent (QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mousePressEvent (QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent (QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent (QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void wheelEvent (QWheelEvent *event) Q_DECL_OVERRIDE;
    virtual bool event (QEvent *event) Q_DECL_OVERRIDE;

    virtual void performProjection();
    virtual void calcPointScale (int &height);

    royale::LensParameters m_lensParam;
    bool m_isInit;
    bool m_allowRotation;

private:
    void startAutoRotation();
    void stopAutoRotation();

    /**
    *  Pause auto rotation during mouse event
    */
    void pauseAutoRotation();

    QImage                m_image;
    ThreeDViewProperties *m_p;

    std::vector<RgbColor> m_colors;
    GLfloat               m_rotate;
    bool                  m_lockView;
    bool                  m_pauseRotation;
    bool                  m_isRotating;
    bool                  m_isFollowing;
    float                 m_rCenter;
    float                 m_rSpeed;
};
