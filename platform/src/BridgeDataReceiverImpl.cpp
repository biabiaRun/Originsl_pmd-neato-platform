/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <BridgeDataReceiverImpl.hpp>

using namespace platform;
using namespace royale::hal;

BridgeDataReceiverImpl::BridgeDataReceiverImpl()
{

}

BridgeDataReceiverImpl::~BridgeDataReceiverImpl()
{

}

void BridgeDataReceiverImpl::setBufferCaptureListener (IBufferCaptureListener *collector)
{
    std::lock_guard<std::mutex> guard (m_changeListenerLock);
    // only save the pointer to the provided collector
    // the framecollector is used for calling the callback
    m_bufferCaptureListener = collector;
}

float BridgeDataReceiverImpl::getPeakTransferSpeed()
{
    return 10.0f; // needs to be changed according to the platform (please read the specification)
}

std::size_t BridgeDataReceiverImpl::executeUseCase (int width, int height, std::size_t preferredBufferCount)
{
    // set the MIPI buffer sizes to the correct new buffer size
    // if we can only work with pre-defined buffer sizes, the interface can also change
    // this would be the most flexible approach of course
    //
    // returns the number of buffers which are successfully allocated
    return preferredBufferCount; // this indicates that the platform has all the buffers we requested
}

void BridgeDataReceiverImpl::startCapture()
{
    // empty implementation
}

void BridgeDataReceiverImpl::stopCapture()
{
    // empty implementation
}

bool BridgeDataReceiverImpl::isConnected() const
{
    return true;
}

void BridgeDataReceiverImpl::queueBuffer (royale::hal::ICapturedBuffer *buffer)
{
    delete buffer;
}

royale::Vector<royale::Pair<royale::String, royale::String>> BridgeDataReceiverImpl::getBridgeInfo()
{
    return royale::Vector<royale::Pair<royale::String, royale::String>>();
}

void BridgeDataReceiverImpl::setEventListener (royale::IEventListener *listener)
{
    // empty implementation
}