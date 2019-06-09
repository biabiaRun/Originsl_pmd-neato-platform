/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <opencv2/opencv.hpp>

using namespace cv;

// During CMake's build script generation, CMake will check whether this compiles.  If it fails,
// then sampleOpenCV is expected to also fail, as described in sampleOpenCV.cpp.
int main (int, char **)
{
    namedWindow ("compilation test", WINDOW_AUTOSIZE);
    return 0;
}
