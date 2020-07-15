/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

#ifdef ROYALE_TARGET_PLATFORM_WINDOWS
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include <common/FileSystem.hpp>

#include <royale.hpp>
#include <royale/IPlaybackStopListener.hpp>
#include <royale/IReplay.hpp>

#include <importExportHelperLib/csvHelper.hpp>
#include <importExportHelperLib/plyHelper.hpp>
#include <importExportHelperLib/binHelper.hpp>
#include <importExportHelperLib/paramHelper.hpp>
#include <importExportHelperLib/imgHelper.hpp>

using namespace std;

namespace
{
    class MyListener : public royale::IExtendedDataListener
    {
    public:
        MyListener (string rrfFile, string outpath, uint32_t numFrames,
                    const std::map<royale::StreamId, bool> &irModes) :
            m_numFrames (numFrames),
            m_rrfFile (std::move (rrfFile)),
            m_outpath (outpath),
            m_currentFrame (0u),
            m_irModes (irModes)
        {
        }

        void onNewData (const royale::IExtendedData *data) override
        {
            m_currentFrame++;
            cout << "Exporting frame " << m_currentFrame << " of " << m_numFrames << endl;

            auto streamId = data->getDepthData()->streamId;
            std::string streamNumber = "_" + std::to_string (streamId);

            std::string fileNamePart = m_outpath + "/" + std::to_string (m_currentFrame) + streamNumber;

            if (m_irModes[streamId])
            {
                cout << "Exporting IR frame" << endl;

                //PNG
                //IR1
                {
                    const std::string csvPath = fileNamePart + "_IR_IlluOn-FPN.png";
                    QImage img ( (int) data->getDepthData()->width, (int) data->getDepthData()->height, QImage::Format_Grayscale8);
                    royale::importExportHelperLib::encodeIR1PNG (data->getDepthData(), img);
                    QString qFilePath = QString::fromStdString (csvPath);
                    img.save (qFilePath, 0);
                }

                //IR2
                {
                    const std::string csvPath = fileNamePart + "_IR_IlluOn-IlluOff.png";
                    QImage img ( (int) data->getIntermediateData()->width, (int) data->getIntermediateData()->height, QImage::Format_Grayscale8);
                    royale::importExportHelperLib::encodeIR2PNG (data->getIntermediateData(), img);
                    QString qFilePath = QString::fromStdString (csvPath);
                    img.save (qFilePath, 0);
                }
            }
            else
            {
                //PNG
                //ampl
                {
                    const std::string csvPath = fileNamePart + "_ampl.png";
                    QImage img ( (int) data->getDepthData()->width, (int) data->getDepthData()->height, QImage::Format_Grayscale8);
                    royale::importExportHelperLib::encodeAmplPNG (data->getDepthData(), img);
                    QString qFilePath = QString::fromStdString (csvPath);
                    img.save (qFilePath, 0);
                }

                //CSV
                //ampl
                {
                    const std::string csvPath = fileNamePart + "_ampl.csv";
                    std::ofstream fileOutput{};
                    fileOutput.open (csvPath);

                    if (fileOutput.is_open())
                    {
                        royale::importExportHelperLib::encodeAmplCSV (data->getDepthData(), fileOutput);
                        fileOutput.close();
                    }
                }

                //depth
                {
                    const std::string csvPath = fileNamePart + "_depth.csv";
                    std::ofstream fileOutput{};
                    fileOutput.open (csvPath);

                    if (fileOutput.is_open())
                    {
                        royale::importExportHelperLib::encodeDepthCSV (data->getDepthData(), fileOutput);
                        fileOutput.close();
                    }


                    //raw
                    {
                        const std::string csvPath = fileNamePart + "_raw.csv";
                        std::ofstream fileOutput{};
                        fileOutput.open (csvPath);

                        if (fileOutput.is_open())
                        {
                            royale::importExportHelperLib::encodeRawCSV (data->getRawData(), fileOutput);
                            fileOutput.close();
                        }
                    }


                    //PLY
                    //ampl + depth
                    {
                        const std::string csvPath = fileNamePart + "_depth_ampl.ply";
                        std::ofstream fileOutput{};
                        fileOutput.open (csvPath);

                        if (fileOutput.is_open())
                        {
                            royale::importExportHelperLib::encodePLY (data->getDepthData(), fileOutput);
                            fileOutput.close();
                        }
                    }

                    //BIN
                    //ampl
                    {
                        const std::string csvPath = fileNamePart + "_ampl.bin";
                        std::ofstream fileOutput{};
                        fileOutput.open (csvPath, ofstream::out | ofstream::binary);

                        if (fileOutput.is_open())
                        {
                            royale::importExportHelperLib::encodeAmplBIN (data->getDepthData(), fileOutput);
                            fileOutput.close();
                        }
                    }

                    //depth (in Meter)
                    {
                        const std::string csvPath = fileNamePart + "_depthFloat.bin";
                        std::ofstream fileOutput{};
                        fileOutput.open (csvPath, ofstream::out | ofstream::binary);

                        if (fileOutput.is_open())
                        {
                            royale::importExportHelperLib::encodeDepthBIN (data->getDepthData(), fileOutput);
                            fileOutput.close();
                        }
                    }

                    //depthMillimeter
                    {
                        const std::string csvPath = fileNamePart + "_depthMillimeter.bin";
                        std::ofstream fileOutput{};
                        fileOutput.open (csvPath, ofstream::out | ofstream::binary);

                        if (fileOutput.is_open())
                        {
                            royale::importExportHelperLib::encodeDepthMMBIN (data->getDepthData(), fileOutput);
                            fileOutput.close();
                        }
                    }

                    //raw (16Bit per pixel)
                    {
                        const std::string csvPath = fileNamePart + "_raw.bin";
                        std::ofstream fileOutput{};
                        fileOutput.open (csvPath, ofstream::out | ofstream::binary);

                        if (fileOutput.is_open())
                        {
                            royale::importExportHelperLib::encodeRawBIN (data->getRawData(), fileOutput);
                            fileOutput.close();
                        }
                    }
                }
            }
        }

    private:
        uint32_t m_numFrames;   // Total number of frames in the recording
        string m_rrfFile;       // Recording file that was opened
        string m_outpath;
        uint32_t m_currentFrame;
        std::map<royale::StreamId, bool> m_irModes;
    };

    class MyPlaybackStopListener : public royale::IPlaybackStopListener
    {
    public:
        MyPlaybackStopListener()
        {
            m_playbackRunning = true;
        }

        void onPlaybackStopped() override
        {
            lock_guard<mutex> lock (m_stopMutex);
            m_playbackRunning = false;
        }

        void waitForStop()
        {
            bool running = true;
            do
            {
                {
                    lock_guard<mutex> lock (m_stopMutex);
                    running = m_playbackRunning;
                }

                this_thread::sleep_for (chrono::milliseconds (50));
            }
            while (running);
        }

    private:
        mutex m_stopMutex;      // Mutex to synchronize the access to m_playbackRunning
        bool m_playbackRunning; // Shows if the playback is still running
    };
}

string getFileName (const string &s)
{

    char sep = '/';

#ifdef ROYALE_TARGET_PLATFORM_WINDOWS
    sep = '\\';
#endif

    auto retVal = s;

    {
        size_t i = retVal.rfind (sep, retVal.length());
        if (i != string::npos)
        {
            retVal = retVal.substr (i + 1, retVal.length() - i);
        }
    }

    {
        size_t i = retVal.rfind ('.', retVal.length());
        if (i != string::npos)
        {
            retVal = retVal.substr (0, i);
        }
    }

    return retVal;
}

int main (int argc, char *argv[])
{
    // This is the data listener which will receive callbacks.  It's declared
    // before the cameraDevice so that, if this function exits with a 'return'
    // statement while the camera is still capturing, it will still be in scope
    // until the cameraDevice's destructor implicitly deregisters the listener.
    unique_ptr<MyListener> listener;

    // PlaybackStopListener which will be called as soon as the playback stops.
    MyPlaybackStopListener stopListener;

    // Royale's API treats the .rrf file as a camera, which it captures data from.
    unique_ptr<royale::ICameraDevice> cameraDevice;

    // check the command line for a given file
    if (argc < 2)
    {
        cout << "Usage " << argv[0] << " rrfFileToExport cfgFile" << endl;
        cout << endl;
        cout << "Each frame of the recording is saved as a separate file " << endl;
        cout << "in the current directory." << endl;
        return 1;
    }

    // Use the camera manager to open the recorded file, this block scope is because we can allow
    // the CameraManager to go out of scope once the file has been opened.
    {
        royale::CameraManager manager (ROYALE_ACCESS_CODE_LEVEL2);

        // create a device from the file
        cameraDevice = manager.createCamera (argv[1]);
    }

    // if the file was loaded correctly the cameraDevice is now available
    if (cameraDevice == nullptr)
    {
        cerr << "Cannot load the file " << argv[1] << endl;
        return 1;
    }

    // cast the cameraDevice to IReplay which offers more options for playing
    // back recordings
    auto replayControls = dynamic_cast<royale::IReplay *> (cameraDevice.get());

    if (replayControls == nullptr)
    {
        cerr << "Unable to cast to IReplay interface" << endl;
        return 1;
    }

    cameraDevice->setCallbackData (royale::CallbackData::Intermediate);

    // IMPORTANT: call the initialize method before working with the camera device
    if (cameraDevice->initialize() != royale::CameraStatus::SUCCESS)
    {
        cerr << "Cannot initialize the camera device" << endl;
        return 1;
    }
    if (argc >= 3)
    {
        royale::String cfgFile (argv[2]);

        if (!royale::common::fileexists (cfgFile))
        {
            cerr << "cfg file doesn't exist" << endl;
            return 2;
        }

        std::map<royale::StreamId, royale::ProcessingParameterVector> params;
        if (royale::importExportHelperLib::loadCfg (cfgFile.toStdString(), params) != royale::CameraStatus::SUCCESS)
        {
            cerr << "could not load config file" << endl;
            return 1;
        }

        for (auto curParams : params)
        {
            cameraDevice->setProcessingParameters (curParams.second, curParams.first);
        }
    }

    // turn off the looping of the playback
    {
        replayControls->loop (false);
    }

    // Turn off the timestamps to speed up the conversion. If timestamps are enabled, an .rrf that
    // was recorded at 5FPS will generate callbacks to onNewData() at only 5 callbacks per second.
    replayControls->useTimestamps (false);

    auto fileName = getFileName (argv[1]);

#ifdef ROYALE_TARGET_PLATFORM_WINDOWS
    _mkdir (fileName.c_str());
#else
    mkdir (fileName.c_str(), S_IRWXU);
#endif

    royale::Vector<royale::StreamId> streamIds;
    cameraDevice->getStreams (streamIds);

    auto numFrames = replayControls->frameCount();

    std::map<royale::StreamId, bool> irModes;

    for (auto curStream : streamIds)
    {
        royale::ProcessingParameterVector params;
        cameraDevice->getProcessingParameters (params, curStream);

        for (auto curParam : params)
        {
            if (curParam.first == royale::ProcessingFlag::SpectreProcessingType_Int)
            {
                if (curParam.second.getInt() == 6)
                {
                    irModes[curStream] = true;
                    break;
                }
                else
                {
                    irModes[curStream] = false;
                    break;
                }
            }
        }
    }

    // Create and register the data listener
    listener.reset (new MyListener (argv[1], fileName, numFrames,
                                    irModes));
    if (cameraDevice->registerDataListenerExtended (listener.get()) != royale::CameraStatus::SUCCESS)
    {
        cerr << "Error registering extended data listener" << endl;
        return 1;
    }

    // register a playback stop listener. This will be called as soon
    // as the file has been played back once (because loop is turned off)
    replayControls->registerStopListener (&stopListener);

    // start capture mode
    if (cameraDevice->startCapture() != royale::CameraStatus::SUCCESS)
    {
        cerr << "Error starting the capturing" << endl;
        return 1;
    }

    // block until the playback has finished
    stopListener.waitForStop();

    // stop capture mode
    if (cameraDevice->stopCapture() != royale::CameraStatus::SUCCESS)
    {
        cerr << "Error stopping the capturing" << endl;
        return 1;
    }

    return 0;
}
