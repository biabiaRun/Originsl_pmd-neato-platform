/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <device/CameraCore.hpp>
#include <factory/BridgeController.hpp>
#include <factory/ICameraCoreBuilder.hpp>

#include <royale/Pair.hpp>
#include <royale/Vector.hpp>
#include <imager/Imager.hpp>

#include <collector/IFrameCaptureListener.hpp>
#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/Exception.hpp>
#include <common/exceptions/InvalidValue.hpp>

#include <PlatformResources.hpp>

#include <cassert>
#include <limits>
#include <thread>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <algorithm>
#include <ostream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GL/freeglut.h>
#else
#include <GL/gl.h>
#include <GL/freeglut.h>
#endif

#include <common/RoyaleLogger.hpp>

using namespace royale::common;
using namespace royale::usecase;
using namespace royale::collector;

static const char *WINDOW_TITLE = "RawViewer";

float aspect = 1.f; //size for displaying the video (will be adjusted before calling glutInitWindowSize)

std::atomic<float> illuTemp;             // illumination temperature
std::atomic<unsigned int> frameRate;     // calculated framerate
std::thread fpsUpdater;                  // measures the current FPS
bool running = true;                     // stop condition for fpsUpdater
std::atomic<std::size_t> phaseIndex;     // index of phase image to visualize
bool grayscaleExpoOn = true;             // true if exposure should be on for grayscale images

royale::usecase::UseCaseDefinition *currentDefinition;

/**
 * Listener to receive data from the camera and display it via OpenGL
 */
class RawViewerListener : public IFrameCaptureListener
{
    /** For handling raw texture data in GL_RGBA GL_UNSIGNED_BYTE format */
    typedef struct
    {
        GLubyte r;
        GLubyte g;
        GLubyte b;
        GLubyte a;
    } RGBAPixel;

public:
    explicit RawViewerListener (IFrameCaptureReleaser *releaser)
    {
        m_startTime = CapturedUseCase::CLOCK_TYPE::now();

        m_releaser = releaser;
        m_waitingCaptureWidth16 = 0;
        m_waitingCaptureHeight16 = 0;

        m_texWidth = 0;
        m_texHeight = 0;
        m_texName = 0;
        m_texUpdated = false;

        resetFrameCounter();
        glGenTextures (1, &m_texName);
    }

    void resetFrameCounter()
    {
        m_frameCounter = 0;
    }

    unsigned int getFrameCounter() const
    {
        return m_frameCounter;
    }

    /**
     * Called in the acquisition thread, stores the data in the local queue for later processing.
     */
    virtual void captureCallback (std::vector<ICapturedRawFrame *> &captured,
                                  const UseCaseDefinition &useCase,
                                  royale::StreamId streamId,
                                  std::unique_ptr<const CapturedUseCase> capturedCase) override
    {
        // scope for the lock
        {
            std::unique_lock<std::mutex> lock (m_waitingLock);
            illuTemp = capturedCase->getIlluminationTemperature();
            m_waitingForProcessing.swap (captured);
            useCase.getImage (m_waitingCaptureWidth16, m_waitingCaptureHeight16);
            m_waitingCapturedCase.swap (capturedCase);
        }
        if (! captured.empty())
        {
            // This is reached when processCaptureQueue has removed the frames that were
            // added during the previous call to captureCallback.  If the capture is started
            // before the OpenGL thread, this will appear at startup.
            LOG (ERROR) << "OpenGL thread didn't process data - dropping a capture in the RawViewer";

            m_releaser->releaseCapturedFrames (captured);
        }

        // \todo The capture should be processed in a separate thread
        processCaptureQueue ();
    }

    void releaseAllFrames () override
    {
        // as the call to processCaptureQueue is done in captureCallback(), this can be a no-op
    }

    /**
     * If there's a new capture in m_waitingForProcessing, update the RGBA data in m_texWorkArea and
     * then release the captured frames.
     *
     * This should be running in a dedicated thread, but it can be called from the acquisition
     * thread.
     *
     * It is safe to call this from the GLUT event thread, but it has to be called from somewhere
     * else as well.  If the only caller is the GLUT event thread, then that thread can deadlock
     * when it calls CameraModule::setUseCase.
     */
    void processCaptureQueue ()
    {
        GLsizei captureWidth {0};
        GLsizei captureHeight {0};

        std::vector<ICapturedRawFrame *> captured;
        std::unique_ptr<const CapturedUseCase> capturedCase;
        // scope for the lock
        {
            std::unique_lock<std::mutex> lock (m_waitingLock);
            m_waitingForProcessing.swap (captured);
            captureWidth = m_waitingCaptureWidth16;
            captureHeight = m_waitingCaptureHeight16;
            m_waitingCapturedCase.swap (capturedCase);
        }

        if (captured.empty())
        {
            return;
        }

        // scope for the lock
        {
            std::unique_lock<std::mutex> lock (m_texLock);
            m_texWidth = captureWidth;
            m_texHeight = captureHeight;
            m_texWorkArea.resize (captureHeight * captureWidth);

            auto frame = captured[std::min (phaseIndex.load(), captured.size() - 1)];
            auto data = frame->getImageData();
            int dataPtr = 0;
            // convert 16 bit data into 8 bit data stream
            // GL coordinates have (0,0) at the bottom-left, thus the direction of the height loop
            for (int y = captureHeight - 1; y >= 0; y--)
            {
                RGBAPixel *texRow = &m_texWorkArea[y * captureWidth];
                for (int x = 0; x < captureWidth; x++)
                {
                    RGBAPixel &texPixel = texRow[x];
                    // for each color, take the 8 most significant bits of the 12-bit data
                    texPixel.r = static_cast<GLubyte> (data[dataPtr] >> 4);
                    texPixel.g = static_cast<GLubyte> (data[dataPtr] >> 4);
                    texPixel.b = static_cast<GLubyte> (data[dataPtr] >> 4);
                    texPixel.a = (GLubyte) 255;
                    dataPtr += 1;
                }
            }

            m_texUpdated = true;
        }

        m_releaser->releaseCapturedFrames (captured);
        m_frameCounter++;
    }

    /**
     * Called from the GLUT event / redraw thread.  If a new frame has been processed, push the new
     * data to OpenGL.  If called while a new frame is being processed in
     * processCaptureQueue() then this blocks just long enough to get the new data.
     */
    void updateTexture()
    {
        std::unique_lock<std::mutex> lock (m_texLock);

        if (!m_texUpdated)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (1));
            return;
        }

        m_texUpdated = false;

        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, m_texWidth,
                      m_texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                      &m_texWorkArea[0]);
    }

    /**
     * Draws the latest-received data on the current active GL context.
     */
    void drawGl (float top, float left, float bottom, float right)
    {
        glEnable (GL_TEXTURE_2D);
        glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glBindTexture (GL_TEXTURE_2D, m_texName);

        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                         GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                         GL_NEAREST);

        updateTexture();

        glBegin (GL_QUADS);
        glTexCoord2f (0.0, 0.0);
        glVertex3f (top, left, 0.0);
        glTexCoord2f (0.0, 1.0);
        glVertex3f (top, right, 0.0);
        glTexCoord2f (1.0, 1.0);
        glVertex3f (bottom, right, 0.0);
        glTexCoord2f (1.0, 0.0);
        glVertex3f (bottom, left, 0.0);
        glEnd();
        glDisable (GL_TEXTURE_2D);
    }

private:
    IFrameCaptureReleaser *m_releaser;
    unsigned int m_frameCounter;

    /** Lock for updating the m_waiting* variables */
    std::mutex m_waitingLock;
    std::vector<ICapturedRawFrame *> m_waitingForProcessing;
    /** Size of each frame in m_waitingForProcessing */
    uint16_t m_waitingCaptureWidth16;
    /** Size of each frame in m_waitingForProcessing */
    uint16_t m_waitingCaptureHeight16;
    std::unique_ptr<const CapturedUseCase> m_waitingCapturedCase;

    /**
     * Lock for the m_tex* variables.
     *
     * Lock ordering: if taking both locks, m_texLock should be taken first.  It's better to block
     * the display thread rather than the acquisition thread.
     */
    std::mutex m_texLock;
    /**
     * Flag that is set when processCaptureQueue() updates, and is cleared when the OpenGL texture
     * has the new data.
     */
    bool m_texUpdated;
    /** Size of the frame in m_texWorkArea. Zero means no data. */
    GLsizei m_texWidth;
    /** Size of the frame in m_texHeight. Zero means no data. */
    GLsizei m_texHeight;

    /**
     * Raw data for the GL texture.
     *
     * This could be a local variable in processCaptureQueue, it's a member variable to avoid
     * allocating it in every loop.
     */
    std::vector<RGBAPixel> m_texWorkArea;
    /** GL texture used by drawGl */
    GLuint m_texName;

    CapturedUseCase::CLOCK_TYPE::time_point m_startTime;
};

//The resources shouldn't be initialized before main(), but need a lifespan longer than the Module.
std::unique_ptr<sample_utils::PlatformResources> platformResources;
std::unique_ptr<royale::device::CameraCore> cameraCore;
std::shared_ptr<const royale::config::ICoreConfig> coreConfig;
std::unique_ptr<RawViewerListener> listener;

void display (void)
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (listener)
    {
        listener->drawGl (-1.0f, -1.0f, 1.0f, 1.0f);
    }

    glFlush();

    // update header w/ new frame rate
    std::ostringstream oss;
    unsigned char degreeSign = 176;
    int illuDegree = static_cast<int> (illuTemp);
    oss << WINDOW_TITLE << " " << std::to_string (frameRate) << " fps, temperature " << illuDegree << " " << degreeSign << "C";
    glutSetWindowTitle (oss.str().c_str());

    // refresh display again since data may have changed
    glutPostRedisplay();
}

void init (void)
{
    // clearing (background) color
    glClearColor (0.0, 0.0, 0.0, 0.0);

    glShadeModel (GL_FLAT);
    glEnable (GL_DEPTH_TEST);

    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
}

void keyboard (unsigned char key, int x, int y)
{
    try
    {
        switch (key)
        {
            case 27: //escape key
                glutLeaveMainLoop();
                break;
            case '1':
                phaseIndex = 0;
                break;
            case '2':
                phaseIndex = 1;
                break;
            case '3':
                phaseIndex = 2;
                break;
            case '4':
                phaseIndex = 3;
                break;
            case '5':
                phaseIndex = 4;
                break;
            case '6':
                phaseIndex = 5;
                break;
            case '7':
                phaseIndex = 6;
                break;
            case '8':
                phaseIndex = 7;
                break;
            case '9':
                phaseIndex = 8;
                break;
            case '0':
                phaseIndex = 9;
                break;
            case '+':
                {
                    auto exposureTimes = currentDefinition->getExposureTimes();
                    auto limits = currentDefinition->getExposureLimits();
                    for (size_t i = 0; i < exposureTimes.size(); ++i)
                    {
                        auto &exposureTime = exposureTimes.at (i);
                        auto limit = limits.at (i).second;
                        if (exposureTime < limit)
                        {
                            exposureTime = std::min (exposureTime + 100, limit);
                        }
                    }
                    currentDefinition->setExposureTimes (exposureTimes);
                    cameraCore->reconfigureImagerExposureTimes (currentDefinition, exposureTimes.toStdVector());
                    break;
                }
            case '-':
                {
                    auto exposureTimes = currentDefinition->getExposureTimes();
                    auto limits = currentDefinition->getExposureLimits();
                    for (size_t i = 0; i < exposureTimes.size(); ++i)
                    {
                        auto &exposureTime = exposureTimes.at (i);
                        auto limit = limits.at (i).first;
                        if (exposureTime > limit)
                        {
                            // need to consider integer overflows for this case
                            if (exposureTime > 100)
                            {
                                exposureTime -= 100;
                            }
                            else
                            {
                                exposureTime = 0;
                            }
                            exposureTime = std::max (exposureTime, limit);
                        }
                    }
                    currentDefinition->setExposureTimes (exposureTimes);
                    cameraCore->reconfigureImagerExposureTimes (currentDefinition, exposureTimes.toStdVector());
                    break;
                }
            case '/':
                {
                    auto currentFrameRate = currentDefinition->getTargetRate();
                    if (currentFrameRate > currentDefinition->getMinRate())
                    {
                        currentFrameRate--;
                    }
                    else
                    {
                        currentFrameRate = currentDefinition->getMinRate();
                    }
                    currentDefinition->setTargetRate (currentFrameRate);
                    cameraCore->reconfigureImagerTargetFrameRate (currentFrameRate);
                    break;
                }
            case '*':
                {
                    auto currentFrameRate = currentDefinition->getTargetRate();
                    if (currentFrameRate < currentDefinition->getMaxRate())
                    {
                        currentFrameRate++;
                    }
                    else
                    {
                        currentFrameRate = currentDefinition->getMaxRate();
                    }
                    currentDefinition->setTargetRate (currentFrameRate);
                    cameraCore->reconfigureImagerTargetFrameRate (currentFrameRate);
                    break;
                }
        }
    }
    catch (const Exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

void reshape (int w, int h)
{
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    gluPerspective (60.0, (GLfloat) w / (GLfloat) h, 1.0, 30.0);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef (0.0, 0.0, -2.0);
    glScalef (aspect, 1.0, 1.0);

}

void fpsCalculation()
{
    // update FPS
    while (running)
    {
        listener->resetFrameCounter();
        std::this_thread::sleep_for (std::chrono::seconds (1));
        frameRate = listener->getFrameCounter();
    }
}

int main (int argc, char **argv)
{
    platformResources.reset (new sample_utils::PlatformResources);

    royale::factory::BridgeController bridgeController;

    // Visualize the last phase of each set.  The first phase is often non-illuminated grayscale,
    // which doesn't change very much when an object moves in front of the sensor.
    phaseIndex = std::numeric_limits<std::size_t>::max();

    try
    {
        auto devices = bridgeController.probeDevices();

        if (devices.empty())
        {
            throw Exception ("No device connected.");
        }

        auto &builder = *devices.front();

        cameraCore = builder.createCameraCore();
        coreConfig = builder.getICoreConfig();
        cameraCore->initializeImager();

    }
    catch (Exception &e)
    {
        LOG (ERROR) << "Cannot create CameraModule, it seems that there are missing components: " << e.what();
        return -1;
    }

    auto useCases = coreConfig->getSupportedUseCases();
    if (useCases.empty())
    {
        LOG (ERROR) << "Module has no supported use cases, can not initialize the module";
        return -1;
    }
    try
    {
        cameraCore->setUseCase (useCases[0].getDefinition());
    }
    catch (Exception &e)
    {
        LOG (ERROR) << "Module does not support its use case: " << e.what();
        return -1;
    }

    currentDefinition = useCases[0].getDefinition();

    uint16_t columns, rows;
    currentDefinition->getImage (columns, rows);
    aspect = columns / (float) rows;

    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize (640, static_cast<int> (640.0f / aspect));
    glutInitWindowPosition (100, 100);

    glutCreateWindow (WINDOW_TITLE);

    init ();

    glutDisplayFunc (display);
    glutReshapeFunc (reshape);
    glutKeyboardFunc (keyboard);
    //glutIdleFunc (idleFunc);

    listener.reset (new RawViewerListener (cameraCore->getCaptureReleaser()));
    cameraCore->setCaptureListener (listener.get());

    try
    {
        cameraCore->startCapture();
    }
    catch (Exception &e)
    {
        LOG (ERROR) << "Exception during startCapture(): " << e.what();
        cameraCore->setCaptureListener (nullptr);
        return 1;
    }

    fpsUpdater = std::thread (fpsCalculation);

    //set the option to proceed after glutMainLoop when the user closes the window
    glutSetOption (GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    glutMainLoop();

    running = false;

    fpsUpdater.join();

    try
    {
        cameraCore->stopCapture();
    }
    catch (Exception &e)
    {
        LOG (ERROR) << "Exception during stopCapture(): " << e.what();
    }
    cameraCore->setCaptureListener (nullptr);

    return 0;
}
