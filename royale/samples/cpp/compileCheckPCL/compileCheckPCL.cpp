/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <pcl/visualization/cloud_viewer.h>

using namespace pcl;
using namespace pcl::visualization;

// During CMake's build script generation, CMake will check whether this compiles.  If it fails,
// then samplePCL is expected to also fail.
int main (int, char **)
{
    CloudViewer viewer ("Cloud Viewer");
    return 0;
}
