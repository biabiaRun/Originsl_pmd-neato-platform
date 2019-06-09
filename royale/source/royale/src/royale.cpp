/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/


#include <royale.hpp>
#include <royale-version.h>
#include <sstream>

using namespace royale;
using namespace std;

void royale::getVersion (unsigned &major, unsigned &minor, unsigned &patch)
{
    major = ROYALE_VERSION_MAJOR;
    minor = ROYALE_VERSION_MINOR;
    patch = ROYALE_VERSION_PATCH;
}

void royale::getVersion (unsigned &major, unsigned &minor, unsigned &patch, unsigned &build)
{
    major = ROYALE_VERSION_MAJOR;
    minor = ROYALE_VERSION_MINOR;
    patch = ROYALE_VERSION_PATCH;
    build = ROYALE_VERSION_BUILD;
}

void royale::getVersion (unsigned &major, unsigned &minor, unsigned &patch, unsigned &build, royale::String &scmRevision)
{
    major = ROYALE_VERSION_MAJOR;
    minor = ROYALE_VERSION_MINOR;
    patch = ROYALE_VERSION_PATCH;
    build = ROYALE_VERSION_BUILD;
    scmRevision = ROYALE_VERSION_SCM;
}

void royale::getVersion(unsigned &major, unsigned &minor, unsigned &patch, unsigned &build, royale::String &customer, royale::String &scmRevision)
{
	major = ROYALE_VERSION_MAJOR;
	minor = ROYALE_VERSION_MINOR;
	patch = ROYALE_VERSION_PATCH;
	build = ROYALE_VERSION_BUILD;
	customer = ROYALE_VERSION_CUSTOMER_SUFFIX;
	scmRevision = ROYALE_VERSION_SCM;
}

royale::String royale::getErrorString (royale::CameraStatus status)
{
    return royale::getStatusString (status);
}
