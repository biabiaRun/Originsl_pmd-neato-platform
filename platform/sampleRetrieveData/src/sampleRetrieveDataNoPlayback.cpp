/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

#include <royale/ICameraDevice.hpp>

#include <CameraFactory.hpp>


using namespace std;

using namespace royale;
using namespace platform;

class MyListener : public royale::IDepthDataListener
{
public:
    MyListener()
    {
    }

    ~MyListener()
    {
    }

    void onNewData (const royale::DepthData *data) override
    {

    }
};

int main (int argc, char **argv)
{
    // this represents the main camera device object
    unique_ptr<royale::ICameraDevice> cameraDevice;
    // if non-null, choose this use case instead of the default
    unique_ptr<royale::String> commandLineUseCase;

    royale::common::LogSettings::getInstance ()->setLogFile ("log.txt");
    uint16_t logSettings = 0xf;
    royale::common::LogSettings::getInstance ()->setLogLevel (logSettings);//royale::common::LogSettings::ENABLE_ALL_LOGS);
    LOG (DEBUG) << "hello";

    CameraFactory factory;
    if (argc > 1)
    {
        auto arg = std::unique_ptr<royale::String> (new royale::String (argv[1]));
        cout << "Assuming command-line argument is the name of a use case" << endl;
        commandLineUseCase = std::move (arg);
    }

    cameraDevice = factory.createCamera();

    if (cameraDevice == nullptr)
    {
        cerr << "Cannot create the camera device" << endl;
        return 1;
    }
    cout << "Initialize camera start" << std::endl;
    // IMPORTANT: call the initialize method before working with the camera device
    if (cameraDevice->initialize() != royale::CameraStatus::SUCCESS)
    {
        cerr << "Cannot initialize the camera device" << endl;
        return 1;
    }
    cout << "Initialize camera OK" << std::endl;

    royale::Vector<royale::String> useCases;
    auto status = cameraDevice->getUseCases (useCases);

    if (status != royale::CameraStatus::SUCCESS || useCases.empty())
    {
        cerr << "No use cases are available" << endl;
        return 1;
    }

    cout << "Supported use cases:" << std::endl;
    const auto listIndent = std::string ("    ");
    const auto noteIndent = std::string ("        ");

    for (size_t i = 0; i < useCases.size(); ++i)
    {
	    cout << listIndent << useCases[i] << endl;

	    uint32_t streamCount = 0;
	    status = cameraDevice->getNumberOfStreams (useCases[i], streamCount);
	    if (royale::CameraStatus::SUCCESS == status && streamCount > 1)
	    {
		    cout << noteIndent << "this operation mode has " << streamCount << " streams" << endl;
	    }
    }

    // choose a use case
    auto selectedUseCaseIdx = 0u;
    if (commandLineUseCase)
    {
        auto useCaseFound = false;

        for (auto i = 0u; i < useCases.size(); ++i)
        {
            if (*commandLineUseCase == useCases[i])
            {
                selectedUseCaseIdx = i;
                useCaseFound = true;
                break;
            }
        }

        if (!useCaseFound)
        {
            cerr << "Error: the chosen use case is not supported by this camera" << endl;
            return 1;
        }
    }
    else
    {
        // choose the first use case
        selectedUseCaseIdx = 0;
    }

    // set an operation mode
    cout << "Set use case " << useCases[selectedUseCaseIdx] << std::endl;
    if (cameraDevice->setUseCase (useCases.at (selectedUseCaseIdx)) != royale::CameraStatus::SUCCESS)
    {
        cerr << "Error setting use case" << endl;
        return 1;
    }

    // Retrieve the IDs of the different streams
    royale::Vector<royale::StreamId> streamIds;
    if (cameraDevice->getStreams (streamIds) != royale::CameraStatus::SUCCESS)
    {
        cerr << "Error retrieving streams" << endl;
        return 1;
    }

    cout << "Stream IDs : ";
    for (auto curStream : streamIds)
    {
        cout << curStream << " ";
    }
    cout << endl;

    // register a data listener
    unique_ptr<MyListener> listener;

    listener.reset (new MyListener ());
    if (cameraDevice->registerDataListener (listener.get()) != royale::CameraStatus::SUCCESS)
    {
        cerr << "Error registering data listener" << endl;
        return 1;
    }

    // start capture mode
    cout << "Capture start" << std::endl;
    if (cameraDevice->startCapture() != royale::CameraStatus::SUCCESS)
    {
        cerr << "Error starting the capturing" << endl;
        return 1;
    }


    //Start recording to file
    cameraDevice->startRecording ("test.rrf");

    // let the camera capture for some time
    this_thread::sleep_for (chrono::seconds (5));

    // Change the exposure time for the first stream of the use case (Royale will limit this to an
    // eye-safe exposure time, with limits defined by the use case).  The time is given in
    // microseconds.
    //
    // Non-mixed mode use cases have exactly one stream, mixed mode use cases have more than one.
    // For this example we only change the first stream.
    if (cameraDevice->setExposureTime (200, streamIds[0]) != royale::CameraStatus::SUCCESS)
    {
        cerr << "Cannot set exposure time for stream" << streamIds[0] << endl;
    }
    else
    {
        cout << "Changed exposure time for stream " << streamIds[0] << " to 200 microseconds ..." << endl;
    }

    // let the camera capture for some time
    this_thread::sleep_for (chrono::seconds (5));


    //Stop recording
    cameraDevice->stopRecording();

    // stop capture mode
    if (cameraDevice->stopCapture() != royale::CameraStatus::SUCCESS)
    {
        cerr << "Error stopping the capturing" << endl;
        return 1;
    }
    cout << "Capture stop" << std::endl;

    return 0;
}
