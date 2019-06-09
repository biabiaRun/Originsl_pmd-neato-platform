/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/bridge/BridgeUvcV4l.hpp>

#include <common/exceptions/Disconnected.hpp>
#include <common/IntegerMath.hpp>
#include <common/RoyaleLogger.hpp>

using namespace royale::common;
using namespace royale::usb::bridge;
using namespace royale::v4l::bridge;

namespace
{
    /**
     * Number of buffers to handle most mixed modes even with FrameTransmissionMode::INDIVIDUAL.
     *
     * The number of buffers allocated may be different to this number, as per the documentation of
     * the VIDIOC_REQBUFS ioctl.
     */
    const std::size_t HARDCODED_INDIVIDUAL_BUFFER_COUNT = 28;

    /**
     * Smaller number of buffers that will handle most 2-stream mixed modes with superframes
     */
    const std::size_t HARDCODED_SUPERFRAME_BUFFER_COUNT = 6;

    /**
     * The largest imageHeight (including pseudodata) for the usual resolution on various imagers
     * when using FrameTransmissionMode::INDIVIDUAL.  If the imageHeight is more than the maximum of
     * this list then the bridge assumes that the device is using superframes.
     *
     * The typical heights are 169, 173 and 288 lines.  The logic here relies on the smallest
     * individual height being at least half of the largest, so that a 2-frame superframe from the
     * smallest imager doesn't fit in the MAX_INDIVIDUAL_HEIGHT.
     *
     * Assumption: the ROI (Region of Interest) is always the full size supported by the imager.
     */
    const int MAX_INDIVIDUAL_HEIGHT = 288;

    /**
     * Given that the imager is in superframe mode, and that the first UseCase chosen needs
     * a given imageHeight, assume that multiplying by this number gives a buffer large enough for
     * any UseCase's superframe.
     *
     * Assumptions: the UseCase with the smallest imageHeight needs a 5-frame superframe (either
     * for a 5 phase mode, or for a 9 phase mode that splits ES1 and ES2). The UseCase with the
     * largest imageHeight is the 11-phase calibration use case.
     */
    const int RATIO_MIN_MAX_SUPERFRAME = static_cast<int> (divideRoundUp (11, 5));

    /**
     * Sanity check in case the first executeUseCase call is for a 9-phase or 11-phase use case, so
     * that the image height doesn't end up being RATIO_MIN_MAX_SUPERFRAME times the largest use
     * case in the imager. For the 100kpixel imager that would exceed the 4050kB dwMaxVideoFrameSize
     * in the device's USB descriptor.
     */
    const int MAX_SUPERFRAME_HEIGHT = 11 * MAX_INDIVIDUAL_HEIGHT;
}

BridgeUvcV4l::BridgeUvcV4l (const std::string &filename) :
    BridgeV4l {filename},
    m_bufferCount {0},
    m_bufferHeight {0},
    m_captureStarted {false}
{
}

BridgeUvcV4l::~BridgeUvcV4l() = default;

std::size_t BridgeUvcV4l::executeUseCase (int imageWidth, int imageHeight, std::size_t bufferCount)
{
    // The caller will ensure that there aren't two simultaneous calls to executeUseCase(), so there
    // is no need to have a lock for accessing m_bufferCount.  The caller (the FrameCollector) will
    // also print an error LOG message if too few buffers are allocated, so no LOG is printed for
    // that in this function.
    if (m_bufferCount == 0)
    {
        if (imageHeight > MAX_SUPERFRAME_HEIGHT)
        {
            // Pass the request directly through to the BridgeV4l, assuming that this is the
            // largest superframe size needed for the device.  Only expected to be reached if the
            // first use case is more than 11 phases and the imager has 100kpixels.
            LOG (WARN) << "Using very large superframes";
            bufferCount = HARDCODED_SUPERFRAME_BUFFER_COUNT;
        }
        else if (imageHeight > MAX_INDIVIDUAL_HEIGHT)
        {
            bufferCount = HARDCODED_SUPERFRAME_BUFFER_COUNT;
            imageHeight = std::min(RATIO_MIN_MAX_SUPERFRAME * imageHeight, MAX_SUPERFRAME_HEIGHT);
        }
        else
        {
            bufferCount = HARDCODED_INDIVIDUAL_BUFFER_COUNT;
        }
        LOG (WARN) << "Buffer allocation overridden by BridgeUvcV4l, using " << bufferCount << " buffers of height " << imageHeight;
        m_bufferCount = BridgeV4l::executeUseCase (imageWidth, imageHeight, bufferCount);
        m_bufferHeight = imageHeight;
    }
    else if (imageHeight > m_bufferHeight)
    {
        // One of the assumptions about the use cases and RATIO_MIN_MAX_SUPERFRAME is wrong
        LOG (ERROR) << "imageHeight exceeds BridgeUvcV4l's chosen height";
        // There are no allocated buffers of the requested size.
        return 0;
    }
    else
    {
        LOG (WARN) << "Buffer reallocation overridden by BridgeUvcV4l, keeping " << m_bufferCount << " already allocated buffers";
    }

    return m_bufferCount;
}

void BridgeUvcV4l::startCapture()
{
    if (!m_captureStarted)
    {
        BridgeV4l::startCapture();
        m_captureStarted = true;
    }
}

void BridgeUvcV4l::stopCapture()
{
    // always a no-op, the only call to BridgeV4l::stopCapture() is done in closeConnection
}

void BridgeUvcV4l::closeConnection()
{
    BridgeV4l::stopCapture();
    BridgeV4l::closeConnection();

    // reset these, so that a subsequent openConnection would be supported (untested)
    m_bufferCount = 0;
    m_bufferHeight = 0;
    m_captureStarted = false;
}

void BridgeUvcV4l::setUsbSpeed (royale::usb::pal::UsbSpeed speed)
{
    m_usbSpeed = speed;
}

float BridgeUvcV4l::getPeakTransferSpeed()
{
    if (!isConnected())
    {
        throw Disconnected();
    }

    // Assume the worst case of 16 bits per pixel, instead of the detected format.  Further testing
    // of the flow control is needed before changing to using getTransferFormat().
    return royale::usb::pal::UsbSpeedUtils::calculatePeakTransferSpeed (m_usbSpeed, royale::buffer::BufferDataFormat::RAW16);
}

void BridgeUvcV4l::setUvcBridgeInfo (royale::Vector<royale::Pair<royale::String, royale::String>> &&info)
{
    m_uvcBridgeInfo = info;
}

royale::Vector<royale::Pair<royale::String, royale::String>> BridgeUvcV4l::getBridgeInfo()
{
    auto info = BridgeV4l::getBridgeInfo();
    for (const auto &x : m_uvcBridgeInfo)
    {
        info.emplace_back (x);
    }
    info.emplace_back ("V4L_BRIDGE_SUB_TYPE", "UVC");
    return info;
}
