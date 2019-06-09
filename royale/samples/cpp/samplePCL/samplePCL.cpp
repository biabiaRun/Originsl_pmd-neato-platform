/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <royale.hpp>

#include <condition_variable>
#include <mutex>
#include <thread>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/visualization/cloud_viewer.h>

#include <sample_utils/PlatformResources.hpp>


using namespace royale;
using namespace sample_utils;
using namespace std;
using namespace pcl;
using namespace pcl::visualization;

namespace
{
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud;
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloudFiltered;
    mutex cloudMutex;
    std::condition_variable cloudCV;
    bool newDataAvailable;
    bool filterData;
}

class RoyaleListener : public IDepthDataListener
{
public:
    RoyaleListener()
    {}

    void onNewData (const DepthData *data)
    {
        std::unique_lock<std::mutex> lock (cloudMutex);
        // Fill in the cloud data
        cloud->width    = data->width;
        cloud->height   = data->height;
        cloud->is_dense = true;
        cloud->points.resize (cloud->width * cloud->height);

        for (size_t i = 0u; i < cloud->points.size (); ++i)
        {
            if (data->points[i].depthConfidence > 0)
            {
                cloud->points[i].x = data->points[i].x;
                cloud->points[i].y = data->points[i].y;
                cloud->points[i].z = data->points[i].z;
            }
            else
            {
                // if the point is invalid, mark it with a special value
                cloud->points[i].x = cloud->points[i].y = cloud->points[i].z =
                                         std::numeric_limits<float>::quiet_NaN();
            }
        }

        // notify the waiting loop in the main thread
        newDataAvailable = true;
        cloudCV.notify_all();
    }
};

void keyboardEvent (const pcl::visualization::KeyboardEvent &event,
                    void *viewer_void)
{
    if (event.getKeySym() == "f" && event.keyDown())
    {
        // toggle filtering of the data
        filterData = !filterData;
    }
}

int main (int argc, char *argv[])
{
    // Windows requires that the application allocate these, not the DLL.
    PlatformResources resources;

    // This is the data listener which will receive callbacks.  It's declared
    // before the cameraDevice so that, if this function exits with a 'return'
    // statement while the camera is still capturing, it will still be in scope
    // until the cameraDevice's destructor implicitly de-registers the listener.
    RoyaleListener listener;

    newDataAvailable = false;
    filterData = false;

    // this represents the main camera device object
    std::unique_ptr<ICameraDevice> cameraDevice;

    // the camera manager will query for a connected camera
    {
        CameraManager manager;

        // check the number of arguments
        if (argc > 1)
        {
            // if the program was called with an argument try to open this as a file
            cout << "Trying to open : " << argv[1] << endl;
            cameraDevice = manager.createCamera (argv[1]);
        }
        else
        {
            // if no argument was given try to open the first connected camera
            royale::Vector<royale::String> camlist (manager.getConnectedCameraList());
            cout << "Detected " << camlist.size() << " camera(s)." << endl;

            if (!camlist.empty())
            {
                cameraDevice = manager.createCamera (camlist[0]);
            }
            else
            {
                cerr << "No suitable camera device detected." << endl
                     << "Please make sure that a supported camera is plugged in, all drivers are "
                     << "installed, and you have proper USB permission" << endl;
                return 1;
            }

            camlist.clear();
        }
    }
    // the camera device is now available and CameraManager can be deallocated here

    if (cameraDevice == nullptr)
    {
        // no cameraDevice available
        if (argc > 1)
        {
            // there was a problem opening the file
            cerr << "Could not open " << argv[1] << endl;
            return 1;
        }
        else
        {
            // we couldn't open any camera
            cerr << "Cannot create the camera device" << endl;
            return 1;
        }
    }

    // IMPORTANT: call the initialize method before working with the camera device
    auto status = cameraDevice->initialize();
    if (status != CameraStatus::SUCCESS)
    {
        cerr << "Cannot initialize the camera device, error string : " << getErrorString (status) << endl;
        return 1;
    }

    // create PointCloud objects that will hold the original data and the filtered one
    cloud.reset (new pcl::PointCloud<pcl::PointXYZ>());
    cloudFiltered.reset (new pcl::PointCloud<pcl::PointXYZ>());

    CloudViewer viewer ("Cloud Viewer");

    // we want to be able to switch between filtered and unfiltered
    // point clouds, that's why we need to register a callback for
    // keyboard events
    viewer.registerKeyboardCallback (keyboardEvent, (void *) &viewer);

    // register a data listener
    if (cameraDevice->registerDataListener (&listener) != CameraStatus::SUCCESS)
    {
        cerr << "Error registering data listener" << endl;
        return 1;
    }

    // start capturing from the device/file
    if (cameraDevice->startCapture() != CameraStatus::SUCCESS)
    {
        cerr << "Error starting the capturing" << endl;
        return 1;
    }

    while (!viewer.wasStopped())
    {
        // while the viewer window is not closed, wait for new data to arrive
        std::unique_lock<std::mutex> lock (cloudMutex);
        auto timeOut = (std::chrono::system_clock::now() + std::chrono::milliseconds (1000));
        if (cloudCV.wait_until (lock, timeOut, [&] { return newDataAvailable; }))
        {
            if (filterData)
            {
                // remove invalid points from the cloud
                std::vector<int> indices;
                pcl::removeNaNFromPointCloud (*cloud, *cloud, indices);

                // set up a point cloud filter and show the filtered result
                StatisticalOutlierRemoval<pcl::PointXYZ> outlierRemoval;
                outlierRemoval.setMeanK (50);
                outlierRemoval.setStddevMulThresh (1.0);

                outlierRemoval.setInputCloud (cloud);
                outlierRemoval.filter (*cloudFiltered);


                viewer.showCloud (cloudFiltered);
            }
            else
            {
                viewer.showCloud (cloud);
            }
            newDataAvailable = false;
        }
        else
        {
            // if we ran into a timeout, just sleep for some time
            std::this_thread::sleep_for (std::chrono::milliseconds (100));
        }
    }

    // stop capturing
    if (cameraDevice->stopCapture() != CameraStatus::SUCCESS)
    {
        cerr << "Error stopping the capturing" << endl;
        return 1;
    }

    return 0;
}
