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

#include <QtWidgets>
#ifdef TARGET_PLATFORM_ANDROID
#include <jni.h>
#endif

#include <QMutex>

#include "SettingsMenuView.hpp"
#include "ColorHelper.hpp"
#include "ui_mainwindow.h"
#include <royale.hpp>
#include <royale/IDepthDataListener.hpp>
#include <royale/IExtendedDataListener.hpp>
#include <royale/IExposureListener2.hpp>
#include <royale/IEventListener.hpp>
#include <royale/IReplay.hpp>
#include <royale/Status.hpp>
#include <royale/Vector.hpp>

#include <string>
#include <fstream>
#include <streambuf>

#ifdef TARGET_PLATFORM_ANDROID
static void fromJNIResult (JNIEnv *env, jobject thiz, jint fd/*, jstring path*/);
#endif

class IView;
class TwoDView;
class ThreeDView;

struct QTViewerParameters
{
    royale::String accessCode;
    std::string playbackFilename;
    royale::String calibrationFileName;
    bool autoConnect;
    bool autoExposure;
    QString startUseCase;
    bool cameraSlave;
};

class QTViewer :
    public QMainWindow,
    public Ui::ViewerWindow,
    public royale::IExtendedDataListener,
    public ISettingsMenu,
    public royale::IExposureListener2,
    public royale::IEventListener
{
    Q_OBJECT

public:
    QTViewer (const QTViewerParameters &params);
    ~QTViewer();
    virtual void settingSelected (const QString &name) override;

    /**
     * Creates m_cameraDevice and returns a status.  The caller must handle errors, including
     * displaying and messages to the user and resetting m_cameraDevice to nullptr.
     */
#ifdef TARGET_PLATFORM_ANDROID
    royale::CameraStatus connectCamera (uint32_t androidUsbDeviceFD,
                                        uint32_t androidUsbDeviceVid,
                                        uint32_t androidUsbDevicePid);
#else
    royale::CameraStatus connectCamera();
#endif

    royale::CameraStatus startCapture();
    royale::CameraStatus stopCapture();

    void onNewData (const royale::IExtendedData *data) override;

    void onEvent (std::unique_ptr<royale::IEvent> &&event) override;

    void keyPressEvent (QKeyEvent *event) override;
    void keyReleaseEvent (QKeyEvent *event) override;

    void dragEnterEvent (QDragEnterEvent *event) override;
    void dropEvent (QDropEvent *event) override;

    /**
     * Creates m_cameraDevice (via connectCamera) and calls startCapture. On an error that should
     * disconnect, this will display an error to the user (possibly only in the message log) reset
     * the m_cameraDevice to nullptr, and return an error code.
     */
#ifdef TARGET_PLATFORM_ANDROID
    royale::CameraStatus initAndStart (uint32_t androidUsbDeviceFD,
                                       uint32_t androidUsbDeviceVid,
                                       uint32_t androidUsbDevicePid);
#else
    royale::CameraStatus initAndStart();
#endif

    void onNewExposure (const uint32_t exposureTime, const royale::StreamId streamId) override;

protected:
    void closeEvent (QCloseEvent *event) override;
    void resizeEvent (QResizeEvent *event) override;
    void layoutSubviews();

    bool eventFilter (QObject *obj, QEvent *event) override;

public slots:
    void cameraButtonClicked ();
    void forceColorButtonClicked();
    void viewButtonClicked();
    void recButtonClicked();
    void changeColorRanges (float minDist, float maxDist, uint16_t minGray, uint16_t maxGray, royale::StreamId streamId);
    void toolsMenuClicked();
    royale::CameraStatus useCaseSelected (const QString &name);
    void cameraPositionPreset (int preset);
    void dataSelectorSwitched (int mode);
    void enableSingleFrameRecording (bool enabled);
    void enableFPSDisplay (bool enabled);
    void enableStreamIdDisplay (bool enabled);
    void enableValidPixelsNumber (bool enabled);
    void hideTool();

    /**
    *  Determine the target 3dView which has sent the signal of cameraPositionChanged()
    *  and change the view in other windows the same as in the target one.
    *  This method will be called when any view has been changed
    *  and "Lock View" has been activated in Mode Mixed (3D).
    *
    *  @param modelViewMatrix The value of ModelViewMatrix from target 3dView
    *  @param rotationMatrix The value of RotationMatrix from target 3dView
    */
    void setCameraPositionChanged (const QMatrix4x4 &modelViewMatrix, const QMatrix4x4 &rotationMatrix);

    /**
    *  Set the status of auto rotation when it has changed and update the AutoRotationView
    *
    *  @param isRotating The current 3dView is/isn't rotating
    *  @param lockView "Lock View" has/hasn't been activated in Mode Mixed (3D)
    *  @param pause The auto rotation has/hasn't been paused
    */
    void setAutoRotationStatusChanged (bool isRotating, bool lockView, bool pause);

    /*
    *  Synchronize the rotating speed of the other 3dViews during "Lock View" and update the AutoRotationView
    *
    *  @param speed The new speed of auto rotation
    */
    void syncRotatingSpeedChanged (float speed);

    void onApplicationStateChanged (Qt::ApplicationState state);
    void updateFPS();
    void updateValidPixelsNumber();
    void exposureChanged (int newExposure);
    void framerateChanged (int newFramerate);
    void distColorRangeChanged (float minValue, float maxValue);
    void grayColorRangeChanged (int minValue, int maxValue);
    void autoColorRangeUpdate();
    void openPlaybackFile (const std::string &filename);
    void autoExposureEnabled (bool enabled);
    void autoRotationEnabled (bool enabled);
    void orientationChanged (Qt::ScreenOrientation);

    void onFilterMinChanged (const float min);
    void onFilterMaxChanged (const float max);

    void onRSpeedChanged (const float speed);
    void onRCenterChanged (const float center);

    void rewind();
    void playbackStopPlay (bool play);
    void forward();
    void seekTo (int frame);
    void repeat();

    void changePipelineParameter (royale::Pair<royale::ProcessingFlag, royale::Variant> parameter);

    void streamIdParameterViewChanged (royale::StreamId streamId);
    void streamIdExposureViewChanged (royale::StreamId streamId);
    void streamIdColorRangeViewChanged (royale::StreamId streamId);
    void streamIdFilterRangeViewChanged (royale::StreamId streamId);
    void streamIdFilterLevelViewChanged (royale::StreamId streamId);
    void streamIdAutoRotationViewChanged (royale::StreamId streamId);
    void setStreamIdChanged (royale::StreamId streamId);

    void newLogMessageAvailable();
    void refreshInfoMessage();
    void displayLog();
    void displayLicense();
    void closeLicense();
    void setHelpTabBarClicked (int index);

    void flipVertically (bool enabled);
    void flipHorizontally (bool enabled);

    void readRegister (QString registerStr);
    void writeRegister (QString registerStr, uint16_t value);

    void loadFile ();

    void filterLevelChanged (const royale::FilterLevel &level, royale::StreamId streamId);

signals:
    void exposureLimitsChanged (int min, int max);
    void framerateLimitsChanged (int min, int max);
    void framerateValue (int val);
    void logMessage (const QString &msg);
    void autoColorRangeFinished (float minDist, float maxDist, uint16_t minGray, uint16_t maxGray, royale::StreamId streamId);

    void registerReadReturn (QString registerStr, uint16_t result);
    void registerWriteReturn (bool result);

#ifdef TARGET_PLATFORM_ANDROID
public:
    // These variables are set by the Java part of the Android viewer
    // (File descriptor, VID, PID)
    static uint32_t androidUsbDeviceFD;
    static uint32_t androidUsbDeviceVid;
    static uint32_t androidUsbDevicePid;

    // Access code set by the Java part of the Android viewer.
    // This is taken from an optional extra intent.
    static std::string androidAccessCode;
#endif

private:

    struct rangeFilterSetting
    {
        float                                       filterMin = 0.0f;
        float                                       filterMax = 7.5f;
    };

    struct autoRotationSetting
    {
        bool                                        autoRotation = false;
        float                                       center = 2.f;
        float                                       speed = 0.01f;
    };

    // The initial settings which can be changed before the camera is turned on
    struct initSetting
    {
        bool                                        hasInit = false;
        bool                                        initAutoRotation = false;
        float                                       initCenter = 2.f;
        float                                       initSpeed = 0.01f;
        float                                       initFilterMin = 0.0f;
        float                                       initFilterMax = 7.5f;
        float                                       initMinDist = 0.1f;
        float                                       initMaxDist = 1.8f;
        uint16_t                                    initMinVal = 1;
        uint16_t                                    initMaxVal = 100;
    };

    void cleanUp();
    void initViews();
    void initAutoExposureModes (bool enabled);
    void findMinMax (const royale::DepthData *data, royale::StreamId streamId);

    void hideButtons (bool hide);
    void setCurrentView (bool twoD);

    void selectCurrentUseCaseInList();

    void fillStreamView();
    void updateStreamId();

    void openCameraOrRecording();
    void closeCameraOrRecording();
    void updateCameraButton();

    /*
    *  Initial Settings of Color Range, Filter (Min/Max) Auto Rotation (3D)
    *  If menu settings are changed before camera starts, the changed settings will be used directly after camera has started.
    */
    void initSettings();

    void countValidPixelsNumber (const royale::DepthData *data, royale::StreamId streamId);

    void stopRecording();

    void checkForNewAccessCode();

    QString getSavePath();

    std::unique_ptr<royale::ICameraDevice>          m_cameraDevice;
    std::string                                     m_serialNumber;
    royale::Vector<royale::String>                  m_useCases;
    QTimer                                          m_timer;
    bool                                            m_2d;
    bool                                            m_modeMixed;

    std::vector<QLabel *>                           m_fpsLabel;
    std::vector<QLabel *>                           m_streamIdLabel;
    std::vector<QLabel *>                           m_validPixelsNumberLabel;
    std::vector<TwoDView *>                         m_2dViews;
    std::vector<ThreeDView *>                       m_3dViews;
    royale::Vector<royale::StreamId>                m_streamIds;
    std::map<royale::StreamId, int>                 m_streamIdMap;

    bool                                            m_forceCapturingOnNextStateChange;
    bool                                            m_isConnected;
    std::map<royale::StreamId, royale::DepthData>   m_currentData;
    std::map<royale::StreamId, royale::IntermediateData> m_currentIntermediateData;
    QMutex                                         *m_dataMutex;
    QTimer                                          m_fpsTimer;
    QTimer                                          m_validPixelsNumberTimer;
    std::map<royale::StreamId, int>                 m_frameCounter;
    bool                                            m_showFPS;
    bool                                            m_showStreamId;
    bool                                            m_showValidPixelsNumber;
    bool                                            m_updateValidPixelsNumber;
    std::map<royale::StreamId, uint32_t>            m_validPixelsNumber;
    std::map<royale::StreamId, int>                 m_currentExposureTime;
    bool                                            m_isCurrentlyConnecting;
    std::vector<std::shared_ptr<ColorHelper>>       m_colorHelper;
    bool                                            m_showDistance;
    bool                                            m_showGrayimage;
    bool                                            m_showOverlayimage;
    bool                                            m_isRecording;
    std::map<royale::StreamId, bool>                m_firstData;
    std::map<royale::StreamId, bool>                m_hasData;
    PMDView                                        *m_toolsPanel;
    PMDView                                        *m_helpPanel;
    PMDView                                        *m_licensePanel;

    royale::IReplay                                *m_replayControl;
    royale::CameraAccessLevel                       m_accessLevel;
    royale::String                                  m_accessCode;
    std::string                                     m_playbackFilename;
    royale::String                                  m_calibrationFileName;
    bool                                            m_autoConnect;
    bool                                            m_autoExposure;
    bool                                            m_cameraSlave;
    QString                                         m_mode;
    royale::Vector<royale::ExposureMode>            m_exposureModes;
    bool                                            m_isPlaybackActive;
    bool                                            m_buttonsHidden;
    bool                                            m_logNotified;

    std::map<royale::StreamId, rangeFilterSetting>  m_rangeFilterSettings;
    std::map<royale::StreamId, autoRotationSetting> m_autoRotationSettings;

    static int const                                m_maxStreams;

    QSettings                                      *m_appSettings;
    royale::StreamId                                m_streamIdInMenu;
    initSetting                                     m_initSettings;

    bool                                            m_flipHorizontal;
    bool                                            m_flipVertical;

    QString                                         m_cameraIdStr;
    QString                                         m_cameraNameStr;
    QString                                         m_cameraInfoStr;

    float                                           m_currentTemperature;
    QString                                         m_appName;

    QString                                         m_lastLoadFolder;
    QString                                         m_saveFolder;
};
