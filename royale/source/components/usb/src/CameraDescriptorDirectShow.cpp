/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/descriptor/CameraDescriptorDirectShow.hpp>

#include <memory>

using namespace royale::usb::bridge;
using namespace royale::usb::descriptor;

CameraDescriptorDirectShow::CameraDescriptorDirectShow (WrappedComPtr<IMoniker> device) :
    m_dev {device}
{
}

WrappedComPtr<IMoniker> CameraDescriptorDirectShow::getDevice()
{
    return m_dev;
}

CameraDescriptorDirectShow::~CameraDescriptorDirectShow()
{
}
