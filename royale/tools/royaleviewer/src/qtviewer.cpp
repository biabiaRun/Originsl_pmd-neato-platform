/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "qtviewer.hpp"
#include <iomanip>
#include <memory>
#include <regex>
#include <QFile>
#include <QMutexLocker>
#include <QTextStream>
#include <QtDebug>
#include <QThread>
#include "MutexTryLocker.hpp"
#include <royale/LensParameters.hpp>
#include <royale/IEvent.hpp>
#include "2DView.hpp"
#include "3DView.hpp"
#ifdef TARGET_PLATFORM_ANDROID
#include <QtAndroidExtras/QAndroidJniObject>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#endif
#include "PLYWriter.hpp"
#include <limits>
#include <FileHelper.hpp>
#include <RoyaleLogger.hpp>

#include <usecase/HardcodedMaxStreams.hpp>

#include <assert.h>

using namespace royale;

// --------------------------------------------------------------------------------
#include "DisplaySupport.hpp"

#include "SettingsMenuView.hpp"

#include "CameraPositionView.hpp"
#include "FilterLevelView.hpp"
#include "FilterMinMaxView.hpp"
#include "AutoRotationView.hpp"
#include "ColorRangeView.hpp"
#include "ParameterView.hpp"
#include "DataSelectorView.hpp"
#include "ExposureView.hpp"
#include "FramerateView.hpp"
#include "HelpView.hpp"
#include "LicenseView.hpp"
#include "ListView.hpp"
#include "OpenFileView.hpp"
#include "PixelInfoView.hpp"
#include "PlayerControl.hpp"
#include "RegisterView.hpp"
#include "StartupMessageView.hpp"

struct Widgets
{
    SettingsMenuView *toolsMenu;
    PMDView *currentTool;

    CameraPositionView *cameraPositionView;
    FilterLevelView *filterLevelView;
    FilterMinMaxView *filterMinMaxView;
    AutoRotationView *autoRotationView;
    ColorRangeView *colorRangeView;
    ParameterView *parameterView;
    DataSelectorView *dataSelectorView;
    ExposureView *exposureView;
    FramerateView *framerateView;
    HelpView *helpView;
    LicenseView *licenseView;
    ListView *usecaseView;
    OpenFileView *fileView;

    PlayerControl *playerControl;
};

namespace
{
    Widgets widgets;

    const QString qStrStopText = QString ("Stop");
    const QString qStrStartText = QString ("Start");

    const QString qStrSaveFolder = QString ("SaveFolder");
}

#ifdef TARGET_PLATFORM_ANDROID
uint32_t QTViewer::androidUsbDeviceFD = 0;
uint32_t QTViewer::androidUsbDeviceVid = 0;
uint32_t QTViewer::androidUsbDevicePid = 0;

std::string QTViewer::androidAccessCode = "";
#endif

// --------------------------------------------------------------------------------

int const QTViewer::m_maxStreams = ROYALE_USECASE_MAX_STREAMS;

QTViewer::QTViewer (const QTViewerParameters &params) :
    m_accessCode (params.accessCode),
    m_playbackFilename (params.playbackFilename),
    m_calibrationFileName (params.calibrationFileName),
    m_autoConnect (params.autoConnect),
    m_autoExposure (params.autoExposure),
    m_cameraSlave (params.cameraSlave),
    m_mode (params.startUseCase),
    m_flipHorizontal (false),
    m_flipVertical (false),
    m_currentTemperature (0.0f)
{
    m_2d                              = true;
    m_modeMixed                       = false;
    m_forceCapturingOnNextStateChange = false;
    m_isConnected                     = false;
    m_isCurrentlyConnecting           = false;
    m_showDistance                    = true;
    m_showGrayimage                   = false;
    m_showOverlayimage                = false;
    m_isRecording                     = false;
    m_replayControl                   = NULL;
    m_isPlaybackActive                = true;
    m_buttonsHidden                   = false;
    m_showFPS                         = false;
    m_showStreamId                    = false;
    m_showValidPixelsNumber           = false;
    m_updateValidPixelsNumber         = true;
    m_logNotified                     = false;
    m_dataMutex = new QMutex (QMutex::Recursive);
    m_currentExposureTime.clear();
    DisplaySupport::sharedInstance()->setAssetsPrefix (":/");

#ifdef TARGET_PLATFORM_ANDROID
    if (!androidAccessCode.empty())
    {
        m_accessCode = androidAccessCode.c_str();
    }
#endif

    m_accessLevel = royale::CameraManager::getAccessLevel (m_accessCode);

    m_appSettings  = new QSettings (QString::fromStdString (royaleviewer::getOutputPath() + "/viewersettings.ini"), QSettings::IniFormat);

    m_saveFolder = getSavePath();
    m_appSettings->setValue (qStrSaveFolder, m_saveFolder);

    QString qActivationCode{ m_appSettings->value ("ActivationCode").toString() };
    royale::String activationCode{ qActivationCode.toStdString() };

    royale::CameraAccessLevel iniAccessLevel{ royale::CameraManager::getAccessLevel (activationCode) };
    if (m_accessLevel < iniAccessLevel)
    {
        m_accessLevel = iniAccessLevel;
        m_accessCode = activationCode;
    }

    setupUi (this);
    initViews();
    initAutoExposureModes (m_autoExposure);
    m_appName = qApp->applicationName();
    unsigned int royaleMajor;
    unsigned int royaleMinor;
    unsigned int royalePatch;
    unsigned int royaleBuild;
    royale::String royaleScmRevision;
    royale::String royaleCustomer;

    royale::getVersion (royaleMajor, royaleMinor, royalePatch, royaleBuild, royaleCustomer, royaleScmRevision);

    QString royalVersion = QString::number (royaleMajor) + "." + QString::number (royaleMinor) + "." +
                           QString::number (royalePatch) + "." +
                           QString::number (royaleBuild) + ( (royaleCustomer.empty()) ? "" : ".") +
                           QString::fromStdString (royaleCustomer.toStdString()) + " (" +
                           QString::fromStdString (royaleScmRevision.toStdString()) + ")";
    m_appName += QString (" ") + royalVersion;

    m_lastLoadFolder = m_saveFolder;

#ifdef ROYALE_PLAYBACK_ONLY
    m_appName += " (Playback mode)";
    cameraButton->setVisible (false);
    cameraButton->setEnabled (false);
#endif

    setFocusPolicy (Qt::StrongFocus);

    widgets.helpView->blockSignals (true);
    widgets.helpView->addLogMessage ("Using Royale " + royalVersion);

    widgets.helpView->addInfoMessage ("Please click Refresh-Button to refresh camera information.");

#if defined(Q_OS_ANDROID)
    widgets.helpView->addHelpMessage ("The user guide is available in the installation package.");
#endif

    widgets.helpView->addAboutMessage ("Royale " + royalVersion);

    if (m_accessLevel >= CameraAccessLevel::L2)
    {
        m_appName += QString (" Level ") + QString::number ( (uint16_t) m_accessLevel);
        widgets.helpView->addLogMessage ("Level " + QString::number ( (uint16_t) m_accessLevel));
    }
    if (m_cameraSlave)
    {
        widgets.helpView->addLogMessage ("Using cameras in slave mode");
    }
    widgets.helpView->blockSignals (false);

    if (m_autoConnect)
    {
        cameraButton->click();
    }

    qApp->setApplicationName (m_appName);
    setWindowTitle (m_appName);
    QObject::connect (&m_fpsTimer, SIGNAL (timeout ()), this, SLOT (updateFPS ()));
    m_fpsTimer.start (1000);
    QObject::connect (&m_validPixelsNumberTimer, SIGNAL (timeout()), this, SLOT (updateValidPixelsNumber()));
    m_validPixelsNumberTimer.start (500);

    if (!m_playbackFilename.empty())
    {
        openPlaybackFile (m_playbackFilename);
    }

    this->setAcceptDrops (true);

    LOG (DEBUG) << "Viewer started";
}

QTViewer::~QTViewer()
{
    for (auto i = 0; i < m_maxStreams; ++i)
    {
        delete m_3dViews[i];
        delete m_2dViews[i];
        //delete m_fpsLabel[i];
    }

    delete widgets.filterLevelView;
    delete widgets.filterMinMaxView;
    delete widgets.autoRotationView;
    delete widgets.colorRangeView;

    delete widgets.parameterView;
    delete widgets.cameraPositionView;
    delete widgets.dataSelectorView;

    delete widgets.usecaseView;
    delete widgets.exposureView;
    delete widgets.framerateView;

    delete widgets.fileView;
    delete m_toolsPanel;

    delete m_helpPanel;
    delete m_licensePanel;
    delete widgets.playerControl;

    delete m_dataMutex;

    delete m_appSettings;

    delete DisplaySupport::sharedInstance();
}

#ifdef TARGET_PLATFORM_ANDROID

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_org_pmdtec_qtviewer_ProbingActivity_ProbingResult (JNIEnv *env,
        jobject obj,
        jint fd,
        jint vid,
        jint pid)
{
    QTViewer::androidUsbDeviceFD = fd;
    QTViewer::androidUsbDeviceVid = vid;
    QTViewer::androidUsbDevicePid = pid;
}

JNIEXPORT void JNICALL
Java_org_pmdtec_qtviewer_ProbingActivity_SetActivationCode (JNIEnv *env,
        jobject obj,
        jstring activationCode)
{
    const char *tempString = env->GetStringUTFChars (activationCode, 0);
    QTViewer::androidAccessCode = tempString;
    env->ReleaseStringUTFChars (activationCode, tempString);
}

#ifdef __cplusplus
}
#endif

#endif

void QTViewer::cameraButtonClicked ()
{
    closeLicense();
    if (!m_isConnected)
    {
        if (m_isCurrentlyConnecting)
        {
            cameraButton->setChecked (true);
            return;
        }
        m_isCurrentlyConnecting = true;

        openCameraOrRecording();
    }
    else if (cameraButton->text() == qStrStopText)
    {
        if (widgets.currentTool)
        {
            widgets.currentTool->hide();
        }
        widgets.toolsMenu->deselect();
        if (m_toolsPanel && m_toolsPanel->isVisible())
        {
            toolsButton->click();
        }
        if (m_helpPanel && m_helpPanel->isVisible())
        {
            helpButton->click();
        }
        closeCameraOrRecording();
    }
}

#ifdef TARGET_PLATFORM_ANDROID
CameraStatus QTViewer::initAndStart (uint32_t androidUsbDeviceFD,
                                     uint32_t androidUsbDeviceVid,
                                     uint32_t androidUsbDevicePid)
#else
CameraStatus QTViewer::initAndStart()
#endif
{
    CameraStatus ret;
    LensParameters lensParams;
#ifdef TARGET_PLATFORM_ANDROID
    ret = connectCamera (androidUsbDeviceFD, androidUsbDeviceVid, androidUsbDevicePid);
#else
    ret = connectCamera();
#endif

    if (ret != CameraStatus::SUCCESS)
    {
        return CameraStatus::COULD_NOT_OPEN;
    }

    widgets.helpView->addLogMessage ("Successfully connected to camera");

    if (m_cameraSlave)
    {
        ret = m_cameraDevice->setExternalTrigger (true);

        if (ret != CameraStatus::SUCCESS)
        {
            widgets.helpView->addLogMessage ("Unable to set camera as slave");
            m_cameraDevice.reset (nullptr);

            return ret;
        }
    }

    // Initialize the camera.
    ret = m_cameraDevice->initialize();

    if (ret != CameraStatus::SUCCESS)
    {
        widgets.helpView->addLogMessage ("Unable to initialize camera");

        switch (ret)
        {
            case CameraStatus::SUCCESS:
                // Should not be possible at this point
                return ret;
            case CameraStatus::CALIBRATION_DATA_ERROR:
                {
                    QMessageBox::warning (this, "No calibration data found", "No calibration found for this camera module " + QString::fromStdString (m_serialNumber));
                    widgets.helpView->addLogMessage ("No calibration found for this camera module " + QString::fromStdString (m_serialNumber) + "!");
                    break;
                }
            case CameraStatus::DATA_NOT_FOUND:
                if (!m_playbackFilename.empty())
                {
                    widgets.helpView->addLogMessage ("Unable to start playback! Data not found!");
                }
                widgets.helpView->addLogMessage ("Camera error : " + QString::number ( (int) ret));
                break;
            case CameraStatus::INSUFFICIENT_BANDWIDTH:
                widgets.helpView->addLogMessage ("Insufficient bandwidth,");
                widgets.helpView->addLogMessage ("please use a USB3 connection.");
                QMessageBox::warning (this, "Connection bandwidth not sufficient", "The camera module must be connected to a USB3 port");
                break;
            case CameraStatus::NO_USE_CASES_FOR_LEVEL:
                widgets.helpView->addLogMessage ("This camera doesn't offer use cases for the current access level!");
                widgets.helpView->addLogMessage ("Please make sure that the correct product code is set.");
                break;
            default:
                widgets.helpView->addLogMessage ("Camera error : " + QString::number ( (int) ret) + " " + QString (getErrorString (ret).c_str()));
                widgets.helpView->addLogMessage ("Try to restart application");
                widgets.helpView->addLogMessage ("and reconnect camera.");
                break;
        }

        ret = CameraStatus::DEVICE_NOT_INITIALIZED;

        m_cameraDevice.reset (nullptr);

        return ret;
    }

    m_cameraDevice->registerDataListenerExtended (static_cast<IExtendedDataListener *> (this));
    m_cameraDevice->setCallbackData (CallbackData::Intermediate);

    m_cameraDevice->registerExposureListener (this);
    ret = m_cameraDevice->getUseCases (m_useCases);

    if (ret != CameraStatus::SUCCESS)
    {
        widgets.helpView->addLogMessage ("Unable to retrieve use cases");
        return ret;
    }
    if (m_useCases.empty())
    {
        widgets.helpView->addLogMessage ("No use cases are available");
        return CameraStatus::RUNTIME_ERROR;
    }

    // If we set a use case on the command line, select this one.
    // Otherwise use the first one from the list.
    auto useCaseIdx = 0u;
    if (!m_mode.isEmpty())
    {
        bool useCaseFound = false;
        for (auto i = 0u; i < m_useCases.size(); ++i)
        {
            if (m_mode.compare (m_useCases[i].c_str()) == 0)
            {
                useCaseIdx = i;
                useCaseFound = true;
                break;
            }
        }

        if (!useCaseFound)
        {
            widgets.helpView->addLogMessage (QString ("The use case that was selected on the ") +
                                             QString ("command line is not available. Selecting the ") +
                                             QString ("first one that is available."));
        }
    }

    ret = m_cameraDevice->setUseCase (m_useCases.at (useCaseIdx));

    if (ret != CameraStatus::SUCCESS)
    {
        widgets.helpView->addLogMessage ("Unable to set use case");
        return ret;
    }

    widgets.usecaseView->clearItems();
    for (size_t i = 0; i < m_useCases.size(); ++i)
    {
        widgets.usecaseView->addItem (m_useCases.at (i).c_str());
    }

    // The lens parameters may not be available, in which case the 3DView will use its built-in
    // default settings. A non-SUCCESS here doesn't disconnect the camera.
    ret = m_cameraDevice->getLensParameters (lensParams);

    if (ret == CameraStatus::SUCCESS)
    {
        uint16_t height;
        ret = m_cameraDevice->getMaxSensorHeight (height);

        uint16_t width;
        auto ret2 = m_cameraDevice->getMaxSensorWidth (width);

        if (ret == CameraStatus::SUCCESS &&
                ret2 == CameraStatus::SUCCESS)
        {
            for (auto cur3DView : m_3dViews)
            {
                cur3DView->updateLensParameters (lensParams, width, height);
            }
        }
    }
    else
    {
        widgets.helpView->addLogMessage ("Unable to retrieve lens parameters, using standard ones");
        for (auto cur3DView : m_3dViews)
        {
            cur3DView->resetLensParameters();
        }
    }

    ret = startCapture();

    if (ret != CameraStatus::SUCCESS)
    {
        royale::String errMessage = royale::String ("startCapture failed : ") + getErrorString (ret);
        widgets.helpView->addLogMessage (errMessage.c_str());
    }

    return ret;
}

void QTViewer::selectCurrentUseCaseInList()
{
    royale::String currentUseCase;
    auto status = m_cameraDevice->getCurrentUseCase (currentUseCase);
    if (status == CameraStatus::SUCCESS)
    {
        widgets.usecaseView->setSelectedItem (QString::fromStdString (currentUseCase.toStdString()));
    }
}

CameraStatus QTViewer::useCaseSelected (const QString &name)
{
    royale::String selected = "";
    for (size_t i = 0; i < m_useCases.size(); ++i)
    {
        if (name.compare (m_useCases.at (i).c_str()) == 0)
        {
            selected = m_useCases.at (i);
            break;
        }
    }

    emit widgets.colorRangeView->forceButtonClicked();

    hideTool();

    royale::CameraStatus ret = CameraStatus::SUCCESS;
    if (selected != "")
    {
        ret = m_cameraDevice->setUseCase (selected);

        if (ret != CameraStatus::SUCCESS)
        {
            // Couldn't set use case
            widgets.helpView->addLogMessage (QString ("Unable to set use case '") + name +
                                             QString ("'! Reverting to previous use case!"));
            selectCurrentUseCaseInList();
            return ret;
        }

        fillStreamView();
    }
    else
    {
        return CameraStatus::DATA_NOT_FOUND;
    }
    return ret;
}

void QTViewer::forceColorButtonClicked()
{
    closeLicense();
    if (widgets.colorRangeView->isVisible())
    {
        for (auto i = 0u; i < m_streamIds.size(); ++i)
        {
            if (m_streamIds[i] != m_streamIdInMenu)
            {
                widgets.colorRangeView->setCurrentStreamId (m_streamIds[i]);
                autoColorRangeUpdate();
            }
        }
        if (m_streamIds.size())
        {
            widgets.colorRangeView->setCurrentStreamId (m_streamIdInMenu);
            autoColorRangeUpdate();
        }
    }
    else
    {
        for (auto i = 0u; i < m_streamIds.size(); ++i)
        {
            widgets.colorRangeView->setCurrentStreamId (m_streamIds[i]);
            autoColorRangeUpdate();
            if (!m_2d)
            {
                m_3dViews[i]->update();
            }
        }
    }
}

void QTViewer::viewButtonClicked()
{
    closeLicense();
    setCurrentView (!m_2d);
}

void QTViewer::initViews()
{
    int value = static_cast<int> (DisplaySupport::sharedInstance()->pointsToPixels (22));
    QString vSBstyle = "QScrollBar:vertical { border: none; background: rgba(0, 0, 0, 0.2); width: " + QString::number (value) + "px; margin: 1px 6px 1px 6px; }";
    QString hSBstyle = "QScrollBar:horizontal { border: none; background: rgba(0, 0, 0, 0.2); height: " + QString::number (value) + "px; margin: 6px 1px 6px 1px; }";
    const QString qmlPath = ":/style.qml";
    QFile file (qmlPath);
    if (file.open (QIODevice::ReadOnly))
    {
        QTextStream stream (&file);
        QString content = "";
        while (!stream.atEnd())
        {
            content += stream.readLine();
        }
        file.close();
        qApp->setStyleSheet (content + vSBstyle + hSBstyle);
    }

    QFont *font = new QFont();
    font->setPixelSize (static_cast<int> (DisplaySupport::sharedInstance()->pointsToPixels (14)));
    qApp->setFont (*font);
    QFont *startFont = new QFont();
    if (DisplaySupport::sharedInstance()->assetsCategory() == "android_hdpi")
    {
        startFont->setPixelSize (14);
    }
    else
    {
        startFont->setPixelSize (28);
    }
    cameraButton->setFont (*startFont);

    QObject::connect (this, SIGNAL (autoColorRangeFinished (float, float, uint16_t, uint16_t, royale::StreamId)), this, SLOT (changeColorRanges (float, float, uint16_t, uint16_t, royale::StreamId)));

    m_3dViews.resize (m_maxStreams);
    m_2dViews.resize (m_maxStreams);
    m_fpsLabel.resize (m_maxStreams);
    m_streamIdLabel.resize (m_maxStreams);
    m_validPixelsNumberLabel.resize (m_maxStreams);
    m_exposureModes.resize (m_maxStreams);

    for (auto i = 0; i < m_maxStreams; ++i)
    {
        m_colorHelper.push_back (std::shared_ptr<ColorHelper> (new ColorHelper()));
        m_3dViews[i] = new ThreeDView (m_colorHelper[i].get (), m_dataMutex, centralWidgetX);
        m_2dViews[i] = new TwoDView (m_colorHelper[i].get(), m_dataMutex, centralWidgetX, 0);

        m_3dViews[i]->installEventFilter (this);
        m_2dViews[i]->installEventFilter (this);

        m_fpsLabel[i] = new QLabel (m_2dViews[i]);
        m_fpsLabel[i]->setAlignment (Qt::AlignCenter);
        m_fpsLabel[i]->setStyleSheet ("QLabel { background-color: rgba(0, 0, 0, 0); color : white;font: 15pt; }");
        m_fpsLabel[i]->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        m_fpsLabel[i]->hide();
        m_fpsLabel[i]->setText ("XXXX fps");
        m_fpsLabel[i]->adjustSize();
        m_fpsLabel[i]->setText ("");

        m_streamIdLabel[i] = new QLabel (m_2dViews[i]);
        m_streamIdLabel[i]->setAlignment (Qt::AlignCenter);
        m_streamIdLabel[i]->setStyleSheet ("QLabel { background-color: rgba(0, 0, 0, 0); color : white;font: 15pt; }");
        m_streamIdLabel[i]->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        m_streamIdLabel[i]->hide();
        m_streamIdLabel[i]->setText ("StreamID: XXXXXXX");
        m_streamIdLabel[i]->adjustSize();
        m_streamIdLabel[i]->setText ("");

        m_validPixelsNumberLabel[i] = new QLabel (m_2dViews[i]);
        m_validPixelsNumberLabel[i]->setAlignment (Qt::AlignCenter);
        m_validPixelsNumberLabel[i]->setStyleSheet ("QLabel { background-color: rgba(0, 0, 0, 0); color : white;font: 15pt; }");
        m_validPixelsNumberLabel[i]->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        m_validPixelsNumberLabel[i]->hide();
        m_validPixelsNumberLabel[i]->setText ("Number of valid pixels: XXXXXX");
        m_validPixelsNumberLabel[i]->adjustSize();
        m_validPixelsNumberLabel[i]->setText ("");
    }

    widgets.filterMinMaxView = new FilterMinMaxView;
    QObject::connect (widgets.filterMinMaxView, SIGNAL (filterMinChanged (float)), this, SLOT (onFilterMinChanged (float)));
    QObject::connect (widgets.filterMinMaxView, SIGNAL (filterMaxChanged (float)), this, SLOT (onFilterMaxChanged (float)));

    QObject::connect (widgets.filterMinMaxView, SIGNAL (filterMinChanged (float)), this, SLOT (repeat()));
    QObject::connect (widgets.filterMinMaxView, SIGNAL (filterMaxChanged (float)), this, SLOT (repeat()));
    widgets.filterMinMaxView->setVisibility (true);

    widgets.filterLevelView = new FilterLevelView;
    QObject::connect (widgets.filterLevelView, SIGNAL (filterLevelChanged (const royale::FilterLevel &, royale::StreamId)),
                      this, SLOT (filterLevelChanged (const royale::FilterLevel &, royale::StreamId)));

    widgets.colorRangeView = new ColorRangeView;
    QObject::connect (widgets.colorRangeView, SIGNAL (distColorRangeChanged (float, float)), this, SLOT (distColorRangeChanged (float, float)));
    QObject::connect (widgets.colorRangeView, SIGNAL (grayColorRangeChanged (int, int)), this, SLOT (grayColorRangeChanged (int, int)));
    QObject::connect (widgets.colorRangeView, SIGNAL (autoColorRangeUpdate()), this, SLOT (autoColorRangeUpdate()));
    widgets.colorRangeView->showGroups (true, false);

    widgets.autoRotationView = new AutoRotationView;
    QObject::connect (widgets.autoRotationView, SIGNAL (rotationModeChanged (bool)), this, SLOT (autoRotationEnabled (bool)));
    QObject::connect (widgets.autoRotationView, SIGNAL (rCenterChanged (float)), this, SLOT (onRCenterChanged (float)));
    QObject::connect (widgets.autoRotationView, SIGNAL (rSpeedChanged (float)), this, SLOT (onRSpeedChanged (float)));
    widgets.autoRotationView->setVisibility (true);

    widgets.parameterView = new ParameterView;
    QObject::connect (widgets.parameterView, SIGNAL (changePipelineParameter (royale::Pair<royale::ProcessingFlag, royale::Variant>)), this, SLOT (changePipelineParameter (royale::Pair<royale::ProcessingFlag, royale::Variant>)));
    QObject::connect (widgets.parameterView, SIGNAL (changePipelineParameter (royale::Pair<royale::ProcessingFlag, royale::Variant>)), this, SLOT (repeat()));

    widgets.cameraPositionView = new CameraPositionView;
    QObject::connect (widgets.cameraPositionView, SIGNAL (cameraPositionPreset (int)), this, SLOT (cameraPositionPreset (int)));

    widgets.dataSelectorView = new DataSelectorView;
    QObject::connect (widgets.dataSelectorView,   SIGNAL (dataSelectorSwitched (int)), this,                       SLOT (dataSelectorSwitched (int)));

    widgets.dataSelectorView->displayUniformMode (!m_2d);

    widgets.usecaseView = new ListView;
    QObject::connect (widgets.usecaseView, SIGNAL (itemSelected (const QString &)), this, SLOT (useCaseSelected (const QString &)));

    widgets.exposureView = new ExposureView;
    QObject::connect (this, SIGNAL (exposureLimitsChanged (int, int)), widgets.exposureView, SLOT (minMaxChanged (int, int)));
    QObject::connect (widgets.exposureView, SIGNAL (exposureValueChanged (int)), this, SLOT (exposureChanged (int)));
    QObject::connect (widgets.exposureView, SIGNAL (exposureModeChanged (bool)), this, SLOT (autoExposureEnabled (bool)));

    widgets.framerateView = new FramerateView;
    QObject::connect (this, SIGNAL (framerateLimitsChanged (int, int)), widgets.framerateView, SLOT (minMaxChanged (int, int)));
    QObject::connect (widgets.framerateView, SIGNAL (framerateValueChanged (int)), this, SLOT (framerateChanged (int)));
    QObject::connect (this, SIGNAL (framerateValue (int)), widgets.framerateView, SLOT (valueChanged (int)));

    widgets.fileView = new OpenFileView;
    QObject::connect (widgets.fileView, SIGNAL (fileSelected (const std::string &)), this, SLOT (openPlaybackFile (const std::string &)));

    toolsButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-tools-default"));
    toolsButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-tools-pressed"));
    forceColorButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-color-default"));
    forceColorButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-color-pressed"));
    helpButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-help-default"));
    helpButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-help-pressed"));
    viewButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-3d-default"));
    viewButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-3d-pressed"));
    cameraButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-connect-default"));
    cameraButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-connect-pressed"));
    cameraButton->setText (qStrStartText);

    recButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-rec-default"));
    recButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-rec-pressed"));

    toolsButton->setCheckable (true);
    helpButton->setCheckable (true);
    recButton->setCheckable (true);

    m_toolsPanel = new PMDView (this, 4, 4);
    widgets.toolsMenu = new SettingsMenuView (m_accessLevel);
    m_toolsPanel->layout()->addWidget (widgets.toolsMenu);
    widgets.toolsMenu->setDelegate (this);

    m_helpPanel = new PMDView (this, 4, 4);
    widgets.helpView = new HelpView();
    m_helpPanel->layout()->addWidget (widgets.helpView);

    m_licensePanel = new PMDView (this, 4, 4);
    widgets.licenseView = new LicenseView();
    m_licensePanel->layout()->addWidget (widgets.licenseView);

    QObject::connect (widgets.helpView, SIGNAL (newLogMessage()), this, SLOT (newLogMessageAvailable()));
    QObject::connect (widgets.helpView, SIGNAL (licensePanelToggled()), this, SLOT (displayLicense()));
    QObject::connect (widgets.helpView, SIGNAL (tabBarClicked (int)), this, SLOT (setHelpTabBarClicked (int)));
    QObject::connect (widgets.helpView, SIGNAL (refreshButtonClicked()), this, SLOT (refreshInfoMessage()));
    QObject::connect (this, SIGNAL (logMessage (const QString &)), widgets.helpView, SLOT (addLogMessage (const QString &)));
    QObject::connect (toolsButton, SIGNAL (clicked (bool)), this, SLOT (toolsMenuClicked()));
    QObject::connect (helpButton, SIGNAL (clicked (bool)), this, SLOT (displayLog()));
    QObject::connect (forceColorButton, SIGNAL (clicked()), this, SLOT (forceColorButtonClicked()));
    QObject::connect (widgets.colorRangeView, SIGNAL (forceButtonClicked()), forceColorButton, SLOT (click()));
    QObject::connect (cameraButton, SIGNAL (clicked (bool)), this, SLOT (cameraButtonClicked()));
    QObject::connect (recButton,    SIGNAL (clicked (bool)), this, SLOT (recButtonClicked()));
    QObject::connect (viewButton,    SIGNAL (clicked (bool)), this, SLOT (viewButtonClicked()));

    QObject::connect (widgets.toolsMenu, SIGNAL (enableLockView (bool)), m_3dViews[0], SLOT (onLockViewEnabled (bool)));
    QObject::connect (widgets.toolsMenu, SIGNAL (enableLockView (bool)), this, SLOT (hideTool()));
    QObject::connect (widgets.toolsMenu, SIGNAL (enableFlipVertically (bool)), this, SLOT (flipVertically (bool)));
    QObject::connect (widgets.toolsMenu, SIGNAL (enableFlipVertically (bool)), this, SLOT (hideTool()));
    QObject::connect (widgets.toolsMenu, SIGNAL (enableFlipHorizontally (bool)), this, SLOT (flipHorizontally (bool)));
    QObject::connect (widgets.toolsMenu, SIGNAL (enableFlipHorizontally (bool)), this, SLOT (hideTool()));
    QObject::connect (widgets.toolsMenu, SIGNAL (enableShowFPS (bool)), this, SLOT (enableFPSDisplay (bool)));
    QObject::connect (widgets.toolsMenu, SIGNAL (enableShowFPS (bool)), this, SLOT (hideTool()));
    QObject::connect (widgets.toolsMenu, SIGNAL (enableShowStreamId (bool)), this, SLOT (enableStreamIdDisplay (bool)));
    QObject::connect (widgets.toolsMenu, SIGNAL (enableShowStreamId (bool)), this, SLOT (hideTool()));
    QObject::connect (widgets.toolsMenu, SIGNAL (enableShowValidPixelsNumber (bool)), this, SLOT (enableValidPixelsNumber (bool)));
    QObject::connect (widgets.toolsMenu, SIGNAL (enableShowValidPixelsNumber (bool)), this, SLOT (hideTool()));
    for (auto i = 0; i < m_maxStreams; ++i)
    {
        QObject::connect (widgets.toolsMenu, SIGNAL (enableFrustumDisplay (bool)), m_3dViews[i], SLOT (frustumDisplayToggled (bool)));
        QObject::connect (widgets.toolsMenu, SIGNAL (enableLockView (bool)), m_3dViews[i], SLOT (lockViewToggled (bool)));


        QObject::connect (m_2dViews[i], SIGNAL (closeLicense()), this, SLOT (closeLicense()));
        QObject::connect (m_3dViews[i], SIGNAL (clicked()), this, SLOT (closeLicense()));
        QObject::connect (m_3dViews[i], SIGNAL (cameraPositionChanged (const QMatrix4x4, const QMatrix4x4)), this, SLOT (setCameraPositionChanged (const QMatrix4x4, const QMatrix4x4)));
        QObject::connect (m_3dViews[i], SIGNAL (autoRotationStatusChanged (bool, bool, bool)), this, SLOT (setAutoRotationStatusChanged (bool, bool, bool)));
        QObject::connect (m_3dViews[i], SIGNAL (rotatingSpeedChanged (float)), this, SLOT (syncRotatingSpeedChanged (float)));

        m_2dViews[i]->lower();
        m_3dViews[i]->lower();
    }

    QObject::connect (widgets.toolsMenu, SIGNAL (enableSingleFrameRecording (bool)), this, SLOT (enableSingleFrameRecording (bool)));

    // r c rs cs
    gridLayout->addWidget (m_toolsPanel, 0, 0, 1, 1);
    gridLayout->addWidget (m_helpPanel, 0, 2, 1, 100);
    gridLayout->addWidget (m_licensePanel, 0, 1);
    gridLayout->setAlignment (m_toolsPanel, Qt::AlignLeft | Qt::AlignTop);
    gridLayout->setAlignment (m_helpPanel, Qt::AlignRight | Qt::AlignTop);
    gridLayout->setAlignment (m_licensePanel, Qt::AlignCenter);

    gridLayout->setColumnStretch (0, 0);
    gridLayout->setColumnStretch (1, 1);
    gridLayout->setColumnStretch (2, 0);
    gridLayout->setColumnStretch (3, 0);
    gridLayout->setColumnStretch (4, 0);

    int margin = 12;
    leftColumn->setContentsMargins (margin, margin, 0, margin);
    gridLayout->setContentsMargins (0, margin, 0, margin);
    rightColumn->setContentsMargins (0, margin, margin, margin);

    movieControl->hide();
    widgets.playerControl = new PlayerControl ();
    movieControl->layout()->addWidget (widgets.playerControl);

    QObject::connect (widgets.playerControl, SIGNAL (rewind()), this, SLOT (rewind()));
    QObject::connect (widgets.playerControl, SIGNAL (playStop (bool)), this, SLOT (playbackStopPlay (bool)));
    QObject::connect (widgets.playerControl, SIGNAL (forward()), this, SLOT (forward()));
    QObject::connect (widgets.playerControl, SIGNAL (seekTo (int)), this, SLOT (seekTo (int)));

    registerWidgetContainer->hide();
    if (m_accessLevel >= CameraAccessLevel::L3)
    {
        auto registerWidget = new RegisterView();
        registerWidgetContainer->layout()->addWidget (registerWidget);
        registerWidgetContainer->show();

        QObject::connect (registerWidget, SIGNAL (readRegister (QString)), this, SLOT (readRegister (QString)));
        QObject::connect (registerWidget, SIGNAL (writeRegister (QString, uint16_t)), this, SLOT (writeRegister (QString, uint16_t)));
        QObject::connect (this, SIGNAL (registerReadReturn (QString, uint16_t)), registerWidget, SLOT (registerReadReturn (QString, uint16_t)));
        QObject::connect (this, SIGNAL (registerWriteReturn (bool)), registerWidget, SLOT (registerWriteReturn (bool)));
    }
}

void QTViewer::initAutoExposureModes (bool enabled)
{
    if (enabled)
    {
        for (auto i = 0; i < m_maxStreams; ++i)
        {
            m_exposureModes[i] = ExposureMode::AUTOMATIC;
        }
    }
}

void QTViewer::closeEvent (QCloseEvent *event)
{
    cleanUp();
}

void QTViewer::cleanUp()
{
    stopCapture();
    m_cameraDevice.reset (nullptr);
}

void QTViewer::toolsMenuClicked()
{
    if (widgets.currentTool)
    {
        widgets.currentTool->hide();
    }
    widgets.toolsMenu->deselect();

    closeLicense();
    m_toolsPanel->toggle();
}

void QTViewer::keyPressEvent (QKeyEvent *event)
{
    if (!widgets.parameterView->isVisible())
    {
        switch (event->key())
        {
#ifndef TARGET_PLATFORM_ANDROID
            case Qt::Key_Return:
            case Qt::Key_Enter:
                {
                    if (event->modifiers().testFlag (Qt::AltModifier))
                    {
                        if (this->isFullScreen())
                        {
                            this->showNormal();
                        }
                        else
                        {
                            this->showFullScreen();
                        }
                    }
                    break;
                }
#endif
            case Qt::Key_Escape:
                {
                    cleanUp();
                    close();
                    break;
                }
            case Qt::Key_H:
                {
                    hideButtons (!m_buttonsHidden);
                    break;
                }
            case Qt::Key_2:
                {
                    if (!m_2d)
                    {
                        viewButton->setCheckable (true);
                        viewButton->setChecked (true);
                    }
                    break;
                }
            case Qt::Key_3:
                {
                    if (m_2d)
                    {
                        viewButton->setCheckable (true);
                        viewButton->setChecked (true);
                    }
                    break;
                }
            case Qt::Key_F1:
                {
                    dataSelectorSwitched (DataSelector_Gray);
                    break;
                }
            case Qt::Key_F2:
                {
                    dataSelectorSwitched (DataSelector_Distance);
                    break;
                }
            case Qt::Key_F3:
                {
                    dataSelectorSwitched (DataSelector_Overlay);
                    break;
                }
            case Qt::Key_F4:
                {
                    if (!m_2d)
                    {
                        dataSelectorSwitched (DataSelector_GrayUni);
                    }
                    break;
                }
            case Qt::Key_O:
                {
#ifndef TARGET_PLATFORM_ANDROID
                    if (event->modifiers().testFlag (Qt::ControlModifier))
                    {
                        loadFile();
                    }
#endif
                    break;
                }
            case Qt::Key_C:
                {
                    forceColorButton->setCheckable (true);
                    forceColorButton->setChecked (true);
                    break;
                }
            case Qt::Key_S:
                {
                    cameraButton->click();
                    break;
                }
            case Qt::Key_R:
                {
                    recButtonClicked();
                    break;
                }
            case Qt::Key_Space:
                {
                    if (m_replayControl)
                    {
                        widgets.playerControl->playStopCallback();
                    }
                    break;
                }
            case Qt::Key_Left:
                {
                    if (m_licensePanel->isVisible())
                    {
                        widgets.licenseView->switchLicenseTab (-1);
                    }
                    else
                    {
                        rewind();
                    }
                    break;
                }
            case Qt::Key_Right:
                {
                    if (m_licensePanel->isVisible())
                    {
                        widgets.licenseView->switchLicenseTab (1);
                    }
                    else
                    {
                        forward();
                    }
                    break;
                }
            case Qt::Key_Up:
                {
                    if (m_helpPanel->isVisible())
                    {
                        widgets.helpView->switchHelpTab (-1);
                    }
                    break;
                }
            case Qt::Key_Down:
                {
                    if (m_helpPanel->isVisible())
                    {
                        widgets.helpView->switchHelpTab (1);
                    }
                    break;
                }
            default:
                {
                    event->ignore();
                    break;
                }
        }
    }
}

void QTViewer::keyReleaseEvent (QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_2:
            {
                if (!widgets.parameterView->isVisible())
                {
                    if (!m_2d)
                    {
                        viewButton->setChecked (false);
                        viewButton->setCheckable (false);
                        setCurrentView (true);
                    }
                }
                break;
            }
        case Qt::Key_3:
            {
                if (!widgets.parameterView->isVisible())
                {
                    if (m_2d)
                    {
                        viewButton->setChecked (false);
                        viewButton->setCheckable (false);
                        setCurrentView (false);
                    }
                }
                break;
            }
        case Qt::Key_C:
            {
                forceColorButton->setChecked (false);
                forceColorButton->setCheckable (false);
                forceColorButtonClicked();
                break;
            }
        default:
            {
                event->ignore();
                break;
            }
    }
}

#ifdef TARGET_PLATFORM_ANDROID
CameraStatus QTViewer::connectCamera (uint32_t androidUsbDeviceFD,
                                      uint32_t androidUsbDeviceVid,
                                      uint32_t androidUsbDevicePid)
#else
CameraStatus QTViewer::connectCamera()
#endif
{
    checkForNewAccessCode ();
    widgets.helpView->clearInfoMessage();

    royale::String accessCode = ROYALE_ACCESS_CODE_LEVEL2;
    if (m_accessLevel > CameraAccessLevel::L2)
    {
        accessCode = m_accessCode;
    }
    CameraManager manager (accessCode);

    // Listen for possible error events.
    manager.registerEventListener (this);

    if (!m_playbackFilename.empty())
    {
        refreshInfoMessage();
        widgets.toolsMenu->disableRRFRecording (true);
        m_cameraDevice = manager.createCamera (m_playbackFilename);
        if (m_cameraDevice)
        {
            widgets.helpView->addLogMessage ("Successfully loaded recording.");
        }
        else
        {
            widgets.helpView->addLogMessage ("Error loading playback file!");
            return CameraStatus::COULD_NOT_OPEN;
        }
        m_serialNumber = "";
        if (!m_calibrationFileName.empty())
        {
            auto ret = m_cameraDevice->setCalibrationData (m_calibrationFileName);
            if (ret == CameraStatus::SUCCESS)
            {
                widgets.helpView->addLogMessage ("Successfully loaded calibration data from file.");
            }
            else
            {
                widgets.helpView->addLogMessage ("Error loading calibration data from file!");
                m_cameraDevice.reset (nullptr);
                return CameraStatus::COULD_NOT_OPEN;
            }
        }
    }
    else
    {
#ifdef TARGET_PLATFORM_ANDROID
        auto connectedCameras = manager.getConnectedCameraList (androidUsbDeviceFD, androidUsbDeviceVid, androidUsbDevicePid);
#else
        auto connectedCameras = manager.getConnectedCameraList();
#endif
        auto cameraNames = manager.getConnectedCameraNames();
        widgets.toolsMenu->disableRRFRecording (false);
        if (connectedCameras.size() == 1)
        {
            widgets.helpView->addLogMessage ("1 camera found");
            widgets.helpView->addInfoMessage ("1 camera found :");
            widgets.helpView->addInfoMessage ("Name : " + QString (cameraNames[0].c_str()));
            widgets.helpView->addInfoMessage ("ID : " + QString (connectedCameras[0].c_str()));
            m_cameraDevice = manager.createCamera (connectedCameras[0]);
            m_serialNumber = connectedCameras[0].toStdString();
        }
        else if (connectedCameras.size() > 1)
        {
            QApplication::restoreOverrideCursor();

            widgets.helpView->addLogMessage (QString::number (connectedCameras.size()) + " cameras found");
            widgets.helpView->addInfoMessage (QString::number (connectedCameras.size()) + " cameras found :");

            QStringList items;
            for (size_t i = 0; i < connectedCameras.size(); ++i)
            {
                widgets.helpView->addInfoMessage ("- " + QString::number (i + 1) + " -");
                widgets.helpView->addInfoMessage ("Name : " + QString (cameraNames[i].c_str()));
                widgets.helpView->addInfoMessage ("ID : " + QString (connectedCameras[i].c_str()));
                items << QString (connectedCameras[i].c_str ());
            }

            QInputDialog dlg;
            QString curText (items.value (0));
            dlg.setWindowTitle ("Select camera");
            dlg.setLabelText ("Cameras");
            dlg.setComboBoxItems (items);
            dlg.setTextValue (curText);
            dlg.setComboBoxEditable (false);
            dlg.resize (300, 100);
            const auto ret = dlg.exec();

            QApplication::setOverrideCursor (Qt::WaitCursor);

            if (ret == 0)
            {
                // User pressed cancel -> no camera selected
                widgets.helpView->addLogMessage ("No camera selected - canceling");
                return CameraStatus::LOGIC_ERROR;
            }

            int selection = -1;
            for (size_t i = 0; i < connectedCameras.size(); ++i)
            {
                if (dlg.textValue() == QString (connectedCameras[i].c_str()))
                {
                    selection = static_cast<int> (i);
                }
            }

            widgets.helpView->addLogMessage ("Selected camera " + QString::number (selection + 1) + " : " +
                                             QString (connectedCameras[selection].c_str()));

            m_cameraDevice = manager.createCamera (connectedCameras[selection]);
            m_serialNumber = connectedCameras[selection].toStdString();
        }
        else
        {
            widgets.helpView->addLogMessage ("No camera found");
            widgets.helpView->addInfoMessage ("No camera found");
            return CameraStatus::COULD_NOT_OPEN;
        }

        if (m_cameraDevice)
        {
            widgets.helpView->addLogMessage ("Successfully connected to camera.");
        }
        else
        {
            widgets.helpView->addLogMessage ("Error connecting to camera!");
            return CameraStatus::COULD_NOT_OPEN;
        }
        if (!m_calibrationFileName.empty())
        {
            auto ret = m_cameraDevice->setCalibrationData (m_calibrationFileName);
            if (ret == CameraStatus::SUCCESS)
            {
                widgets.helpView->addLogMessage ("Successfully loaded calibration data from file.");
            }
            else
            {
                widgets.helpView->addLogMessage ("Error loading calibration data from file!");
                m_cameraDevice.reset (nullptr);
                return CameraStatus::COULD_NOT_OPEN;
            }
        }
    }

    return CameraStatus::SUCCESS;
}

CameraStatus QTViewer::startCapture()
{
    if (m_cameraDevice == nullptr)
    {
        widgets.helpView->addLogMessage ("Runtime Error during capture start.");
        return CameraStatus::RUNTIME_ERROR;
    }
    bool connected;
    auto ret = m_cameraDevice->isConnected (connected);
    if (ret != CameraStatus::SUCCESS)
    {
        return ret;
    }
    if (!connected)
    {
        widgets.helpView->addLogMessage ("Camera disconnected.");
        return CameraStatus::DISCONNECTED;
    }
    widgets.helpView->addLogMessage ("Starting camera capture");
    m_cameraDevice->registerEventListener (this);

    return m_cameraDevice->startCapture();
}

CameraStatus QTViewer::stopCapture()
{
    widgets.helpView->addLogMessage ("Stopping camera capture");
    if (m_cameraDevice == nullptr)
    {
        return CameraStatus::RUNTIME_ERROR;
    }
    m_cameraDevice->stopCapture();
    m_cameraDevice->unregisterEventListener();
#ifndef TARGET_PLATFORM_ANDROID
    m_cameraDevice.reset (nullptr);
#endif

    return CameraStatus::SUCCESS;
}

void QTViewer::onNewData (const royale::IExtendedData *data)
{
    if (data->hasRawData())
    {
        m_currentTemperature = data->getRawData()->illuminationTemperature;
    }
    if (data->hasDepthData() &&
            data->hasIntermediateData())
    {
        MutexTryLocker tlocker (*m_dataMutex);
        if (!tlocker.isLocked())
        {
            return;
        }

        auto curDepthData = data->getDepthData();
        auto curIntermediateData = data->getIntermediateData();

        int curViewIdx = 0;
        if (m_streamIdMap.find (curDepthData->streamId) == m_streamIdMap.end())
        {
            // This shouldn't happen, the stream ID was not registered
            return;
        }
        else
        {
            curViewIdx = m_streamIdMap[curDepthData->streamId];
        }

        m_currentData[curDepthData->streamId] = *curDepthData;
        m_currentIntermediateData[curDepthData->streamId] = *curIntermediateData;

        if (m_firstData[curDepthData->streamId])
        {
            findMinMax (&m_currentData[curDepthData->streamId], curDepthData->streamId);
        }

        if (m_flipHorizontal ||
                m_flipVertical)
        {
            float factorX = 1.0f;
            float factorY = 1.0f;

            if (m_flipHorizontal)
            {
                factorX = -1.0f;
            }
            if (m_flipVertical)
            {
                factorY = -1.0f;
            }

            for (auto &curPoint : m_currentData[curDepthData->streamId].points)
            {
                curPoint.x *= factorX;
                curPoint.y *= factorY;
            }
        }

        m_hasData[curDepthData->streamId] = true;
        m_frameCounter[curDepthData->streamId]++;

        if (m_showValidPixelsNumber && m_updateValidPixelsNumber)
        {
            countValidPixelsNumber (&m_currentData[curDepthData->streamId], curDepthData->streamId);
        }

        if (m_2d)
        {
            emit m_2dViews[curViewIdx]->newData (&m_currentData[curDepthData->streamId],
                                                 &m_currentIntermediateData[curDepthData->streamId]);
        }
        else
        {
            emit m_3dViews[curViewIdx]->newData (&m_currentData[curDepthData->streamId],
                                                 &m_currentIntermediateData[curDepthData->streamId]);
        }

        if (m_replayControl)
        {
            widgets.playerControl->updateSlider (m_replayControl->currentFrame());
        }

        m_firstData[curDepthData->streamId] = false;
    }
}

void QTViewer::findMinMax (const royale::DepthData *data, royale::StreamId streamId)
{
    // calculate the number of points (except invalid points) and the corresponding total distance
    double sumDist = 0;
    uint32_t sumPoints = 0;
    std::vector<int> depthConfidence;
    depthConfidence.resize (data->points.size());
    for (size_t i = 0; i < data->points.size(); ++i)
    {
        auto curPoint = data->points.at (i);
        if (curPoint.z < m_rangeFilterSettings[streamId].filterMin || curPoint.z > m_rangeFilterSettings[streamId].filterMax)
        {
            curPoint.depthConfidence = 0;
        }
        if (curPoint.depthConfidence > 0)
        {
            sumDist += curPoint.z;
            sumPoints++;
            depthConfidence[i] = 1;
        }
        else
        {
            depthConfidence[i] = 0;
        }
    }

    if (sumPoints != 0)
    {
        // calculate the mean distance
        float meanDist = static_cast<float> (sumDist / sumPoints);

        // calculate the absolute deviation (the distance bewteen pixel and mean distance)
        std::vector<float> adDist;
        adDist.resize (data->points.size());
        double sumAdDist = 0;
        for (size_t i = 0; i < data->points.size(); ++i)
        {
            auto curPoint = data->points.at (i);
            if (depthConfidence[i])
            {
                adDist[i] = std::fabs (curPoint.z - meanDist);
                sumAdDist += adDist[i];
            }
            else
            {
                adDist[i] = -1;
            }
        }
        depthConfidence.clear();
        // calculate the mean absolute deviation
        float meanAdDist = static_cast<float> (sumAdDist / sumPoints);

        // mark the points which are close to the mean distance (not larger than mean absolute deviation) as validPoints
        // save other points as outliers (except invalid points)
        std::vector<int> mark;
        mark.resize (data->points.size());
        std::vector<size_t> outliers;
        outliers.resize (data->points.size());
        uint32_t validPoints = 0;
        uint32_t outliersCount = 0;
        for (size_t i = 0; i < data->points.size(); ++i)
        {
            if (adDist[i] >= 0)
            {
                if (adDist[i] <= meanAdDist)
                {
                    mark[i] = 1;
                    validPoints++;
                }
                else
                {
                    mark[i] = 0;
                    outliers[outliersCount] = i;
                    outliersCount++;
                }
            }
            else
            {
                mark[i] = 0;
            }
        }

        // ensure the number of valid points.
        // relax the threshold to add valid points from outliers, if the number of valid points is not enough
        // initial threshold is mean absolute deviation
        float threshold = meanAdDist;
        float rate = static_cast<float> (validPoints) / static_cast<float> (sumPoints);
        std::vector<size_t> newOutliers;
        while (rate < 0.8)
        {
            threshold += 0.1f;
            newOutliers.resize (outliersCount);
            uint32_t newOutliersCount = 0;

            for (size_t i = 0; i < outliersCount; ++i)
            {
                if (adDist[outliers[i]] <= threshold)
                {
                    mark[outliers[i]] = 1;
                    validPoints++;
                }
                else
                {
                    newOutliers[newOutliersCount] = i;
                    newOutliersCount++;
                }
            }
            outliers.clear();
            outliers.resize (outliersCount);
            outliers = newOutliers;
            newOutliers.clear();
            rate = static_cast<float> (validPoints) / static_cast<float> (sumPoints);
        }
        outliers.clear();
        adDist.clear();

        float minDist = std::numeric_limits<float>::max();
        float maxDist = std::numeric_limits<float>::min();
        uint16_t minGray = std::numeric_limits<uint16_t>::max();
        uint16_t maxGray = std::numeric_limits<uint16_t>::min();

        bool newMinMaxSet = false;

        // determine the maximum and minimum values in valid points
        for (size_t i = 0; i < data->points.size(); ++i)
        {
            if (mark[i] == 1)
            {
                auto curPoint = data->points.at (i);
                if (curPoint.z < minDist)
                {
                    minDist = curPoint.z;
                    newMinMaxSet = true;
                }
                else if (curPoint.z > maxDist)
                {
                    maxDist = curPoint.z;
                    newMinMaxSet = true;
                }
                if (curPoint.grayValue < minGray)
                {
                    minGray = curPoint.grayValue;
                    newMinMaxSet = true;
                }
                else if (curPoint.grayValue > maxGray)
                {
                    maxGray = curPoint.grayValue;
                    newMinMaxSet = true;
                }
            }
        }
        mark.clear();

        if (newMinMaxSet)
        {
            emit autoColorRangeFinished (minDist, maxDist, minGray, maxGray, streamId);
        }
    }
}

void QTViewer::settingSelected (const QString &name)
{
    PMDView *panel = nullptr;
    if (name == "Color Range")
    {
        PMDStreamView *tmppanel = new PMDStreamView (this, 4, 4);
        tmppanel->layout()->addWidget (widgets.colorRangeView);
        QObject::connect (tmppanel, SIGNAL (streamIdChanged (royale::StreamId)), this, SLOT (setStreamIdChanged (royale::StreamId)));
        QObject::connect (tmppanel, SIGNAL (streamIdChanged (royale::StreamId)), this, SLOT (streamIdColorRangeViewChanged (royale::StreamId)));
        tmppanel->updateStreams (m_streamIds);
        panel = tmppanel;
    }
    else if (name == "Filter level")
    {
        PMDStreamView *tmppanel = new PMDStreamView (this, 2, 3);
        tmppanel->layout()->addWidget (widgets.filterLevelView);
        QObject::connect (tmppanel, SIGNAL (streamIdChanged (royale::StreamId)), this, SLOT (streamIdFilterLevelViewChanged (royale::StreamId)));
        tmppanel->updateStreams (m_streamIds);
        panel = tmppanel;
    }
    else if (name == "Filter (Min/Max)")
    {
        PMDStreamView *tmppanel = new PMDStreamView (this, 3, 3);
        tmppanel->layout()->addWidget (widgets.filterMinMaxView);
        QObject::connect (tmppanel, SIGNAL (streamIdChanged (royale::StreamId)), this, SLOT (streamIdFilterRangeViewChanged (royale::StreamId)));
        tmppanel->updateStreams (m_streamIds);
        panel = tmppanel;
    }
    else if (name == "Auto Rotation")
    {
        PMDStreamView *tmppanel = new PMDStreamView (this, 3, 3);
        tmppanel->layout()->addWidget (widgets.autoRotationView);
        QObject::connect (tmppanel, SIGNAL (streamIdChanged (royale::StreamId)), this, SLOT (setStreamIdChanged (royale::StreamId)));
        QObject::connect (tmppanel, SIGNAL (streamIdChanged (royale::StreamId)), this, SLOT (streamIdAutoRotationViewChanged (royale::StreamId)));
        tmppanel->updateStreams (m_streamIds);
        panel = tmppanel;
    }
    else if (name == "Camera Preset")
    {
        panel = new PMDView (this, 3, 3);
        panel->layout()->addWidget (widgets.cameraPositionView);
    }
    else if (name == "Data")
    {
        panel = new PMDView (this, 3, 3);
        panel->layout()->addWidget (widgets.dataSelectorView);
    }
    else if (name == "Parameters")
    {
        PMDStreamView *tmppanel = new PMDStreamView (this, 4, 6);
        tmppanel->layout()->addWidget (widgets.parameterView);
        QObject::connect (tmppanel, SIGNAL (streamIdChanged (royale::StreamId)), this, SLOT (streamIdParameterViewChanged (royale::StreamId)));
        tmppanel->updateStreams (m_streamIds);
        panel = tmppanel;
    }
    else if (name == "Use Case (FPS)")
    {
        panel = new PMDView (this, 3, 3);
        panel->layout()->addWidget (widgets.usecaseView);
    }
    else if (name == "Exposure Time")
    {
        PMDStreamView *tmppanel = new PMDStreamView (this, 3, 3);
        tmppanel->layout()->addWidget (widgets.exposureView);
        QObject::connect (tmppanel, SIGNAL (streamIdChanged (royale::StreamId)), this, SLOT (streamIdExposureViewChanged (royale::StreamId)));
        tmppanel->updateStreams (m_streamIds);
        panel = tmppanel;
    }
    else if (name == "Frame Rate")
    {
        panel = new PMDView (this, 3, 3);
        panel->layout()->addWidget (widgets.framerateView);
    }
    else if (name == "Load File")
    {
#ifdef TARGET_PLATFORM_ANDROID
        panel = new PMDView (this, 3, 4);
        panel->layout()->addWidget (widgets.fileView);
#else
        loadFile ();
        hideTool();
#endif
    }
    else if (name == "Hide buttons")
    {
        hideButtons (true);
        toolsMenuClicked();
        m_helpPanel->hide();
        m_licensePanel->hide();
    }

    if (widgets.currentTool)
    {
        widgets.currentTool ->hide();
    }
    const int toolRow = 0;
    const int toolColumn = 1;
    if (panel)
    {
        gridLayout->addWidget (panel, toolRow, toolColumn, 1, 100);
        gridLayout->setAlignment (panel, Qt::AlignLeft | Qt::AlignTop);
        panel->show();
        widgets.currentTool = panel;
    }
}

void QTViewer::cameraPositionPreset (int preset)
{
    for (auto cur3DView : m_3dViews)
    {
        switch (preset)
        {
            case CameraPositionPreset_Side:
                cur3DView->setCameraPosition (ThreeDView::ViewType::SIDE);
                break;
            case CameraPositionPreset_Bird:
                cur3DView->setCameraPosition (ThreeDView::ViewType::BIRD);
                break;
            case CameraPositionPreset_Front:
            // FALLTHROUGH
            default:
                cur3DView->setCameraPosition (ThreeDView::ViewType::FRONT);
                break;
        }
    }

    for (auto i = 0u; i < m_streamIds.size(); ++i)
    {
        m_autoRotationSettings[m_streamIds[i]].center = 2.f;
    }
}

void QTViewer::onApplicationStateChanged (Qt::ApplicationState state)
{
    switch (state)
    {
        case Qt::ApplicationState::ApplicationActive:
            checkForNewAccessCode();

            //reconnect the camera
            if (m_forceCapturingOnNextStateChange)
            {
                // default startup
#ifdef TARGET_PLATFORM_ANDROID
                startCapture();
                // reset
                m_forceCapturingOnNextStateChange = false;
#endif
            }
            break;
        case Qt::ApplicationState::ApplicationInactive:
            {
#ifdef TARGET_PLATFORM_ANDROID
                //application is sent to background (e.g.: android home button pressed)
                if (m_cameraDevice != nullptr)
                {
                    bool capturing;
                    auto res = m_cameraDevice->isCapturing (capturing);

                    if (capturing && res == CameraStatus::SUCCESS)
                    {
                        stopCapture();
                        m_forceCapturingOnNextStateChange = true;
                    }
                }
#endif
            }
            break;
        default:
            break;
    }
}

void QTViewer::changeColorRanges (float minDist, float maxDist, uint16_t minGray, uint16_t maxGray, royale::StreamId streamId)
{
    if (widgets.colorRangeView->getCurrentStreamId() == streamId)
    {
        widgets.colorRangeView->setDistRange (minDist, maxDist);
        widgets.colorRangeView->setGrayRange (minGray, maxGray);
    }
}

void QTViewer::dataSelectorSwitched (int mode)
{
    switch (mode)
    {
        case DataSelector_Distance:
        // FALLTHROUGH
        default:
            m_showDistance = true;
            m_showGrayimage = false;
            m_showOverlayimage = false;
            widgets.colorRangeView->setDistSliderMinMax (10, 450);
            widgets.colorRangeView->showGroups (true, false);
            for (auto i = 0; i < m_maxStreams; ++i)
            {
                m_2dViews[i]->switchToDistanceBuffer();
                m_3dViews[i]->switchToDistanceBuffer();
            }
            break;
        case DataSelector_Gray:
            m_showDistance = false;
            m_showGrayimage = true;
            m_showOverlayimage = false;
            widgets.colorRangeView->setGraySliderMinMax (0, 4095);
            widgets.colorRangeView->showGroups (false, true);
            for (auto i = 0; i < m_maxStreams; ++i)
            {
                m_2dViews[i]->switchToGrayBuffer (false);
                m_3dViews[i]->switchToGrayBuffer (false);
            }
            break;
        case DataSelector_GrayUni:
            m_showDistance = false;
            m_showGrayimage = true;
            m_showOverlayimage = false;
            widgets.colorRangeView->setGraySliderMinMax (0, 4095);
            widgets.colorRangeView->showGroups (false, true);
            for (auto i = 0; i < m_maxStreams; ++i)
            {
                m_2dViews[i]->switchToGrayBuffer (true);
                m_3dViews[i]->switchToGrayBuffer (true);
            }
            break;
        case DataSelector_Amplitude:
            widgets.colorRangeView->setGraySliderMinMax (0, 4095);
            widgets.colorRangeView->showGroups (false, true);
            for (auto i = 0; i < m_maxStreams; ++i)
            {
                m_2dViews[i]->switchToAmplitudeBuffer();
                m_3dViews[i]->switchToAmplitudeBuffer();
            }
            break;

        case DataSelector_Overlay:
            m_showDistance = false;
            m_showGrayimage = false;
            m_showOverlayimage = true;
            widgets.colorRangeView->setDistSliderMinMax (10, 450);
            widgets.colorRangeView->setGraySliderMinMax (0, 4095);
            widgets.colorRangeView->showGroups (true, true);
            for (auto i = 0; i < m_maxStreams; ++i)
            {
                m_2dViews[i]->switchToOverlay();
                m_3dViews[i]->switchToOverlay();
            }
            break;
    }
    hideTool();
    autoColorRangeUpdate();
}

void QTViewer::orientationChanged (Qt::ScreenOrientation)
{
    layoutSubviews();
}

void QTViewer::resizeEvent (QResizeEvent *event)
{
    QMainWindow::resizeEvent (event);
    layoutSubviews();
}

void QTViewer::layoutSubviews()
{
    if (m_streamIds.size())
    {
        for (auto i = 0u; i < m_streamIds.size(); ++i)
        {
            if ( (int) m_streamIds.size() == 1)
            {
                m_2dViews[i]->setImageAlignmentMode (ImageAlignmentMode::SingleCentered, cameraButton->width());
            }
            else
            {
                if (i == 0)
                {
                    m_2dViews[i]->setImageAlignmentMode (ImageAlignmentMode::LeftIndent, cameraButton->width());
                }
                else if (i == (m_streamIds.size() - 1))
                {
                    m_2dViews[i]->setImageAlignmentMode (ImageAlignmentMode::RightIndent, cameraButton->width());
                }
                else
                {
                    m_2dViews[i]->setImageAlignmentMode (ImageAlignmentMode::Centered, cameraButton->width());
                }
            }
            m_2dViews[i]->setGeometry ( (int) (this->size().width() / m_streamIds.size()) * i, 0,
                                        (int) (this->size().width() / m_streamIds.size()), this->size().height());
            m_3dViews[i]->setGeometry ( (int) (this->size().width() / m_streamIds.size()) * i, 0,
                                        (int) (this->size().width() / m_streamIds.size()), this->size().height());
            m_2dViews[i]->prepareBackgroundImage();

            m_fpsLabel[i]->move ( (m_2dViews[i]->width() - m_fpsLabel[0]->width()) / 2, m_fpsLabel[0]->height());
            if (m_showFPS)
            {
                m_fpsLabel[i]->show();
            }
            else
            {
                m_fpsLabel[i]->hide();
            }

            m_streamIdLabel[i]->move ( (m_2dViews[i]->width() - m_streamIdLabel[0]->width()) / 2, m_streamIdLabel[0]->height() * 2);
            if (m_showStreamId)
            {
                m_streamIdLabel[i]->show();
            }
            else
            {
                m_streamIdLabel[i]->hide();
            }

            m_validPixelsNumberLabel[i]->move ( (m_2dViews[i]->width() - m_validPixelsNumberLabel[0]->width()) / 2, m_validPixelsNumberLabel[0]->height() * 3);
            if (m_showValidPixelsNumber)
            {
                m_validPixelsNumberLabel[i]->show();
            }
            else
            {
                m_validPixelsNumberLabel[i]->hide();
            }

            if (m_2d)
            {
                m_2dViews[i]->show();
                m_3dViews[i]->hide();
            }
            else
            {
                m_2dViews[i]->hide();
                m_3dViews[i]->show();
            }
        }
        for (auto i = (int) m_streamIds.size(); i < m_maxStreams; ++i)
        {
            m_2dViews[i]->hide();
            m_3dViews[i]->hide();
            m_fpsLabel[i]->hide();
            m_streamIdLabel[i]->hide();
            m_validPixelsNumberLabel[i]->hide();
        }
    }
    else
    {
        m_2dViews[0]->setGeometry (0, 0, this->size().width(), this->size().height());
        m_3dViews[0]->setGeometry (0, 0, this->size().width(), this->size().height());
        m_2dViews[0]->prepareBackgroundImage();
        m_fpsLabel[0]->move ( (this->size().width() - m_fpsLabel[0]->width()) / 2, m_fpsLabel[0]->height());
        m_streamIdLabel[0]->move ( (this->size().width() - m_streamIdLabel[0]->width()) / 2, m_streamIdLabel[0]->height() * 2);
        m_validPixelsNumberLabel[0]->move ( (this->size().width() - m_validPixelsNumberLabel[0]->width()) / 2, m_validPixelsNumberLabel[0]->height() * 3);
    }
    m_licensePanel->setFixedSize (this->size().width() - cameraButton->width() * 3, this->size().height() - cameraButton->width() / 3);
}

void QTViewer::recButtonClicked()
{
    closeLicense();
    QString curDate = QDate::currentDate().toString ("yyyyMMdd");
    QString curTime = QTime::currentTime().toString ("hhmmss");
    QString extension = widgets.toolsMenu->singleFrameRecordingEnabled (!m_playbackFilename.empty()) ? "ply" : "rrf";
    QMutexLocker locker (m_dataMutex);

    if (widgets.toolsMenu->singleFrameRecordingEnabled (!m_playbackFilename.empty()))
    {
        if (m_streamIds.size() != 0)
        {
            QString fileBaseName = QString ("royale_%1_%2_%3.%4").arg (curDate).arg (curTime);
            recButton->setChecked (true);
            recButton->repaint();
            widgets.helpView->addLogMessage ("Single frame recorded.");

            // if the CameraAccessLevel is bigger than one:
            // write a config file that describes the processing parameters
            // and some meta data corresponding to a RRF-File
            if (m_accessLevel >= CameraAccessLevel::L2)
            {
                // The File to store the processing parameters
                const std::string paramPath = m_saveFolder.toStdString() + "/" + fileBaseName.arg ("").arg ("cfg").toStdString();
                std::ofstream oFile (paramPath.c_str());

                oFile << "# This file is generated by the royale viewer and contains the configuration of a captured frame. " << std::endl;
                oFile << "# It can be used to simply store the information to a captured frame                              " << std::endl;
                oFile << "# or to reload the configuration to another capture.                                              " << std::endl;
                oFile << "# To reload the configuration simply drag and drop this file into the royale viewer.              " << std::endl;
                oFile << "#                                                                                                 " << std::endl;
                oFile << "# If you want to edit this file manually you should note the syntax.                              " << std::endl;
                oFile << "# Every stream described by this file is separated thou a line like [100]                         " << std::endl;
                oFile << "# where the number between the []-Brackets is the stream id.                                      " << std::endl;
                oFile << "# The following key value pairs until the next stream are associated with the stream id.          " << std::endl;
                oFile << "#                                                                                                 " << std::endl;
                oFile << "# Lines which start with a # are ignored by the parser.                                           " << std::endl;
                oFile << "#                                                                                                 " << std::endl;
                oFile << "# The regex describing the stream sections is: ^(?!#)\\s*\\[([0-9]+)\\]\\s*([^\\[]*)              " << std::endl;
                oFile << "# The regex describing the key values is:     ^(?!#)\\s*(\\S+)\\s*=\\s*(\\S+)                     " << std::endl;
                oFile << "#                                                                                                 " << std::endl;
                oFile << std::endl;
                oFile << std::endl;

                for (size_t i = 0; i < m_streamIds.size(); i++)
                {
                    royale::ProcessingParameterVector params;

                    if (m_cameraDevice->getProcessingParameters (params, m_streamIds[i]) == royale::CameraStatus::SUCCESS)
                    {
                        // write the stream id
                        // this line is used to separate the processing parameters for every stream stored in this file
                        //
                        oFile << '[' << i << ']' << std::endl;
                        oFile << std::endl;

                        // write the Frame ID
                        if (m_replayControl)
                        {
                            auto frameNumber = m_replayControl->currentFrame();

                            oFile << "FrameID = " << frameNumber + 1 << std::endl;
                            oFile << std::endl;
                        }

                        // write the Color mapping
                        oFile << "# Color Mapping (Meter):" << std::endl;
                        oFile << "ColorMappingRed  = " << m_colorHelper[m_streamIdMap[m_streamIds[i]]]->getMinDist() << std::endl;
                        oFile << "ColorMappingBlue = " << m_colorHelper[m_streamIdMap[m_streamIds[i]]]->getMaxDist() << std::endl;
                        oFile << std::endl;

                        // write the Amplitude/IR (Color)
                        oFile << "# Amplitude/IR (Color):" << std::endl;
                        oFile << "AmplitudeIRColorMin = " << m_colorHelper[m_streamIdMap[m_streamIds[i]]]->getMinVal() << std::endl;
                        oFile << "AmplitudeIRColorMax = " << m_colorHelper[m_streamIdMap[m_streamIds[i]]]->getMaxVal() << std::endl;
                        oFile << std::endl;

                        // write all processing parameters
                        oFile << "# Processing parameters:" << std::endl;

                        for (auto it : params)
                        {
                            // write a single parameter
                            oFile << std::setw (29) << std::left << royale::getProcessingFlagName (it.first).c_str() << "= ";
                            switch (it.second.variantType())
                            {
                                case VariantType::Int:
                                    oFile << it.second.getInt();
                                    break;
                                case VariantType::Float:
                                    oFile << it.second.getFloat();
                                    break;
                                case VariantType::Bool:
                                    if (it.second.getBool())
                                    {
                                        oFile << "True";
                                    }
                                    else
                                    {
                                        oFile << "False";
                                    }
                                    break;
                            }
                            oFile << std::endl;
                        }
                    }
                    else
                    {
                        widgets.helpView->addLogMessage ("Failed to receive the Processing Parameters from stream " + QString::number (i));
                    }
                }
            }

            for (auto i = 0u; i < m_streamIds.size(); ++i)
            {
                const std::string plyPath = m_saveFolder.toStdString() + "/" + fileBaseName.arg (QString::number (i)).arg ("ply").toStdString();
                PLYWriter::writePLY (plyPath, &m_currentData[m_streamIds[i]]);
                const std::string imagePath = m_saveFolder.toStdString() + "/" + fileBaseName.arg (QString::number (i)).arg ("png").toStdString();
                widgets.helpView->addLogMessage ("Output in " + QString (plyPath.c_str()));
                widgets.helpView->addLogMessage ("Output in " + QString (imagePath.c_str()));

                if (m_2d)
                {
                    const QImage &currentImage = m_2dViews[i]->currentImage();
                    currentImage.scaled (m_2dViews[i]->size(), Qt::KeepAspectRatio, Qt::FastTransformation).save (imagePath.c_str());
                }
                else
                {
                    m_3dViews[i]->grabFramebuffer().save (imagePath.c_str());
                }
            }
        }
        else
        {
            widgets.helpView->addLogMessage ("Couldn't start recording, start camera first!");
        }
        recButton->setChecked (false);
    }
    else
    {
        if (m_cameraDevice)
        {
            if (m_isRecording)
            {
                stopRecording();
            }
            else
            {
                QString fileBaseName = QString ("royale_%1_%2.%3").arg (curDate).arg (curTime);
                QString filename = fileBaseName.arg (extension);
                std::string filepath = m_saveFolder.toStdString() + "/" + filename.toStdString();
                if (m_cameraDevice->startRecording (filepath) == royale::CameraStatus::SUCCESS)
                {
                    widgets.helpView->addLogMessage ("Raw recorder started.");
                    widgets.helpView->addLogMessage ("Output in " + QString (filepath.c_str()));
                    m_isRecording = true;
                    recButton->setChecked (true);
                }
                else
                {
                    widgets.helpView->addLogMessage ("Couldn't start recording!");
                    recButton->setChecked (false);
                }
            }
        }
        else
        {
            widgets.helpView->addLogMessage ("Couldn't start recording, start camera first!");
            recButton->setChecked (false);
        }
    }
}

void QTViewer::stopRecording()
{
    if (m_cameraDevice->stopRecording() == royale::CameraStatus::SUCCESS)
    {
        widgets.helpView->addLogMessage ("Raw recorder stopped.");
        m_isRecording = false;
        recButton->setChecked (false);
    }
    else
    {
        widgets.helpView->addLogMessage ("Couldn't stop recording!");
        m_isRecording = false;
        recButton->setChecked (true);
    }
}

void QTViewer::updateFPS()
{
    for (auto i = 0; i < (int) m_streamIds.size(); ++i)
    {
        m_fpsLabel[i]->setText (QString::number (m_frameCounter[m_streamIds[i]]) + QString (" fps"));
        m_frameCounter[m_streamIds[i]] = 0;
    }

    if (m_isConnected &&
            m_accessLevel >= CameraAccessLevel::L2)
    {
        setWindowTitle (m_appName + "   " + QString::number (m_currentTemperature, 'f', 2) + QChar (0xb0) + " C");
    }
}

void QTViewer::updateStreamId()
{
    for (auto i = 0; i < (int) m_streamIds.size(); ++i)
    {
        m_streamIdLabel[i]->setText (QString ("StreamID: ") + QString::number (m_streamIds[i]));
    }
}

void QTViewer::updateValidPixelsNumber ()
{
    m_updateValidPixelsNumber = true;
    for (auto i = 0; i < (int) m_streamIds.size(); ++i)
    {
        m_validPixelsNumberLabel[i]->setText (QString ("Number of valid Pixels: ") + QString::number (m_validPixelsNumber[m_streamIds[i]]));
    }
}

void QTViewer::countValidPixelsNumber (const royale::DepthData *data, royale::StreamId streamId)
{
    m_validPixelsNumber[streamId] = 0;
    for (size_t i = 0; i < data->points.size(); ++i)
    {
        auto curPoint = data->points.at (i);
        if (curPoint.z < m_rangeFilterSettings[streamId].filterMin || curPoint.z > m_rangeFilterSettings[streamId].filterMax)
        {
            curPoint.depthConfidence = 0;
        }
        if (curPoint.depthConfidence > 0)
        {
            m_validPixelsNumber[streamId]++;
        }
    }
    m_updateValidPixelsNumber = false;
}

void QTViewer::distColorRangeChanged (float minValue, float maxValue)
{
    auto curStreamId = widgets.colorRangeView->getCurrentStreamId();
    if (curStreamId == 0)
    {
        m_initSettings.hasInit = true;
        m_initSettings.initMinDist = minValue;
        m_initSettings.initMaxDist = maxValue;
    }
    else
    {
        m_colorHelper[m_streamIdMap[curStreamId]]->setMinDist (minValue);
        m_colorHelper[m_streamIdMap[curStreamId]]->setMaxDist (maxValue);

        for (auto i = 0u; i < m_streamIds.size(); ++i)
        {
            if (curStreamId == m_streamIds[i])
            {
                if (m_2d)
                {
                    m_2dViews[i]->colorRangeChanged();
                }
                else
                {
                    m_3dViews[i]->colorRangeChanged();
                }
                break;
            }
        }
    }
}

void QTViewer::grayColorRangeChanged (int minValue, int maxValue)
{
    auto curStreamId = widgets.colorRangeView->getCurrentStreamId();
    if (curStreamId == 0)
    {
        m_initSettings.hasInit = true;
        m_initSettings.initMinVal = (uint16_t) minValue;
        m_initSettings.initMaxVal = (uint16_t) maxValue;
    }
    else
    {
        m_colorHelper[m_streamIdMap[curStreamId]]->setMinVal ( (uint16_t) minValue);
        m_colorHelper[m_streamIdMap[curStreamId]]->setMaxVal ( (uint16_t) maxValue);
        for (auto i = 0u; i < m_streamIds.size(); ++i)
        {
            if (curStreamId == m_streamIds[i])
            {
                if (m_2d)
                {
                    m_2dViews[i]->colorRangeChanged();
                }
                else
                {
                    m_3dViews[i]->colorRangeChanged();
                }
                break;
            }
        }
    }
}

void QTViewer::autoColorRangeUpdate()
{
    QMutexLocker locker (m_dataMutex);
    auto curStreamId = widgets.colorRangeView->getCurrentStreamId();
    if (m_hasData[curStreamId])
    {
        findMinMax (&m_currentData[curStreamId], curStreamId);
    }
}

void QTViewer::setCameraPositionChanged (const QMatrix4x4 &modelViewMatrix, const QMatrix4x4 &rotationMatrix)
{
    ThreeDView *activeView = qobject_cast<ThreeDView *> (sender());
    for (auto i = 0u; i < m_streamIds.size(); ++i)
    {
        if (activeView != m_3dViews[i])
        {
            m_3dViews[i]->setCameraPosition (modelViewMatrix, rotationMatrix);
        }
    }
}

void QTViewer::setAutoRotationStatusChanged (bool isRotating, bool lockView, bool pause)
{
    ThreeDView *activeView = qobject_cast<ThreeDView *> (sender());
    for (auto i = 0u; i < m_streamIds.size(); ++i)
    {
        if (lockView)
        {
            if (activeView != m_3dViews[i])
            {
                m_3dViews[i]->setFollowRotationStatus (isRotating);
            }
            if (!pause)
            {
                m_autoRotationSettings[m_streamIds[i]].autoRotation = isRotating;
                streamIdAutoRotationViewChanged (m_streamIds[i]);
            }
        }
        else
        {
            if (activeView == m_3dViews[i])
            {
                m_autoRotationSettings[m_streamIds[i]].autoRotation = isRotating;
                streamIdAutoRotationViewChanged (m_streamIds[i]);
                break;
            }
        }
    }
}

void QTViewer::syncRotatingSpeedChanged (float speed)
{
    ThreeDView *activeView = qobject_cast<ThreeDView *> (sender());
    for (auto i = 0u; i < m_streamIds.size(); ++i)
    {
        if (activeView != m_3dViews[i])
        {
            m_3dViews[i]->setRotatingSpeed (speed, false);
            m_autoRotationSettings[m_streamIds[i]].speed = speed;
            streamIdAutoRotationViewChanged (m_streamIds[i]);
        }
    }
}

void QTViewer::enableSingleFrameRecording (bool enabled)
{
    if (enabled == true && m_cameraDevice && m_isRecording)
    {
        stopRecording();
    }
}

void QTViewer::openPlaybackFile (const std::string &filename)
{
    if (widgets.currentTool)
    {
        widgets.currentTool->hide();
    }

    widgets.helpView->addLogMessage ("Trying to load : " + QString::fromStdString (filename));
    if (m_isConnected)
    {
        // We have to call the function directly in this case, as the
        // button is disabled in playback mode and no slots will be called in this case
        closeCameraOrRecording();
        cameraButton->setChecked (false);
    }
    m_cameraDevice.reset (nullptr);
    m_playbackFilename = filename;
    if (m_toolsPanel && m_toolsPanel->isVisible())
    {
        toolsButton->click();
    }
    if (m_helpPanel && m_helpPanel->isVisible())
    {
        helpButton->click();
    }

    openCameraOrRecording();
    cameraButton->setChecked (true);

    if (m_isConnected)
    {
        // Deactivate this until the controls work properly
        movieControl->show();
        m_replayControl = dynamic_cast<IReplay *> (m_cameraDevice.get());
        widgets.playerControl->reset();
        if (m_replayControl)
        {
            widgets.playerControl->setFrameCount (m_replayControl->frameCount ());
            widgets.helpView->addLogMessage ("Successfully loaded : " + QString::fromStdString (filename));
            widgets.helpView->addLogMessage ("File version : " + QString::number (m_replayControl->getFileVersion()));
        }
    }
    else
    {
        widgets.helpView->addLogMessage ("Could not load : " + QString::fromStdString (filename));
        m_playbackFilename.clear();
    }
    QDir currentDir;
    m_lastLoadFolder = currentDir.absoluteFilePath (QString::fromStdString (filename));
}

void QTViewer::dragEnterEvent (QDragEnterEvent *event)
{
    QString localFile;

    for (QUrl url : event->mimeData()->urls())
    {
        localFile = url.toLocalFile();

        if (localFile.endsWith (".rrf") | localFile.endsWith (".cfg"))
        {
            event->acceptProposedAction();
            return;
        }
    }
}

inline bool endsWith (const royale::String &value, const royale::String &ending)
{
    if (ending.size() > value.size())
    {
        return false;
    }
    return value.compare (value.length() - ending.length(), ending.length(), ending) == 0;
}

void QTViewer::dropEvent (QDropEvent *event)
{
    if (event->mimeData()->urls().count() > 1)
    {
        widgets.helpView->addLogMessage ("Notice you only can load One file at once!\n Only the first RRF-File or CFG-File will be loaded");
    }

    QString localFile;
    bool notice = false;

    for (auto url : event->mimeData()->urls())
    {
        localFile = url.toLocalFile();

        if (localFile.endsWith (".rrf"))
        {
            this->openPlaybackFile (localFile.toLocal8Bit().constData());
            event->acceptProposedAction();
        }
        else if (localFile.endsWith (".cfg"))
        {
            if (m_accessLevel >= CameraAccessLevel::L2)
            {
                std::ifstream ifs (localFile.toStdString());
                std::string configString ( (std::istreambuf_iterator<char> (ifs)),
                                           (std::istreambuf_iterator<char>()));

                std::regex streamRegex ("^(?!#)\\s*\\[([0-9]+)\\]\\s*([^\\[]*)");
                std::regex keyValueRegex ("^(?!#)\\s*(\\S+)\\s*=\\s*(\\S+)\\s*");

                std::smatch streamMatcher;
                while (std::regex_search (configString, streamMatcher, streamRegex))
                {
                    widgets.helpView->addLogMessage (localFile);

                    std::string streamIDString = streamMatcher[1];
                    std::string streamConfigString = streamMatcher[2];

                    royale::ProcessingParameterVector params;
                    auto streamID = static_cast<uint16_t> (atoi (streamIDString.c_str()));

                    std::smatch keyValueMatcher;
                    while (std::regex_search (streamConfigString, keyValueMatcher, keyValueRegex))
                    {
                        std::string keyString = keyValueMatcher[1];
                        std::string valueString = keyValueMatcher[2];

                        royale::String key = keyString;

                        try
                        {

                            if (keyString.compare ("ColorMappingRed") == 0)
                            {
                                auto value = static_cast<float> (atof (valueString.c_str()));
                                m_colorHelper[m_streamIdMap[m_streamIds[streamID]]]->setMinDist (value);
                            }
                            else if (keyString.compare ("ColorMappingBlue") == 0)
                            {
                                auto value = static_cast<float> (atof (valueString.c_str()));
                                m_colorHelper[m_streamIdMap[m_streamIds[streamID]]]->setMaxDist (value);
                            }
                            else if (keyString.compare ("AmplitudeIRColorMin") == 0)
                            {
                                auto value = static_cast<uint16_t> (atoi (valueString.c_str()));
                                m_colorHelper[m_streamIdMap[m_streamIds[streamID]]]->setMinVal (value);
                            }
                            else if (keyString.compare ("AmplitudeIRColorMax") == 0)
                            {
                                auto value = static_cast<uint16_t> (atoi (valueString.c_str()));
                                m_colorHelper[m_streamIdMap[m_streamIds[streamID]]]->setMaxVal (value);
                            }
                            else
                            {
                                royale::ProcessingFlag processingFlag;

                                if (royale::parseProcessingFlagName (key, processingFlag))
                                {

                                    if (endsWith (key, "_Bool"))
                                    {
                                        bool variantValue = valueString.compare ("True") == 0;
                                        royale::Variant variant (variantValue);

                                        std::pair<royale::ProcessingFlag, royale::Variant> processingParameter (processingFlag, variant);
                                        params.push_back (processingParameter);
                                    }

                                    else if (endsWith (key, "_Int"))
                                    {
                                        int variantValue = atoi (valueString.c_str());
                                        royale::Variant variant (variantValue);

                                        std::pair<royale::ProcessingFlag, royale::Variant> processingParameter (processingFlag, variant);
                                        params.push_back (processingParameter);
                                    }

                                    else if (endsWith (key, "_Float"))
                                    {
                                        auto variantValue = static_cast<float> (atof (valueString.c_str()));
                                        royale::Variant variant (variantValue);

                                        std::pair<royale::ProcessingFlag, royale::Variant> processingParameter (processingFlag, variant);
                                        params.push_back (processingParameter);
                                    }
                                }
                            }
                        }
                        catch (...)
                        {
                            widgets.helpView->addLogMessage ("config parameter can not be parsed");
                        }

                        streamConfigString = keyValueMatcher.suffix().str();
                    }

                    m_cameraDevice->setProcessingParameters (params, streamID);
                    widgets.parameterView->setCurrentParameters (params);
                    widgets.parameterView->parameterChanged();

                    configString = streamMatcher.suffix().str();
                }

                event->acceptProposedAction();
                repeat();
            }
        }
        else
        {
            if (!notice)
            {
                widgets.helpView->addLogMessage ("Notice you only can load RRF-Files or CFG-Files!");
                notice = true;
            }
            widgets.helpView->addLogMessage ("Could not load : " + localFile);
        }
    }
}

void QTViewer::exposureChanged (int newExposure)
{
    if (m_cameraDevice)
    {
        auto curStreamId = widgets.exposureView->getCurrentStreamId();
        widgets.exposureView->blockSlider (true);
        CameraStatus ret = CameraStatus::DATA_NOT_FOUND;

        // If the device is busy we try to set the exposure "tries" times
        int tries = 5;
        do
        {
            ret = m_cameraDevice->setExposureTime (newExposure, curStreamId);
            if (ret == CameraStatus::DEVICE_IS_BUSY)
            {
                QThread::msleep (10);
                widgets.helpView->addLogMessage ("Error changing the exposure time! Device is busy. Retrying.");
                tries--;
            }
        }
        while (tries > 0 && ret == CameraStatus::DEVICE_IS_BUSY);

        if (ret == CameraStatus::SUCCESS)
        {
            m_currentExposureTime[curStreamId] = newExposure;
        }
        else
        {
            widgets.exposureView->valueChanged (m_currentExposureTime[curStreamId]);
            widgets.helpView->addLogMessage (QString ("Error changing the exposure time! Error : ") + QString (getErrorString (ret).c_str()));
        }
        widgets.exposureView->blockSlider (false);
    }
}

void QTViewer::framerateChanged (int newFramerate)
{
    if (m_cameraDevice)
    {
        auto rate = static_cast<uint16_t> (newFramerate);
        if (static_cast<int> (rate) != newFramerate)
        {
            // this narrow_cast failure was unexpected, but just return
            return;
        }

        auto ret = m_cameraDevice->setFrameRate (rate);
        if (ret == CameraStatus::SUCCESS)
        {
            widgets.framerateView->valueChanged (newFramerate);
        }
    }
}

void QTViewer::enableFPSDisplay (bool enabled)
{
    m_showFPS = enabled;
    for (auto i = 0; i < (int) m_streamIds.size(); ++i)
    {
        if (m_fpsLabel[i])
        {
            m_fpsLabel[i]->setVisible (m_showFPS);
        }
    }
}

void QTViewer::enableStreamIdDisplay (bool enabled)
{
    m_showStreamId = enabled;
    for (auto i = 0; i < (int) m_streamIds.size(); ++i)
    {
        if (m_streamIdLabel[i])
        {
            m_streamIdLabel[i]->setVisible (m_showStreamId);
        }
    }
}

void QTViewer::enableValidPixelsNumber (bool enabled)
{
    m_showValidPixelsNumber = enabled;
    for (auto i = 0; i < (int) m_streamIds.size(); ++i)
    {
        if (m_validPixelsNumberLabel[i])
        {
            m_validPixelsNumberLabel[i]->setVisible (m_showValidPixelsNumber);
        }
        if (m_validPixelsNumberLabel[i]->isVisible() && !m_cameraDevice)
        {
            QMutexLocker locker (m_dataMutex);
            if (m_hasData[m_streamIds[i]])
            {
                countValidPixelsNumber (&m_currentData[m_streamIds[i]], m_streamIds[i]);
            }
        }
    }
}

void QTViewer::autoRotationEnabled (bool enabled)
{
    auto curStreamId = widgets.autoRotationView->getCurrentStreamId();
    if (curStreamId == 0)
    {
        m_initSettings.hasInit = true;
        m_initSettings.initAutoRotation = true;
    }
    else
    {
        for (auto i = 0u; i < m_streamIds.size(); ++i)
        {
            if (curStreamId == m_streamIds[i])
            {
                m_3dViews[i]->setAutoRotationStatus (enabled);
                break;
            }
        }
    }
}

void QTViewer::autoExposureEnabled (bool enabled)
{
    if (m_cameraDevice)
    {
        auto curStreamId = widgets.exposureView->getCurrentStreamId();
        if (enabled)
        {
            auto ret = m_cameraDevice->setExposureMode (ExposureMode::AUTOMATIC, curStreamId);
            widgets.exposureView->autoExposureChanged (ret == CameraStatus::SUCCESS);
        }
        else
        {
            auto ret = m_cameraDevice->setExposureMode (ExposureMode::MANUAL, curStreamId);
            widgets.exposureView->autoExposureChanged (! (ret == CameraStatus::SUCCESS));
        }

        if (m_autoExposure)
        {
            for (auto i = 0u; i < m_streamIds.size(); ++i)
            {
                if (curStreamId == m_streamIds[i])
                {
                    m_cameraDevice->getExposureMode (m_exposureModes[i], curStreamId);
                    break;
                }
            }
        }
    }
}

void QTViewer::onNewExposure (const uint32_t exposureTime, const royale::StreamId streamId)
{
    m_currentExposureTime[streamId] = exposureTime;

    if (widgets.exposureView->getCurrentStreamId() == streamId)
    {
        widgets.exposureView->valueChanged (m_currentExposureTime[streamId]);
    }
}

void QTViewer::rewind()
{
    if (!m_replayControl)
    {
        return;
    }

    auto frameNumber = m_replayControl->currentFrame();
    const auto numFrames = m_replayControl->frameCount();

    if (frameNumber == 0u)
    {
        // Seek is zero-based, frameCount is not
        frameNumber = numFrames - 1;
    }
    else
    {
        auto skip = uint32_t{ 1 };

        if (m_isPlaybackActive)
        {
            // If the playback is already running than rewind
            // by 10% of all frames
            skip = std::max (3u, numFrames / 10);
        }

        // Seek to the first frame if skip is too large
        frameNumber -= std::min (frameNumber, skip);
    }
    m_replayControl->seek (frameNumber);
}

void QTViewer::repeat()
{
    if (!m_replayControl || m_isPlaybackActive)
    {
        return;
    }

    m_replayControl->seek (m_replayControl->currentFrame());

}

void QTViewer::playbackStopPlay (bool play)
{
    if (!m_replayControl)
    {
        return;
    }

    m_isPlaybackActive = play;

    if (play)
    {
        m_replayControl->resume();
    }
    else
    {
        m_replayControl->pause();
    }
}

void QTViewer::forward()
{
    if (!m_replayControl)
    {
        return;
    }

    auto frameNumber = m_replayControl->currentFrame();
    const auto numFrames = m_replayControl->frameCount();

    if (frameNumber == numFrames - 1)
    {
        frameNumber = 0u;
    }
    else
    {
        auto skip = uint32_t{ 1 };

        if (m_isPlaybackActive)
        {
            // If the playback is already running than forward
            // by 10% of all frames
            skip = std::max (3u, numFrames / 10);
        }

        if (frameNumber + skip >= numFrames)
        {
            // Seek is zero-based, frameCount is not
            frameNumber = numFrames - 1;
        }
        else
        {
            frameNumber += skip;
        }
    }
    m_replayControl->seek (frameNumber);
}

void QTViewer::seekTo (int frame)
{
    if (!m_replayControl)
    {
        return;
    }

    if (m_isPlaybackActive)
    {
        m_replayControl->pause();
        m_replayControl->seek (frame);
        m_replayControl->resume();
    }
    else
    {
        m_replayControl->seek (frame);
    }
}


void QTViewer::onEvent (std::unique_ptr<royale::IEvent> &&event)
{
    if (event->severity() >= royale::EventSeverity::ROYALE_WARNING)
    {
        auto m = event->describe();
        auto msg = m.toStdString();
        emit logMessage (QString::fromStdString (msg));

        if (event->type() == royale::EventType::ROYALE_DEVICE_DISCONNECTED ||
                event->type() == royale::EventType::ROYALE_EYE_SAFETY)
        {
            cameraButton->clicked();
        }
    }
}

void QTViewer::changePipelineParameter (Pair<ProcessingFlag, Variant> parameter)
{
    if (m_cameraDevice)
    {
        auto curStreamId = widgets.parameterView->getCurrentStreamId();

        // Set the new parameter value
        ProcessingParameterVector vec;
        vec.push_back (parameter);
        auto ret = m_cameraDevice->setProcessingParameters (vec, curStreamId);

        if (ret != CameraStatus::SUCCESS)
        {
            // In case something went wrong we have to reset the GUI

            // Retrieve the current parameters
            ProcessingParameterVector vecCurrent;
            m_cameraDevice->getProcessingParameters (vecCurrent, curStreamId);

            widgets.helpView->addLogMessage (QString ("Couldn't set parameter : " +
                                             QString::fromStdString (getProcessingFlagName (parameter.first).toStdString())));
            for (auto curParam : vecCurrent)
            {
                if (curParam.first == parameter.first)
                {
                    widgets.parameterView->setValue (curParam);
                    return;
                }
            }
        }
    }
}

void QTViewer::hideButtons (bool hide)
{
    toolsButton->setHidden (hide);
    forceColorButton->setHidden (hide);
    viewButton->setHidden (hide);
    helpButton->setHidden (hide);
    recButton->setHidden (hide);
    cameraButton->setHidden (hide);
    if (!m_playbackFilename.empty())
    {
        movieControl->setHidden (hide);
    }
    m_toolsPanel->hide();
    toolsButton->setChecked (false);
    m_helpPanel->hide();
    helpButton->setChecked (false);
    m_licensePanel->hide();
    if (widgets.currentTool && widgets.currentTool->isVisible())
    {
        widgets.currentTool->hide();
    }
    m_buttonsHidden = hide;
}


bool QTViewer::eventFilter (QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        m_toolsPanel->hide();
        toolsButton->setChecked (false);
        if (widgets.currentTool)
        {
            widgets.currentTool->hide();
        }
        m_helpPanel->hide();
        helpButton->setChecked (false);
        return false;
    }
    else
    {
        return QObject::eventFilter (obj, event);
    }
}

void QTViewer::setCurrentView (bool twoD)
{
    if (!twoD)
    {
        if (m_streamIds.size())
        {
            for (auto i = 0u; i < m_streamIds.size(); ++i)
            {
                m_2dViews[i]->hide();
                m_3dViews[i]->show();
                if (m_hasData[m_streamIds[i]])
                {
                    m_3dViews[i]->onNewData (&m_currentData[m_streamIds[i]],
                                             &m_currentIntermediateData[m_streamIds[i]]);
                }
            }
            for (auto i = 0; i < m_maxStreams; ++i)
            {
                m_fpsLabel[i]->setParent (m_3dViews[i]);
                m_streamIdLabel[i]->setParent (m_3dViews[i]);
                m_validPixelsNumberLabel[i]->setParent (m_3dViews[i]);
            }
        }
        else
        {
            // we don't have any streams ...
        }
    }
    else
    {
        if (m_streamIds.size())
        {
            for (auto i = 0u; i < m_streamIds.size(); ++i)
            {
                m_3dViews[i]->hide();
                m_2dViews[i]->show();
                if (m_hasData[m_streamIds[i]])
                {
                    m_2dViews[i]->onNewData (&m_currentData[m_streamIds[i]],
                                             &m_currentIntermediateData[m_streamIds[i]]);
                }
            }
            for (auto i = 0; i < m_maxStreams; ++i)
            {
                m_fpsLabel[i]->setParent (m_2dViews[i]);
                m_streamIdLabel[i]->setParent (m_2dViews[i]);
                m_validPixelsNumberLabel[i]->setParent (m_2dViews[i]);
            }
        }
        else
        {
            // we don't have any streams ...
        }
    }

    widgets.dataSelectorView->displayUniformMode (!twoD);

    m_2d = twoD;
    if (m_2d)
    {
        viewButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-3d-default"));
        viewButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-3d-pressed"));
    }
    else
    {
        viewButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-2d-default"));
        viewButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-2d-pressed"));
    }

    if (widgets.currentTool)
    {
        widgets.currentTool->hide();
    }

    widgets.toolsMenu->updateToolEntries (m_isConnected, m_2d, m_modeMixed, !m_playbackFilename.empty());
    layoutSubviews();
}

void QTViewer::fillStreamView()
{
    m_streamIdMap.clear();
    m_streamIds.clear();

    royale::Vector<royale::StreamId> tempStreamIds;
    auto status = m_cameraDevice->getStreams (tempStreamIds);
    if (status != CameraStatus::SUCCESS)
    {
        widgets.helpView->addLogMessage ("Unable to retrieve streams");
        return;
    }
    if (tempStreamIds.size() > m_maxStreams)
    {
        auto message = QString ("This use case has ")
                       + QString::number (tempStreamIds.size())
                       + QString (" streams, but RoyaleViewer is compiled for a maximum of ")
                       + QString::number (m_maxStreams)
                       + QString (" streams.");
        widgets.helpView->addLogMessage (std::move (message));
        return;
    }

    m_streamIds = std::move (tempStreamIds);
    m_firstData.clear();
    int curIdx = 0;
    for (auto &stream : m_streamIds)
    {
        m_streamIdMap[stream] = curIdx;
        m_firstData[stream] = true;
        m_frameCounter[stream] = 0;

        ++curIdx;
    }
    updateStreamId();
    if (m_streamIds.size() <= 1)
    {
        m_modeMixed = false;
    }
    else
    {
        m_modeMixed = true;
    }

    if (widgets.currentTool && widgets.currentTool->isVisible())
    {
        widgets.currentTool->hide();
    }

    widgets.toolsMenu->updateToolEntries (m_isConnected, m_2d, m_modeMixed, !m_playbackFilename.empty());

    layoutSubviews();

    if (m_autoExposure)
    {
        for (auto i = 0u; i < m_streamIds.size(); ++i)
        {
            m_cameraDevice->setExposureMode (m_exposureModes[i], m_streamIds[i]);
        }
    }

    uint16_t framerateMax;
    if (m_cameraDevice->getMaxFrameRate (framerateMax) == CameraStatus::SUCCESS)
    {
        emit framerateLimitsChanged (1, framerateMax);
    }

    uint16_t framerateCurr;
    if (m_cameraDevice->getFrameRate (framerateCurr) == CameraStatus::SUCCESS)
    {
        emit framerateValue (framerateCurr);
    }


    if (m_initSettings.hasInit)
    {
        initSettings();
    }
}

void QTViewer::initSettings()
{
    for (auto i = 0u; i < m_streamIds.size(); ++i)
    {
        if (m_initSettings.initAutoRotation)
        {
            m_3dViews[i]->setAutoRotationStatus (m_initSettings.initAutoRotation);
        }
        if (m_initSettings.initCenter != 2.0f)
        {
            m_autoRotationSettings[m_streamIds[i]].center = m_initSettings.initCenter;
            m_3dViews[i]->setRotatingCenter (m_initSettings.initCenter);
        }
        if (m_initSettings.initSpeed != 0.01f)
        {
            m_autoRotationSettings[m_streamIds[i]].speed = m_initSettings.initSpeed;
            m_3dViews[i]->setRotatingSpeed (m_initSettings.initSpeed, true);
        }
        if (m_initSettings.initFilterMin != 0.0f || m_initSettings.initFilterMax != 7.5f)
        {
            m_rangeFilterSettings[m_streamIds[i]].filterMin = m_initSettings.initFilterMin;
            m_rangeFilterSettings[m_streamIds[i]].filterMax = m_initSettings.initFilterMax;
            m_2dViews[i]->setFilterMinMax (m_rangeFilterSettings[m_streamIds[i]].filterMin, m_rangeFilterSettings[m_streamIds[i]].filterMax, m_isConnected);
            m_3dViews[i]->setFilterMinMax (m_rangeFilterSettings[m_streamIds[i]].filterMin, m_rangeFilterSettings[m_streamIds[i]].filterMax, m_isConnected);
        }
        if (m_initSettings.initMinDist != 0.1f || m_initSettings.initMaxDist != 1.8f)
        {
            m_colorHelper[i]->setMinDist (m_initSettings.initMinDist);
            m_colorHelper[i]->setMaxDist (m_initSettings.initMaxDist);
            if (m_2d)
            {
                m_2dViews[i]->colorRangeChanged();
            }
            else
            {
                m_3dViews[i]->colorRangeChanged();
            }
        }
        if (m_initSettings.initMinVal != 1 || m_initSettings.initMaxVal != 100)
        {
            m_colorHelper[i]->setMinVal (m_initSettings.initMinVal);
            m_colorHelper[i]->setMaxVal (m_initSettings.initMaxVal);
            if (m_2d)
            {
                m_2dViews[i]->colorRangeChanged();
            }
            else
            {
                m_3dViews[i]->colorRangeChanged();
            }
        }
    }
    m_initSettings.hasInit = false;
}

void QTViewer::streamIdParameterViewChanged (royale::StreamId streamId)
{
    widgets.parameterView->reset();
    widgets.parameterView->setCurrentStreamId (streamId);
    if (m_cameraDevice)
    {
        ProcessingParameterVector parameters;
        if (m_cameraDevice->getProcessingParameters (parameters, streamId) == CameraStatus::SUCCESS)
        {
            widgets.parameterView->setCurrentParameters (parameters);
        }
    }
}

void QTViewer::streamIdExposureViewChanged (royale::StreamId streamId)
{
    royale::Pair<uint32_t, uint32_t> limits;
    if (m_cameraDevice->getExposureLimits (limits, streamId) != CameraStatus::SUCCESS)
    {
        // Couldn't retrieve exposure limits
    }
    widgets.exposureView->minMaxChanged (static_cast<int> (limits.first), static_cast<int> (limits.second));
    widgets.exposureView->valueChanged (m_currentExposureTime[streamId]);
    widgets.exposureView->setCurrentStreamId (streamId);
    bool autoExposureOn = true;
    royale::ExposureMode expoMode = ExposureMode::MANUAL;

    if (m_cameraDevice->getExposureMode (expoMode, streamId) != CameraStatus::SUCCESS)
    {
        // Couldn't retrieve current exposure mode
    }

    if (expoMode == ExposureMode::MANUAL)
    {
        autoExposureOn = false;
    }
    widgets.exposureView->autoExposureChanged (autoExposureOn);

    if (!m_playbackFilename.empty())
    {
        // We're currently in playback mode
        widgets.exposureView->setEnabled (false);
    }
    else
    {
        // We're currently in live mode
        widgets.exposureView->setEnabled (true);
        widgets.exposureView->enableAutoExposure (true);
    }
}

void QTViewer::streamIdColorRangeViewChanged (royale::StreamId streamId)
{
    widgets.colorRangeView->setCurrentStreamId (streamId);

    widgets.colorRangeView->setDistRange (m_colorHelper[m_streamIdMap[streamId]]->getMinDist(),
                                          m_colorHelper[m_streamIdMap[streamId]]->getMaxDist());
    widgets.colorRangeView->setGrayRange (m_colorHelper[m_streamIdMap[streamId]]->getMinVal(),
                                          m_colorHelper[m_streamIdMap[streamId]]->getMaxVal());
}

void QTViewer::streamIdFilterRangeViewChanged (royale::StreamId streamId)
{
    widgets.filterMinMaxView->setCurrentStreamId (streamId);
    widgets.filterMinMaxView->valuesChanged (static_cast<int> (m_rangeFilterSettings[streamId].filterMin * 100.0f),
            static_cast<int> (m_rangeFilterSettings[streamId].filterMax * 100.0f));
}

void QTViewer::streamIdAutoRotationViewChanged (royale::StreamId streamId)
{
    widgets.autoRotationView->setCurrentStreamId (streamId);
    widgets.autoRotationView->autoRotationChanged (m_autoRotationSettings[streamId].autoRotation);
    widgets.autoRotationView->setRSpeedSlider (static_cast<int> (m_autoRotationSettings[streamId].speed * 1000.f));
    widgets.autoRotationView->setRCenterSlider (static_cast<int> (m_autoRotationSettings[streamId].center * 100.f));
    if (m_streamIdInMenu != streamId)
    {
        streamIdAutoRotationViewChanged (m_streamIdInMenu);
    }
}

void  QTViewer::setStreamIdChanged (royale::StreamId streamId)
{
    m_streamIdInMenu = streamId;
}

void QTViewer::newLogMessageAvailable()
{
    if (!m_helpPanel->isVisible())
    {
        helpButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-log-notify"));
        helpButton->update();
        widgets.helpView->setLogTabChecked();
        m_logNotified = false;
    }
    else
    {
        if (widgets.helpView->getTabWidgetCurrentIndex() != 0)
        {
            helpButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-log-notify"));
            helpButton->update();
            m_logNotified = true;
        }
    }
}

void QTViewer::displayLog()
{
    closeLicense();
    m_helpPanel->toggle();
    if (m_helpPanel->isVisible())
    {
        helpButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-help-default"));
        helpButton->update();
    }
    else
    {
        if (m_logNotified)
        {
            helpButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-log-notify"));
            helpButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-help-pressed"));
            helpButton->update();
            widgets.helpView->setLogTabChecked();
            m_logNotified = false;
        }
    }
}

void QTViewer::setHelpTabBarClicked (int index)
{
    if (index == 0 && m_logNotified)
    {
        helpButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-help-pressed"));
        helpButton->update();
        m_logNotified = false;
    }
}

void QTViewer::displayLicense()
{
    if (m_toolsPanel->isVisible())
    {
        toolsButton->click();
    }
    if (m_helpPanel->isVisible())
    {
        helpButton->click();
    }
    if (!m_playbackFilename.empty())
    {
        movieControl->setHidden (true);
    }
    m_licensePanel->setFixedSize (this->size().width() - cameraButton->width() * 3, this->size().height() - cameraButton->width() / 3);
    m_licensePanel->toggle();
}

void QTViewer::closeLicense()
{
    if (m_licensePanel->isVisible())
    {
        m_licensePanel->toggle();
    }
    if (!m_playbackFilename.empty())
    {
        movieControl->setVisible (true);
    }
}

void QTViewer::checkForNewAccessCode()
{
#ifdef TARGET_PLATFORM_ANDROID
    // Check if we received a new access code
    if (!androidAccessCode.empty() &&
            androidAccessCode != m_accessCode.toStdString())
    {
        m_accessCode = androidAccessCode.c_str();
        m_accessLevel = royale::CameraManager::getAccessLevel (m_accessCode);
        if (m_accessLevel >= CameraAccessLevel::L2)
        {
            widgets.helpView->addLogMessage ("Level " + QString::number ( (uint16_t) m_accessLevel));
        }
    }
#endif
}

void QTViewer::onFilterMinChanged (const float min)
{
    auto curStreamId = widgets.filterMinMaxView->getCurrentStreamId();
    if (curStreamId == 0)
    {
        m_initSettings.hasInit = true;
        m_initSettings.initFilterMin = min;
    }
    else
    {
        m_rangeFilterSettings[curStreamId].filterMin = min;
        for (auto i = 0u; i < m_streamIds.size(); ++i)
        {
            if (curStreamId == m_streamIds[i])
            {
                m_2dViews[i]->setFilterMinMax (min, m_rangeFilterSettings[m_streamIds[i]].filterMax, m_isConnected);
                m_3dViews[i]->setFilterMinMax (min, m_rangeFilterSettings[m_streamIds[i]].filterMax, m_isConnected);

                if (m_validPixelsNumberLabel[i] && m_validPixelsNumberLabel[i]->isVisible() && !m_isConnected)
                {
                    QMutexLocker locker (m_dataMutex);
                    if (m_hasData[m_streamIds[i]])
                    {
                        countValidPixelsNumber (&m_currentData[m_streamIds[i]], m_streamIds[i]);
                    }
                }
                break;
            }
        }
    }
}

void QTViewer::onFilterMaxChanged (const float max)
{
    auto curStreamId = widgets.filterMinMaxView->getCurrentStreamId();
    if (curStreamId == 0)
    {
        m_initSettings.hasInit = true;
        m_initSettings.initFilterMax = max;
    }
    else
    {
        m_rangeFilterSettings[curStreamId].filterMax = max;
        for (auto i = 0u; i < m_streamIds.size(); ++i)
        {
            if (curStreamId == m_streamIds[i])
            {
                m_2dViews[i]->setFilterMinMax (m_rangeFilterSettings[m_streamIds[i]].filterMin, max, m_isConnected);
                m_3dViews[i]->setFilterMinMax (m_rangeFilterSettings[m_streamIds[i]].filterMin, max, m_isConnected);

                if (m_validPixelsNumberLabel[i] && m_validPixelsNumberLabel[i]->isVisible() && !m_isConnected)
                {
                    QMutexLocker locker (m_dataMutex);
                    if (m_hasData[m_streamIds[i]])
                    {
                        countValidPixelsNumber (&m_currentData[m_streamIds[i]], m_streamIds[i]);
                    }
                }
                break;
            }
        }
    }
}

void QTViewer::onRCenterChanged (const float center)
{
    auto curStreamId = widgets.autoRotationView->getCurrentStreamId();
    if (curStreamId == 0)
    {
        m_initSettings.hasInit = true;
        m_initSettings.initCenter = center;
    }
    else
    {
        m_autoRotationSettings[curStreamId].center = center;
        for (auto i = 0u; i < m_streamIds.size(); ++i)
        {
            if (curStreamId == m_streamIds[i])
            {
                m_3dViews[i]->setRotatingCenter (center);
                break;
            }
        }
    }
}

void QTViewer::onRSpeedChanged (const float speed)
{
    auto curStreamId = widgets.autoRotationView->getCurrentStreamId();
    if (curStreamId == 0)
    {
        m_initSettings.hasInit = true;
        m_initSettings.initSpeed = speed;
    }
    else
    {
        m_autoRotationSettings[curStreamId].speed = speed;
        for (auto i = 0u; i < m_streamIds.size(); ++i)
        {
            if (curStreamId == m_streamIds[i])
            {
                m_3dViews[i]->setRotatingSpeed (speed, true);
                break;
            }
        }
    }
}

void QTViewer::flipVertically (bool enabled)
{
    m_flipVertical = enabled;
    for (auto i = 0; i < m_maxStreams; ++i)
    {
        m_2dViews[i]->flipVertically (enabled);
    }

}

void QTViewer::flipHorizontally (bool enabled)
{
    m_flipHorizontal = enabled;
    for (auto i = 0; i < m_maxStreams; ++i)
    {
        m_2dViews[i]->flipHorizontally (enabled);
    }
}

void QTViewer::openCameraOrRecording()
{
    CameraStatus ret;
    if (m_cameraDevice == nullptr)
    {
        QString statusMessage = "Opening camera";
        if (!m_playbackFilename.empty())
        {
            statusMessage = "Opening recording";
        }

        std::unique_ptr<StartupMessageView> statusView (new StartupMessageView (statusMessage, this));

        // no connection established yet
#ifdef TARGET_PLATFORM_ANDROID
        ret = initAndStart (androidUsbDeviceFD, androidUsbDeviceVid, androidUsbDevicePid);
#else
        ret = initAndStart();
#endif
        statusView.reset();

        if (ret == CameraStatus::SUCCESS)
        {
            m_isConnected = true;
        }
        else
        {
            widgets.helpView->addLogMessage ("Error: capturing not started.");
        }
        m_isCurrentlyConnecting = false;

        if (m_isConnected)
        {
            widgets.helpView->addInfoMessage ("");
            if (!m_playbackFilename.empty())
            {
                widgets.helpView->addInfoMessage ("- Playback Mode -");
                widgets.helpView->addInfoMessage ("used camera of playback file :");
            }
            else
            {
                widgets.helpView->addInfoMessage ("started camera :");
            }

            royale::String cameraName;
            royale::String cameraId;
            ret = m_cameraDevice->getCameraName (cameraName);
            if (ret == CameraStatus::SUCCESS)
            {
                widgets.helpView->addLogMessage (QString ("Camera Name : ") + QString (cameraName.toStdString().c_str()));
                widgets.helpView->addInfoMessage (QString ("Name : ") + QString (cameraName.toStdString().c_str()));
                m_cameraNameStr = QString (cameraName.toStdString().c_str());
            }
            else
            {
                widgets.helpView->addLogMessage ("Error reading Camera Name: " + QString::number ( (int) ret) + " " + QString (getErrorString (ret).c_str()));
            }

            ret = m_cameraDevice->getId (cameraId);
            if (ret == CameraStatus::SUCCESS)
            {
                widgets.helpView->addLogMessage (QString ("Camera ID : ") + QString (cameraId.toStdString().c_str()));
                widgets.helpView->addInfoMessage (QString ("ID : ") + QString (cameraId.toStdString().c_str()));
                m_cameraIdStr = QString (cameraId.toStdString().c_str());
            }
            else
            {
                widgets.helpView->addLogMessage ("Error reading Camera ID: " + QString::number ( (int) ret) + " " + QString (getErrorString (ret).c_str()));
            }

            Vector<Pair<String, String>> cameraInfo;
            ret = m_cameraDevice->getCameraInfo (cameraInfo);
            if (ret == CameraStatus::SUCCESS)
            {
                for (size_t i = 0; i < cameraInfo.size(); ++i)
                {
                    widgets.helpView->addLogMessage (QString (cameraInfo.at (i).first.c_str()) + QString (" : ") + QString (cameraInfo.at (i).second.c_str()));
                    widgets.helpView->addInfoMessage (QString (cameraInfo.at (i).first.c_str()) + QString (" : ") + QString (cameraInfo.at (i).second.c_str()));
                    m_cameraInfoStr += QString (cameraInfo.at (i).first.c_str()) + QString (" : ") + QString (cameraInfo.at (i).second.c_str()) + "\n";
                }
                widgets.helpView->addInfoMessage ("");
            }
            else
            {
                widgets.helpView->addLogMessage ("Error reading Camera Info: " + QString::number ( (int) ret) + " " + QString (getErrorString (ret).c_str()));
            }

            if (cameraName == "PICOS_STANDARD" ||
                    cameraName.find ("EVALBOARD_") != royale::String::npos)
            {
                QMessageBox::warning (this, "Calibration data not sufficient", "The validity of the displayed data cannot be guaranteed with this calibration data");
                widgets.helpView->addLogMessage ("Calibration data not sufficient: The validity of the displayed data cannot be guaranteed.");
            }
        }
    }
    else
    {
        // we already have a CameraDevice, so continue capturing
        ret = startCapture();
        switch (ret)
        {
            case CameraStatus::CALIBRATION_DATA_ERROR:
                {
                    QMessageBox::warning (this, "No calibration data found", "No calibration found for this camera module " + QString::fromStdString (m_serialNumber));
                    widgets.helpView->addLogMessage ("No calibration found for this camera module " + QString::fromStdString (m_serialNumber) + "!");
                    widgets.helpView->addLogMessage ("Error: capturing not started.");
                    m_cameraDevice.reset (nullptr);
                    break;
                }
            case CameraStatus::SUCCESS:
                m_isConnected = true;
                break;
            default:
                widgets.helpView->addLogMessage ("Error: capturing not started.");
                widgets.helpView->addLogMessage ("Error : " + QString (getErrorString (ret).c_str()));
                break;
        }
        m_isCurrentlyConnecting = false;
    }

    if (m_isConnected)
    {
        selectCurrentUseCaseInList();
        initAutoExposureModes (m_autoExposure);
        fillStreamView();
    }
    else
    {
        widgets.toolsMenu->updateToolEntries (false, m_2d, m_modeMixed, false);
    }

    updateCameraButton();
}

void QTViewer::closeCameraOrRecording()
{
    CameraStatus ret;
    if (recButton->isChecked())
    {
        recButton->setChecked (false);
        stopRecording();
    }
    ret = stopCapture();
    switch (ret)
    {
        case CameraStatus::CALIBRATION_DATA_ERROR:
        case CameraStatus::SUCCESS:
            m_isConnected = false;
            break;
        default:
            widgets.helpView->addLogMessage ("Error: capturing not stopped.");
            widgets.helpView->addLogMessage ("Error : " + QString (getErrorString (ret).c_str()));
            break;
    }
    if (!m_playbackFilename.empty())
    {
        m_playbackFilename.clear();
        m_replayControl = NULL;
        movieControl->hide();
#ifndef TARGET_PLATFORM_ANDROID
        m_cameraDevice.reset (nullptr);
#endif
    }

    widgets.toolsMenu->updateToolEntries (false, m_2d, m_modeMixed, !m_playbackFilename.empty());

    updateCameraButton();
}

void QTViewer::updateCameraButton()
{
    if (m_isConnected)
    {
        cameraButton->setText (qStrStopText);
        cameraButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-disconnect-default"));
        cameraButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-disconnect-pressed"));
        cameraButton->setChecked (true);
    }
    else
    {
        cameraButton->setText (qStrStartText);
        cameraButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-connect-default"));
        cameraButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-connect-pressed"));
        cameraButton->setChecked (false);
    }
}

void QTViewer::refreshInfoMessage()
{
    QApplication::setOverrideCursor (Qt::WaitCursor);
    QApplication::processEvents();
    checkForNewAccessCode();
    royale::String accessCode = ROYALE_ACCESS_CODE_LEVEL2;
    if (m_accessLevel > CameraAccessLevel::L2)
    {
        accessCode = m_accessCode;
    }
    CameraManager manager (accessCode);

#ifdef TARGET_PLATFORM_ANDROID
    Vector<royale::String> connectedCameras;
    if (m_isConnected && !m_replayControl)
    {
        // Currently, information update is not supported in Android while the camera is running
        QApplication::restoreOverrideCursor();
        return;
    }
    else
    {
        connectedCameras = manager.getConnectedCameraList (androidUsbDeviceFD, androidUsbDeviceVid, androidUsbDevicePid);
    }
#else
    auto connectedCameras = manager.getConnectedCameraList();
#endif
    auto cameraNames = manager.getConnectedCameraNames();

    widgets.helpView->clearInfoMessage();
    if (m_isConnected && !m_replayControl)
    {
        if (connectedCameras.size() == 0)
        {
            widgets.helpView->addInfoMessage ("1 camera found :");
        }
        else
        {
            widgets.helpView->addInfoMessage (QString::number (connectedCameras.size() + 1) + " cameras found :");

            for (size_t i = 0; i < connectedCameras.size(); ++i)
            {
                widgets.helpView->addInfoMessage ("- " + QString::number (i + 1) + " -");
                widgets.helpView->addInfoMessage ("Name : " + QString (cameraNames[i].c_str()));
                widgets.helpView->addInfoMessage ("ID : " + QString (connectedCameras[i].c_str()));
            }
            widgets.helpView->addInfoMessage ("- " + QString::number (connectedCameras.size() + 1) + " -");
        }
        widgets.helpView->addInfoMessage ("Name : " + m_cameraNameStr);
        widgets.helpView->addInfoMessage ("ID : " + m_cameraIdStr);

        widgets.helpView->addInfoMessage ("");
        widgets.helpView->addInfoMessage ("started camera :");
        widgets.helpView->addInfoMessage ("Name : " + m_cameraNameStr);
        widgets.helpView->addInfoMessage ("ID : " + m_cameraIdStr);
        widgets.helpView->addInfoMessage (m_cameraInfoStr);
    }
    else
    {
        if (connectedCameras.size() == 0)
        {

            widgets.helpView->addInfoMessage ("No camera found");
        }
        else if (connectedCameras.size() == 1)
        {
            widgets.helpView->addInfoMessage ("1 camera found :");
            widgets.helpView->addInfoMessage ("Name : " + QString (cameraNames[0].c_str()));
            widgets.helpView->addInfoMessage ("ID : " + QString (connectedCameras[0].c_str()));
        }
        else
        {
            widgets.helpView->addInfoMessage (QString::number (connectedCameras.size()) + " cameras found :");
            for (size_t i = 0; i < connectedCameras.size(); ++i)
            {
                widgets.helpView->addInfoMessage ("- " + QString::number (i + 1) + " -");
                widgets.helpView->addInfoMessage ("Name : " + QString (cameraNames[i].c_str()));
                widgets.helpView->addInfoMessage ("ID : " + QString (connectedCameras[i].c_str()));
            }
        }

        if (m_replayControl)
        {
            widgets.helpView->addInfoMessage ("");
            widgets.helpView->addInfoMessage ("- Playback Mode -");
            widgets.helpView->addInfoMessage ("used camera of playback file :");
            widgets.helpView->addInfoMessage ("Name : " + m_cameraNameStr);
            widgets.helpView->addInfoMessage ("ID : " + m_cameraIdStr);
        }
    }
    QApplication::restoreOverrideCursor();
}

void QTViewer::hideTool()
{
    if (widgets.currentTool)
    {
        widgets.currentTool->hide();
    }
    widgets.toolsMenu->deselect();
    if (m_toolsPanel && m_toolsPanel->isVisible())
    {
        toolsButton->click();
    }
}

void QTViewer::readRegister (QString registerStr)
{
    if (!m_cameraDevice)
    {
        return;
    }

    Vector<Pair<String, uint64_t>> inRegister = { { registerStr.toStdString().c_str(), 0x0 } };

    m_cameraDevice->readRegisters (inRegister);

    emit registerReadReturn (registerStr, static_cast<uint16_t> (inRegister.at (0).second));
}

void QTViewer::writeRegister (QString registerStr, uint16_t value)
{
    if (!m_cameraDevice)
    {
        return;
    }

    Vector<Pair<String, uint64_t>> inRegister = { { registerStr.toStdString().c_str(), value } };

    auto res = m_cameraDevice->writeRegisters (inRegister);

    if (res == CameraStatus::SUCCESS)
    {
        emit registerWriteReturn (true);
    }
    else
    {
        emit registerWriteReturn (false);
    }
}

void QTViewer::loadFile ()
{
    QString fileName = QFileDialog::getOpenFileName (this, tr ("Open File"),
                       m_lastLoadFolder,
                       tr ("Royale recordings (*.rrf)"));
    if (fileName.isEmpty())
    {
        return;
    }

    openPlaybackFile (fileName.toLocal8Bit().constData());
}

QString QTViewer::getSavePath()
{
    QDir tempDir;
    if (m_appSettings->contains (qStrSaveFolder))
    {
        tempDir.setPath (m_appSettings->value (qStrSaveFolder).toString());
        if (!tempDir.exists())
        {
            tempDir.setPath (QString::fromStdString (royaleviewer::getOutputPath()));
        }
    }
    else
    {
        tempDir.setPath (QString::fromStdString (royaleviewer::getOutputPath()));
    }

    return tempDir.absolutePath();
}

void QTViewer::filterLevelChanged (const royale::FilterLevel &level, royale::StreamId streamId)
{
    if (!m_cameraDevice)
    {
        return;
    }

    m_cameraDevice->setFilterLevel (level, streamId);
}

void QTViewer::streamIdFilterLevelViewChanged (royale::StreamId streamId)
{
    widgets.filterLevelView->setCurrentStreamId (streamId);

    FilterLevel level;
    m_cameraDevice->getFilterLevel (level, streamId);
    widgets.filterLevelView->setFilterLevel (level);
}
