/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "3DView.hpp"
#include "ArcBall.hpp"
#include "Axis.hpp"
#include "Depth3d.hpp"
#if defined(ROYALE_S7_WORKAROUND) && defined(TARGET_PLATFORM_ANDROID)
#include <cstdio>
#include "Depth3dSamsungS7.hpp"
#endif
#include "Frustum.hpp"
#include "MutexTryLocker.hpp"
#include <QDebug>
#include <QMouseEvent>
#include <QMutexLocker>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QtMath>


using namespace royale;

struct ThreeDViewProperties
{
    float       zoomFactor;
    float       fovx;
    float       fovy;
    float       projRatio;
    float       pointScaleFactor;
    bool        showFrustum;
    float       downX;
    float       downY;
    ArcBall     *arcBall;
    Axis        *axis = NULL;
    Frustum     *frustum = NULL;
    Depth3d     *depth3d = NULL;
    ThreeDView::ViewType viewType;
    QMatrix4x4  finalModelViewMatrix;
    QMatrix4x4  modelViewMatrix;
    QMatrix4x4  mvpMatrix;
    QMatrix4x4  rotationMatrix;
    QMatrix4x4  projectionMatrix;
    QMatrix4x4  globalModelTransformation;

    explicit ThreeDViewProperties (ColorHelper *colorHelper)
    {
        zoomFactor = 1.0f;
        fovx = 1.0f; // this default value will be overwritten in updateLensParameters
        fovy = 1.0f; // this default value will be overwritten in updateLensParameters
        projRatio = 1.f;
        pointScaleFactor = 1.f;
        showFrustum = true;
        downX = -1.f;
        downY = -1.f;
        axis = new Axis;
        arcBall = new ArcBall (10.0f, 10.0f);
        frustum = new Frustum ();

#if defined (ROYALE_S7_WORKAROUND) && defined(TARGET_PLATFORM_ANDROID)
        bool samsungS7found = false;

        // Look for Samsung Galaxy S7 (Exynos) identifier "herolte" in Android's default.prop
        FILE *defaultprop = std::fopen ("/default.prop", "rb");
        if (defaultprop)
        {
            std::fseek (defaultprop, 0, SEEK_END);
            std::vector<char> buf (ftell (defaultprop) + 1);
            std::fseek (defaultprop, 0, SEEK_SET);
            std::fread (buf.data (), buf.size (), 1, defaultprop);
            std::fclose (defaultprop);

            buf[buf.size () - 1] = 0;

            std::string propStr = buf.data ();
            samsungS7found = std::string::npos != propStr.find ("herolte");
        }

        if (samsungS7found)
        {
            depth3d = new Depth3dSamsungS7 (colorHelper);
        }
        else
        {
            depth3d = new Depth3d (colorHelper);
        }
#else
        depth3d = new Depth3d (colorHelper);
#endif
        viewType = ThreeDView::ViewType::FRONT;
        finalModelViewMatrix.setToIdentity();
        modelViewMatrix.setToIdentity();
        mvpMatrix.setToIdentity();
        rotationMatrix.setToIdentity();
        projectionMatrix.setToIdentity();
        globalModelTransformation.setToIdentity();
    }

    ThreeDViewProperties (const ThreeDViewProperties &) = delete;
    ThreeDViewProperties (ThreeDViewProperties &&) = delete;
    ThreeDViewProperties &operator= (const ThreeDViewProperties &) = delete;
    ThreeDViewProperties &operator= (ThreeDViewProperties &&) = delete;

    ~ThreeDViewProperties()
    {
        delete axis;
        delete arcBall;
        delete frustum;
        delete depth3d;
    }
};

ThreeDView::ThreeDView (ColorHelper *colorHelper, QMutex *dataMutex, QWidget *parent, Qt::WindowFlags f)
    : IView (colorHelper, dataMutex, parent, f),
      m_isInit (false),
      m_allowRotation (false),
      m_p (new ThreeDViewProperties (colorHelper)),
      m_lockView (false),
      m_pauseRotation (false),
      m_isRotating (false),
      m_isFollowing (false)
{
    // setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect (this, SIGNAL (newData (const royale::DepthData *, const royale::IntermediateData *)),
                      this, SLOT (onNewData (const royale::DepthData *, const royale::IntermediateData *)), Qt::QueuedConnection);

    QTimer *timer = new QTimer (this);
    connect (timer, SIGNAL (timeout()), this, SLOT (onAutoRotation()));
    timer->start (10);
    m_rotate = 0.f;
    m_rCenter = 2.f;
    m_rSpeed = 0.01f;
}

ThreeDView::~ThreeDView()
{
    if (m_isInit)
    {
        makeCurrent();
    }

    if (m_p->axis)
    {
        delete m_p->axis;
        m_p->axis = NULL;
    }

    if (m_p->frustum)
    {
        delete m_p->frustum;
        m_p->frustum = NULL;
    }

    if (m_p->depth3d)
    {
        delete m_p->depth3d;
        m_p->depth3d = NULL;
    }
    delete m_p;

    if (m_isInit)
    {
        doneCurrent();
    }
}

void ThreeDView::onNewData (const royale::DepthData *data, const royale::IntermediateData *intData)
{
    if (m_p->depth3d)
    {
        m_p->depth3d->setData (data, intData);
    }
    update();
}

QSize ThreeDView::minimumSizeHint() const
{
    return QSize (50, 50);
}

QSize ThreeDView::sizeHint() const
{
    return QSize (200, 200);
}

void ThreeDView::performProjection()
{
    m_p->projectionMatrix.setToIdentity();
    m_p->projectionMatrix.perspective (m_p->fovy, m_p->projRatio, 0.01f, 30.f);
}

void ThreeDView::calcPointScale (int &height)
{
    float tanhalffovy = (float) qTan (qDegreesToRadians (m_p->fovy) * 0.5f);
    float pointScaleFactor = (float) height * 0.5f / tanhalffovy;
    m_p->depth3d->setPointScaleFactor (pointScaleFactor);
}

void ThreeDView::translate (float x, float y, float z)
{
    QMatrix4x4 tmpMatrix;
    tmpMatrix.setToIdentity();
    QVector3D transVec = QVector3D (x, y, z);
    tmpMatrix.translate (transVec);
    m_p->modelViewMatrix = m_p->modelViewMatrix * tmpMatrix;
}

void ThreeDView::initializeGL()
{
    bool initOK = true;
    initializeOpenGLFunctions();

    setCameraPosition (m_p->viewType);
    glEnable (GL_DEPTH_TEST);
    glEnable (GL_CULL_FACE);
    if (!m_p->axis->initGL())
    {
        initOK = false;
    }

    glBindBuffer (GL_ARRAY_BUFFER, 0);
    glClearColor (0.0f, 0.0f, 0.0f, 1.0f);

    if (!m_p->frustum->initGLWithFov (m_p->fovx, m_p->fovy))
    {
        initOK = false;
    }

    if (initOK)
    {
        m_isInit = true;
    }

    setStyleSheet ("background-color:transparent");
}

const QImage &ThreeDView::currentImage()
{
    m_image = grabFramebuffer();
    return m_image;
}

void ThreeDView::paintGL ()
{
    // prevent app from crashing
    if (!m_isInit)
    {
        return;
    }
    glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_DEPTH_TEST);

    m_p->finalModelViewMatrix.setToIdentity();
    m_p->finalModelViewMatrix = m_p->modelViewMatrix * m_p->rotationMatrix;
    // currently not used anymore
    //m_p->mvpMatrix = m_p->projectionMatrix * m_p->finalModelViewMatrix;
    // draw the renderables...
    QMutexLocker locker (m_dataMutex);
    m_p->depth3d->render (m_p->finalModelViewMatrix, m_p->projectionMatrix, m_p->globalModelTransformation);
    if (m_p->showFrustum)
    {
        m_p->axis->render (m_p->finalModelViewMatrix, m_p->projectionMatrix, m_p->globalModelTransformation);
        m_p->frustum->render (m_p->finalModelViewMatrix, m_p->projectionMatrix, m_p->globalModelTransformation);
    }
    glDisable (GL_BLEND);
}

void ThreeDView::resizeGL (int width, int height)
{
    glViewport (0, 0, width, height);
    m_p->projRatio = (float) width / (float) height;
    // set projection matrix
    performProjection();
    m_p->arcBall->setBounds ( (float) width, (float) height);
    calcPointScale (height);
    update();
}

void ThreeDView::wheelEvent (QWheelEvent *event)
{
    // NOTE: got the magic numbers from the Qt doc "...Most mouse
    // types work in steps of 15 degrees, in which case the delta
    // value is a multiple of 120; i.e., 120 units * 1/8 = 15
    // degrees.", qt-4.8.5-vs10/doc/html/qwheelevent.html.
    const float smoothness = 0.1f;
    float numDegrees = (float) event->delta() / 8.0f;
    float numSteps = numDegrees / 15.0f;
    float zoomFactor = numSteps * smoothness;
    translate (0.0f, 0.0f, zoomFactor);

    if (m_lockView)
    {
        emit cameraPositionChanged (m_p->modelViewMatrix, m_p->rotationMatrix);
    }
    update();
}

void ThreeDView::mouseDoubleClickEvent (QMouseEvent *event)
{
    // auto rotation
    if (event->buttons() == Qt::LeftButton)
    {
        if (m_isFollowing)
        {
            m_isFollowing = false;
        }
        else
        {
            if (!m_isRotating)
            {
                startAutoRotation();
            }
            else
            {
                stopAutoRotation();
            }
        }
        emit autoRotationStatusChanged (m_isRotating, m_lockView, m_pauseRotation);
    }
}

void ThreeDView::mousePressEvent (QMouseEvent *event)
{
    pauseAutoRotation();
    // translation case
    if ( (event->buttons() == (Qt::LeftButton | Qt::RightButton)) || (event->buttons() == Qt::MiddleButton))
    {
        m_allowRotation = false;
        m_p->downX = static_cast<float> (event->x());
        m_p->downY = static_cast<float> (event->y());
    }
    // rotation case
    else if (event->buttons() == Qt::LeftButton)
    {
        m_p->arcBall->mouseDown (vec3f (static_cast<float> (event->x()), static_cast<float> (event->y()), 0.0f));
        m_allowRotation = true;
    }
}

void ThreeDView::mouseMoveEvent (QMouseEvent *event)
{
    if ( (event->buttons() == (Qt::LeftButton | Qt::RightButton)) || (event->buttons() == Qt::MiddleButton))
    {
        m_allowRotation = false;

        if (m_p->downX > -1)
        {
            float factor =  180.0f;
            float curDownX = static_cast<float> (event->x());
            float curDownY = static_cast<float> (event->y());
            translate ( (curDownX - m_p->downX) / factor, -1.f * (curDownY - m_p->downY) / factor, 0.f);
            m_p->downX = curDownX;
            m_p->downY = curDownY;
        }
    }
    else if (event->buttons() == Qt::LeftButton)
    {
        if (!m_allowRotation)
        {
            // reset here - seems to be bug in qt - no mousePressEvent recognized after touch event
            m_p->arcBall->mouseDown (vec3f (static_cast<float> (event->x()), static_cast<float> (event->y()), 0.0f));
            m_allowRotation = true;
        }
        m_p->arcBall->mouseMove (vec3f (static_cast<float> (event->x()), static_cast<float> (event->y()), 0.0f));
        m_p->rotationMatrix = m_p->arcBall->rotationMatrix();
    }

    if (m_lockView)
    {
        emit cameraPositionChanged (m_p->modelViewMatrix, m_p->rotationMatrix);
    }
    update();
}

void ThreeDView::mouseReleaseEvent (QMouseEvent * /* event */)
{
    m_p->downX = -1.f;
    m_p->downY = -1.f;

    emit clicked();
    if (m_pauseRotation)
    {
        m_pauseRotation = false;
        startAutoRotation();
        if (m_lockView)
        {
            emit autoRotationStatusChanged (m_isRotating, m_lockView, m_pauseRotation);
        }
    }
}

bool ThreeDView::event (QEvent *event)
{
    switch (event->type())
    {
        case QEvent::TouchBegin:
            {
                // nothing yet
            }
        case QEvent::TouchEnd:
            {
                // nothing yet
            }
        case QEvent::TouchUpdate:

            {
                QTouchEvent *touchEvent = static_cast<QTouchEvent *> (event);
                QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
                if (touchPoints.count() == 2)
                {
                    // determine scale factor
                    const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
                    const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();
                    qreal zoomFactor = QLineF (touchPoint0.pos(), touchPoint1.pos()).length() / QLineF (touchPoint0.lastPos(), touchPoint1.lastPos()).length();
                    qreal pxSpan = qAbs (QLineF (touchPoint0.pos(), touchPoint1.pos()).length() - QLineF (touchPoint0.lastPos(), touchPoint1.lastPos()).length());

                    qreal curCenterX = (touchPoint0.pos().x() + touchPoint1.pos().x()) / 2.f;
                    qreal curCenterY = (touchPoint0.pos().y() + touchPoint1.pos().y()) / 2.f;
                    qreal prevCenterX = (touchPoint0.lastPos().x() + touchPoint1.lastPos().x()) / 2.f;
                    qreal prevCenterY = (touchPoint0.lastPos().y() + touchPoint1.lastPos().y()) / 2.f;

                    bool applyZoom = false;
                    bool applyTranslation = false;

                    if (pxSpan > 4.f)
                    {
                        applyZoom = true;
                    }

                    // check for movement and ensure that during movement the touchpoints do not get too close/far
                    if ( (qAbs (curCenterX - prevCenterX) > 3.f || qAbs (curCenterY - prevCenterY) > 3.f) && qAbs (zoomFactor - 1.f) < 0.2f)
                    {
                        applyTranslation = true;
                    }

                    // zoom
                    if (applyZoom)
                    {
                        float sign = (zoomFactor < 1.f) ? -1.f : 1.f;
                        float sensibility = 0.08f;
                        translate (0.0f, 0.0f, sign * sensibility * static_cast<float> (zoomFactor));
                    }
                    // translation
                    else if (applyTranslation)
                    {
                        float factor =  180.0f;
                        translate (static_cast<float> (curCenterX - prevCenterX) / factor, -1.f * static_cast<float> (curCenterY - prevCenterY) / factor, 0.f);
                    }
                    m_allowRotation = false;
                }
                update();
                return true;
            }
        default:
            break;
    }
    return QWidget::event (event);
}

void ThreeDView::updateLensParameters (royale::LensParameters lensParam, uint16_t width, uint16_t height)
{
    m_lensParam = lensParam;

    m_p->fovx = qRadiansToDegrees (2.0f * atanf (static_cast<float> (width) / (2.0f * lensParam.focalLength.first)));
    m_p->fovy = qRadiansToDegrees (2.0f * atanf (static_cast<float> (height) / (2.0f * lensParam.focalLength.second)));

    m_p->depth3d->updateLensParameters (lensParam);
    m_p->frustum->updateFov (m_p->fovx, m_p->fovy);

    if (m_isInit)
    {
        resizeGL (this->width(), this->height());
    }
}

void ThreeDView::setCameraPosition (ViewType viewType)
{
    if (m_p)
    {
        m_p->viewType = viewType;

        m_p->rotationMatrix.setToIdentity();
        m_p->finalModelViewMatrix.setToIdentity();
        m_p->modelViewMatrix.setToIdentity();
        m_p->globalModelTransformation.setToIdentity();
        m_p->globalModelTransformation.translate (0.f, 0.f, 2.f);
        m_rCenter = 2.f;

        switch (m_p->viewType)
        {
            case ThreeDView::ViewType::FRONT:
                {
                    m_p->modelViewMatrix.lookAt (QVector3D (0.f, 0, 2.f), QVector3D (0, 0, 0), QVector3D (0.f, 1.0f, 0.f));
                }
                break;
            case ThreeDView::ViewType::BIRD:
                {
                    m_p->rotationMatrix.rotate (90.0f, QVector3D (1.0f, 0.0f, 0.0f));
                    m_p->modelViewMatrix.lookAt (QVector3D (0.f, 0.5, 7.5f), QVector3D (0, 0, 0.f), QVector3D (0.f, 1.0f, 0.f));
                }
                break;
            case ThreeDView::ViewType::SIDE:
                {
                    m_p->rotationMatrix.rotate (-90.0f, QVector3D (0.0f, 1.0f, 0.0f));
                    m_p->modelViewMatrix.lookAt (QVector3D (0.5f, 0.0f, 7.5f), QVector3D (0.5f, 0.0f, 0.f), QVector3D (0.f, 1.0f, 0.f));
                }
                break;
            default:
                break;
        }
        m_p->arcBall->reset (m_p->rotationMatrix);
        m_p->arcBall->setBounds ( (float) width(), (float) height());
    }
    update();
}

void ThreeDView::setCameraPosition (const QMatrix4x4 &modelViewMatrix, const QMatrix4x4 &rotationMatrix)
{
    m_p->modelViewMatrix = modelViewMatrix;
    m_p->rotationMatrix = rotationMatrix;
    m_p->arcBall->reset (rotationMatrix);
    update();
}

void ThreeDView::setAutoRotationStatus (bool isRotating)
{
    if (isRotating)
    {
        startAutoRotation();
    }
    else
    {
        if (m_isFollowing)
        {
            m_isFollowing = false;
        }
        else
        {
            stopAutoRotation();
        }
    }
    emit autoRotationStatusChanged (m_isRotating, m_lockView, m_pauseRotation);
}

void ThreeDView::setFollowRotationStatus (bool isFollowing)
{
    m_isRotating = false;
    m_isFollowing = isFollowing;
}

void ThreeDView::setRotatingSpeed (float speed, bool sync)
{
    m_rSpeed = speed;
    if (m_lockView && sync)
    {
        emit rotatingSpeedChanged (m_rSpeed);
    }
}

void ThreeDView::setRotatingCenter (float center)
{
    m_rCenter = center;
    m_p->globalModelTransformation.setToIdentity();
    m_p->globalModelTransformation.translate (0.f, 0.f, m_rCenter);
    update();
}

void ThreeDView::setFilterMinMax (float filterMin, float filterMax, bool cameraStart)
{
    m_p->depth3d->setFilterMinMax (filterMin, filterMax, cameraStart);
    update();
}

void ThreeDView::onLockViewEnabled (bool enabled)
{
    if (enabled)
    {
        emit cameraPositionChanged (m_p->modelViewMatrix, m_p->rotationMatrix);
        emit autoRotationStatusChanged (m_isRotating, enabled, m_pauseRotation);
        emit rotatingSpeedChanged (m_rSpeed);
    }
}

void ThreeDView::frustumDisplayToggled (bool enabled)
{
    if (m_p)
    {
        m_p->showFrustum = enabled;
    }
    update();
}

void ThreeDView::lockViewToggled (bool enabled)
{
    m_lockView = enabled;
    if (!m_lockView)
    {
        if (m_isFollowing)
        {
            m_isFollowing = false;
            emit autoRotationStatusChanged (m_isRotating, m_lockView, m_pauseRotation);
        }
    }
}

void ThreeDView::switchToDistanceBuffer()
{
    if (m_p->depth3d)
    {
        m_p->depth3d->switchToDistanceBuffer();
    }
    update();
}

void ThreeDView::switchToAmplitudeBuffer()
{
    if (m_p->depth3d)
    {
        m_p->depth3d->switchToAmplitudeBuffer();
    }
    update();
}

void ThreeDView::switchToGrayBuffer (bool uniform)
{
    if (m_p->depth3d)
    {
        m_p->depth3d->switchToGrayBuffer (uniform);
    }
    update();
}

void ThreeDView::switchToOverlay ()
{
    if (m_p->depth3d)
    {
        m_p->depth3d->switchToOverlay ();
    }
    update();
}

void ThreeDView::colorRangeChanged()
{
    m_p->depth3d->colorRangeChanged();
}

void ThreeDView::resetLensParameters()
{
    // Set to standard values from pico flexx
    // The focal length is the only value used from the parameters
    m_p->fovx = 60.0f;
    m_p->fovy = 45.0f;
    m_lensParam.focalLength.first = 210.0f;
    m_lensParam.focalLength.second = 210.0f;

    m_p->depth3d->updateLensParameters (m_lensParam);
    m_p->frustum->updateFov (m_p->fovx, m_p->fovy);

    resizeGL (this->width(), this->height());
}

void ThreeDView::onAutoRotation()
{
    if (m_isRotating)
    {
        m_p->arcBall->mouseMove (vec3f (static_cast<float> (m_rotate), static_cast<float> (height() / 2), 0.0f));
        m_p->rotationMatrix = m_p->arcBall->rotationMatrix();
        if (m_lockView)
        {
            emit cameraPositionChanged (m_p->modelViewMatrix, m_p->rotationMatrix);
        }

        m_rotate += m_rSpeed;
        if (m_rotate > m_rSpeed)
        {
            m_rotate = 0.f;
            m_p->arcBall->mouseDown (vec3f (static_cast<float> (m_rotate), static_cast<float> (height() / 2), 0.0f));
        }
        update();
    }
}

void ThreeDView::startAutoRotation()
{
    m_p->arcBall->mouseDown (vec3f (static_cast<float> (m_rotate), static_cast<float> (height() / 2), 0.0f));
    m_isRotating = true;
    m_allowRotation = true;
}

void ThreeDView::stopAutoRotation()
{
    m_isRotating = false;
    m_allowRotation = false;
}

void ThreeDView::pauseAutoRotation()
{
    if (m_isRotating)
    {
        stopAutoRotation();
        m_pauseRotation = true;
    }
    if (m_isFollowing)
    {
        emit autoRotationStatusChanged (m_isRotating, m_lockView, m_pauseRotation);
        m_pauseRotation = true;
        m_isFollowing = false;
    }
}
