/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/bridge/BridgeUvcDirectShow.hpp>
#include <buffer/BufferUtils.hpp>
#include <usb/descriptor/CameraDescriptorDirectShow.hpp>
#include <buffer/OffsetBasedCapturedBuffer.hpp>

#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/Disconnected.hpp>

#include <common/events/EventCaptureStream.hpp>

#include <common/MakeUnique.hpp>

#include <common/NarrowCast.hpp>
#include <common/RoyaleLogger.hpp>

#include <algorithm>
#include <atomic>

#include <initguid.h>

using namespace royale::common;
using namespace royale::buffer;
using namespace royale::usb::descriptor;
using namespace royale::usb::bridge;
using namespace royale::usb::pal;
using royale::common::makeUnique;
using std::size_t;

// The classes in this namespace are provided in the platform libraries, but the
// headers are no longer in the SDKs.  Microsoft's MSDN forums themselves
// suggest copying the old header contents as a workaround.
namespace
{
    /**
     * ID for a bit-bucket filter at the end of the acquisition graph.
     * This is exported by strmiids.lib, but not declared in strmiids.h
     */
    EXTERN_C const CLSID CLSID_NullRenderer;

    MIDL_INTERFACE ("0579154A-2B53-4994-B0D0-E773148EFF85")
ISampleGrabberCB :
    public IUnknown
    {
public:
        virtual HRESULT STDMETHODCALLTYPE SampleCB (
            double SampleTime,
            IMediaSample * pSample) = 0;

        virtual HRESULT STDMETHODCALLTYPE BufferCB (
            double SampleTime,
            BYTE * pBuffer,
            long BufferLen) = 0;

    };

    /** Argument to ISampleGrabber::SetCallback to choose ISampleGrabberCB::SampleCB */
    const long SG_CB_METHOD = 0;

    MIDL_INTERFACE ("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
ISampleGrabber :
    public IUnknown
    {
public:
        virtual HRESULT STDMETHODCALLTYPE SetOneShot (
            BOOL OneShot) = 0;

        virtual HRESULT STDMETHODCALLTYPE SetMediaType (
            const AM_MEDIA_TYPE * pType) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType (
            AM_MEDIA_TYPE * pType) = 0;

        virtual HRESULT STDMETHODCALLTYPE SetBufferSamples (
            BOOL BufferThem) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer (
            /* [out][in] */ long * pBufferSize,
            /* [out] */ long * pBuffer) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetCurrentSample (
            /* [retval][out] */ IMediaSample **ppSample) = 0;

        virtual HRESULT STDMETHODCALLTYPE SetCallback (
            ISampleGrabberCB * pCallback,
            long WhichMethodToCallback) = 0;

    };

    EXTERN_C const CLSID CLSID_SampleGrabber;
    EXTERN_C const IID IID_ISampleGrabber;
}

/**
 * This class receives a callback for each frame captured by the UVC framework.
 *
 * It is a friend of BridgeUvcDirectShow, so that it can call Bridge->bufferCallback().
 */
class BridgeUvcDirectShow::AcquisitionReceiver : public ISampleGrabberCB
{
public:
    explicit AcquisitionReceiver (BridgeUvcDirectShow *bridge) :
        m_refcount {1},
        m_bridge {bridge}
    {
    }

    ~AcquisitionReceiver ()
    {
    }

    HRESULT STDMETHODCALLTYPE SampleCB (
        double SampleTime,
        IMediaSample *sample) override
    {
        uint8_t *data = nullptr;
        auto hr = sample->GetPointer (&data);
        if (FAILED (hr))
        {
            LOG (ERROR) << "Can't access data of captured MediaSample";
            return hr;
        }
        const auto sampleSize = static_cast<size_t> (sample->GetActualDataLength());

        // \todo ROYAL-1927 timestamp (has never been supported by BridgeUvcDirectShow)

        m_bridge->bridgeAcquisitionCallback (sampleSize, data, 0);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE BufferCB (
        double SampleTime,
        BYTE *pBuffer,
        long BufferLen) override
    {
        LOG (ERROR) << "Unexpected call to BufferCB";
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef () override
    {
        return InterlockedIncrement (&m_refcount);
    }

    ULONG STDMETHODCALLTYPE Release () override
    {
        auto count = InterlockedDecrement (&m_refcount);
        if (count == 0)
        {
            delete this;
        }
        return count;
    }

    STDMETHODIMP QueryInterface (const IID &rrid, void **ppvObject) override
    {
        if (ppvObject == nullptr)
        {
            return E_POINTER;
        }
        if (rrid == __uuidof (IUnknown))
        {
            *ppvObject = static_cast<IUnknown *> (this);
        }
        else if (rrid == __uuidof (ISampleGrabberCB))
        {
            *ppvObject = static_cast<ISampleGrabberCB *> (this);
        }
        else
        {
            *ppvObject = nullptr;
            return E_NOTIMPL;
        }
        AddRef();
        return S_OK;
    }

private:
    volatile unsigned int m_refcount;
    /** Where the callback goes */
    BridgeUvcDirectShow *m_bridge;
};

BridgeUvcDirectShow::BridgeUvcDirectShow (std::unique_ptr<CameraDescriptorDirectShow> desc) :
    m_graphHandle {},
    m_mediaControl {}
{
    if (desc == nullptr)
    {
        throw LogicError ("CameraDescriptor is null");
    }

    m_unopenedDevice = desc->getDevice();
    // cameraDescriptor is released when the method returns
}

bool BridgeUvcDirectShow::isConnected() const
{
    return m_graphHandle && m_isConnected;
}

BridgeUvcDirectShow::~BridgeUvcDirectShow()
{
    if (isConnected())
    {
        try
        {
            closeConnection();
        }
        catch (...)
        {
        }
    }
}

void BridgeUvcDirectShow::openConnection ()
{
    if (isConnected ())
    {
        LOG (ERROR) << "bridge already connected";
        throw LogicError ("bridge already connected");
    }

    HRESULT hr;

    WrappedComPtr<IGraphBuilder> graph;
    WrappedComPtr<ICaptureGraphBuilder2> builder;
    hr = CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, graph.getRef ());
    if (SUCCEEDED (hr))
    {
        hr = CoCreateInstance (CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, builder.getRef ());
        if (SUCCEEDED (hr))
        {
            hr = builder->SetFiltergraph (graph.get ());
        }
    }
    if (FAILED (hr))
    {
        throw CouldNotOpen ("Unable to create media graph");
    }

    WrappedComPtr<IBaseFilter> videoSourceFilter;
    hr = m_unopenedDevice->BindToObject (NULL, NULL, IID_IBaseFilter, videoSourceFilter.getRef());
    if (FAILED (hr))
    {
        throw CouldNotOpen ("Unable to open the UVC source");
    }

    hr = graph->AddFilter (videoSourceFilter.get (), L"USB Video Source");
    if (FAILED (hr))
    {
        throw CouldNotOpen ("Couldn't add USB Video Source to graph");
    }


    // block for scoping the videoStreamControl
    {
        WrappedComPtr<IAMStreamConfig> videoStreamControl;
        hr = builder->FindInterface (&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, videoSourceFilter.get(), IID_IAMStreamConfig, videoStreamControl.getRef ());
        if (FAILED (hr))
        {
            throw CouldNotOpen ("Couldn't get Video stream control");
        }

        AM_MEDIA_TYPE *pMt;
        hr = videoStreamControl->GetFormat (&pMt);
        if (FAILED (hr))
        {
            throw CouldNotOpen ("Couldn't get Video stream format");
        }

        pMt->bFixedSizeSamples = FALSE;
        hr = videoStreamControl->SetFormat (pMt);

        if (FAILED (hr))
        {
            throw CouldNotOpen ("Couldn't set Video stream format");
        }
    }

    // Because the data isn't really MJPG, it needs to be captured by class in the filter (not
    // the render) part of the pipeline (and then not decoded afterwards by the system filter).
    WrappedComPtr<IBaseFilter> sampleGrabberFilter;
    hr = CoCreateInstance (CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, sampleGrabberFilter.getRef ());
    if (SUCCEEDED (hr))
    {
        hr = graph->AddFilter (sampleGrabberFilter.get (), L"Sample Grabber");
        if (SUCCEEDED (hr))
        {
            WrappedComPtr<ISampleGrabber> sampleGrabber;
            hr = sampleGrabberFilter->QueryInterface (IID_ISampleGrabber, sampleGrabber.getRef ());
            if (SUCCEEDED (hr))
            {
                sampleGrabber->SetOneShot (FALSE);
                sampleGrabber->SetCallback (new AcquisitionReceiver (this), SG_CB_METHOD);
            }
        }
    }
    if (FAILED (hr))
    {
        throw CouldNotOpen ("Couldn't add sample grabber");
    }

    // use Null Renderer as we do not want to display the data
    WrappedComPtr<IBaseFilter> nullRenderer;
    hr = CoCreateInstance (CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, nullRenderer.getRef ());
    if (SUCCEEDED (hr))
    {
        hr = graph->AddFilter (nullRenderer.get (), L"Null Renderer");
    }
    if (FAILED (hr))
    {
        throw CouldNotOpen ("Couldn't add null renderer to the pipeline");
    }

    hr = builder->RenderStream (&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, videoSourceFilter.get(), sampleGrabberFilter.get(), nullRenderer.get());
    if (FAILED (hr))
    {
        throw CouldNotOpen ("Couldn't connect pins of graph");
    }

    // Set the Reference Clock of the Graph to NULL to make it run as fast as possible
    {
        WrappedComPtr<IMediaFilter> pMediaFilter;
        hr = graph->QueryInterface (IID_IMediaFilter, pMediaFilter.getRef());
        if (SUCCEEDED (hr))
        {
            hr = pMediaFilter->SetSyncSource (NULL);
        }
        if (FAILED (hr))
        {
            throw CouldNotOpen ("Couldn't unset the sync source");
        }
    }

    m_isConnected = true;

    hr = graph->QueryInterface (IID_IMediaControl, m_mediaControl.getRef ());
    if (FAILED (hr))
    {
        throw CouldNotOpen ("Couldn't get the MediaControl (start failed)");
    }

    // Start the capture at the OS layer immediately.  Multiple applications can open the UVC
    // device, but capturing is an exclusive operation.
    //
    // The imager shouldn't be sending any data yet.  Even if it's already running from a previous
    // run of Royale then the check in the callback code (in BridgeCopyAndNormalize) for
    // m_captureStarted will discard any data sent.
    hr = m_mediaControl->Run ();
    if (FAILED (hr))
    {
        throw CouldNotOpen ("Couldn't start the MediaControl (can't take exclusive ownership of the UVC device)");
    }

    // Keep the reference needed for getMediaSource
    WrappedComPtr<IUnknown> topologyInfoForMediaSource { videoSourceFilter.get() };
    swap (m_topologyInfoForMediaSource, topologyInfoForMediaSource);
    // Keep the graph for the full lifecycle of this Bridge, and make isConnected() return true
    swap (m_graphHandle, graph);
}

WrappedComPtr<IUnknown> BridgeUvcDirectShow::getMediaSource()
{
    WrappedComPtr<IUnknown> unknownPtr;
    m_topologyInfoForMediaSource->QueryInterface (IID_IUnknown, unknownPtr.getRef());
    return unknownPtr;
}

void BridgeUvcDirectShow::closeConnection()
{
    stopCapture();

    if (m_mediaControl)
    {
        auto hr = m_mediaControl->Stop ();
        if (FAILED (hr))
        {
            LOG (ERROR) << "Couldn't stop the MediaControl";
            // This wasn't expected to fail, and it's not obvious if there is any circumstance that
            // could reach this.  Continue to release the graph, it seems the best recovery method.
        }
        m_mediaControl.reset();
    }
    m_graphHandle.reset();
    m_isConnected = false;
}

float BridgeUvcDirectShow::getPeakTransferSpeed()
{
    if (!m_graphHandle)
    {
        m_isConnected = false;
        throw Disconnected();
    }

    // Assume the worst case of 16 bits per pixel, instead of the detected format.  Further testing
    // of the flow control is needed before changing to using getTransferFormat().
    return UsbSpeedUtils::calculatePeakTransferSpeed (m_usbSpeed, royale::buffer::BufferDataFormat::RAW16);
}

void BridgeUvcDirectShow::setUsbSpeed (royale::usb::pal::UsbSpeed speed)
{
    m_usbSpeed = speed;
}
