/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

/*
 * This tool is a small utility which allows the user to validate the average depth value
 * of a given planar object. This is required to quickly have a plausibility test for
 * correct depth values.
 *
 * Capture mode can be stopped by pressing any key and then Enter
 */

#include <royale.hpp>
#include <iostream>
#include <royale/IDepthDataListener.hpp>
#include <chrono>
#include <thread>

using namespace royale;

class DepthDataListener : public IDepthDataListener
{
public:
    DepthDataListener () {}
    virtual ~DepthDataListener () {}

    void onNewData (const DepthData *data)
    {
        float avgDepth = 0.0f;
        auto cnt = 0;
        for (size_t i = 0; i < data->points.size(); i++)
        {
            if (data->points[i].depthConfidence > 0) // only take valid pixels
            {
                avgDepth += data->points[i].z;
                cnt++;
            }
        }
        avgDepth /= (float) cnt;
        printf ("Valid pixels %10d - Depth %5.2f [cm]\n", cnt, avgDepth * 100.0);
    }
};

int main (int argc, char *argv[])
{
    CameraManager manager;

    auto cameraList = manager.getConnectedCameraList();

    if (cameraList.empty())
    {
        std::cout << "Could not find a connected camera" << std::endl;
        return 1;
    }

    auto camera = manager.createCamera (cameraList[0]);

    if (camera == nullptr)
    {
        std::cout << "Cannot create the camera device" << std::endl;
        return 1;
    }

    // initialize camera
    auto ret = camera->initialize();

    // register data callback
    std::shared_ptr<DepthDataListener> listener { new DepthDataListener() };
    camera->registerDataListener (listener.get());

    // start capture mode
    ret = camera->startCapture();

    if (ret != CameraStatus::SUCCESS)
    {
        std::cout << "Cannot start capture mode: " << (int) ret << std::endl;
        return 1;
    }

    char x = 0;
    std::cin >> x;

    // stop capture mode
    ret = camera->stopCapture();

    if (ret != CameraStatus::SUCCESS)
    {
        std::cout << "Cannot stop capture mode" << std::endl;
    }

    return 0;
}
