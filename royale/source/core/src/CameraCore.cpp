/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <device/CameraCore.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/Crc32.hpp>
#include <common/RoyaleLogger.hpp>
#include <device/TemperatureMonitor.hpp>
#include <device/TemperatureSensorCheckAdapter.hpp>
#include <device/PsdTemperatureSensorCheckAdapter.hpp>
#include <common/events/EventCaptureStream.hpp>
#include <collector/BufferActionCalcIndividual.hpp>
#include <collector/BufferActionCalcSuper.hpp>
#include <common/MakeUnique.hpp>
#include <pal/IStorageAccessUnderlying.hpp>

#include <iomanip>

using namespace royale::device;
using namespace royale::common;
using namespace royale::config;
using namespace royale::usecase;
using namespace royale::collector;

CameraCore::CameraCore (std::shared_ptr<const royale::config::ICoreConfig> config,
                        std::shared_ptr<royale::hal::IImager> imager,
                        std::shared_ptr<royale::hal::IBridgeDataReceiver> bridgeReceiver,
                        std::shared_ptr<royale::hal::ITemperatureSensor> tempSensor,
                        std::shared_ptr<royale::hal::INonVolatileStorage> storage,
                        std::shared_ptr<royale::common::IFlowControlStrategy> flowControl,
                        std::shared_ptr<royale::hal::IPsdTemperatureSensor> psdTemperatureSensor,
                        royale::CameraAccessLevel access) :
    m_config (config),
    m_storage (storage),
    m_psdTemperatureSensor (psdTemperatureSensor),
    m_temperatureSensor (tempSensor),
    m_bridgeReceiver (bridgeReceiver),
    m_imager (imager),
    m_frameCollector (nullptr),
    m_flowControl (flowControl),
    m_access (access)
{
    uint16_t lensCenterRow, lensCenterCol;
    config->getLensCenterDesign (lensCenterCol, lensCenterRow);
    m_roiLensCenter.reset (new RoiLensCenter (lensCenterCol, lensCenterRow));

    try
    {
        init ();
    }
    catch (...)
    {
        if (m_bridgeReceiver)
        {
            // CameraCore must call setBufferCaptureListener (nullptr) before destroying the bridge,
            // the ~CameraCore destructor does that, but it won't be called if an exception is
            // thrown from this constructor.
            m_bridgeReceiver->setBufferCaptureListener (nullptr);
        }
        throw;
    }
}

void CameraCore::init ()
{
    // check that each component is available
    if (m_config == nullptr)
    {
        throw InvalidValue ("invalid value", "CoreConfig cannot be null");
    }
    if (m_bridgeReceiver == nullptr)
    {
        throw InvalidValue ("invalid value", "BridgeReceiver cannot be null");
    }
    if (m_temperatureSensor == nullptr)
    {
        if (m_psdTemperatureSensor == nullptr)
        {
            throw InvalidValue ("invalid value", "psdTemperatureSensor cannot be null if there is no other tmperature sensor");
        }
    }
    if (m_storage == nullptr)
    {
        LOG (WARN) << "Storage shouldn't be null";
    }
    if (m_imager == nullptr)
    {
        throw InvalidValue ("invalid value", "Imager cannot be null");
    }

    // Create frame collector
    std::unique_ptr<royale::collector::IBufferActionCalc> bufferActionCalc;
    switch (m_config->getFrameTransmissionMode())
    {
        case FrameTransmissionMode::INDIVIDUAL:
            bufferActionCalc = common::makeUnique<royale::collector::BufferActionCalcIndividual>();
            break;
        case FrameTransmissionMode::SUPERFRAME:
            bufferActionCalc = common::makeUnique<royale::collector::BufferActionCalcSuper>();
            break;
        default:
            throw LogicError ("Frame collector type is not supported");
    }
    m_frameCollector = common::makeUnique<FrameCollectorBase> (m_imager->createPseudoDataInterpreter(), m_bridgeReceiver, std::move (bufferActionCalc));

    if (m_temperatureSensor)
    {
        // Set up temperature monitoring via decorated temperature sensor.
        auto checker = makeUnique<device::TemperatureMonitor> (
                           m_config->getTemperatureLimitSoft(),
                           m_config->getTemperatureLimitHard());
        checker->setEventListener (&m_eventQueueInt);
        auto adapter = std::make_shared<TemperatureSensorCheckAdapter> (m_temperatureSensor, std::move (checker));
        m_temperatureSensor = adapter;
        m_frameCollector->setTemperatureSensor (m_temperatureSensor);
    }
    if (!m_temperatureSensor && m_psdTemperatureSensor)
    {
        auto checker = makeUnique<device::TemperatureMonitor> (
                           m_config->getTemperatureLimitSoft(),
                           m_config->getTemperatureLimitHard());
        checker->setEventListener (&m_eventQueueInt);
        auto adapter = std::make_shared<PsdTemperatureSensorCheckAdapter> (m_psdTemperatureSensor, std::move (checker));
        m_psdTemperatureSensor = adapter;
        m_frameCollector->setTemperatureSensor (m_psdTemperatureSensor);
    }

    m_bridgeReceiver->setBufferCaptureListener (m_frameCollector.get());

    // power-cycle the imager, so that it's in a known state
    m_imager->sleep();
    m_imager->wake();

    //m_config->setEventListener(&m_eventQueue);
    m_bridgeReceiver->setEventListener (&m_eventQueueInt);
    //m_temperatureSensor->setEventListener(&m_eventQueue);
    //m_storage->setEventListener(&m_eventQueue);
    //m_imager->setEventListener(&m_eventQueue);
    m_frameCollector->setEventListener (&m_eventQueueInt);

    m_eventQueueInt.setEventListener (this);

    m_isCapturing = false;
}


void CameraCore::initializeImager()
{
    // initialize imager
    m_imager->initialize();
}

CameraCore::~CameraCore()
{
    if (m_isCapturing)
    {
        try
        {
            stopCapture();
        }
        catch (...)
        {
            // Silently catch any exception
        }
    }
    m_frameCollector->setEventListener (nullptr);
    m_bridgeReceiver->setEventListener (nullptr);
    m_bridgeReceiver->setBufferCaptureListener (nullptr);
    m_frameCollector->releaseAllBuffers();
}

void CameraCore::startCapture()
{
    m_imager->startCapture();
    m_bridgeReceiver->startCapture();
    m_isCapturing = true;
}

void CameraCore::stopCapture()
{
    // Even if something goes wrong we will assume
    // the capturing was stopped
    m_isCapturing = false;
    try
    {
        // This may fail if we've lost our USB device.
        m_bridgeReceiver->stopCapture();
        m_imager->stopCapture();
    }
    catch (const Exception &e)
    {
        std::unique_ptr<royale::IEvent> event (new royale::event::EventCaptureStream (royale::EventSeverity::ROYALE_ERROR, e.getUserDescription()));
        m_eventQueueInt.onEvent (std::move (event));
        throw;
    }
}

royale::usecase::VerificationStatus CameraCore::verifyUseCase (const royale::usecase::UseCaseDefinition *useCase)
{
    if (useCase == nullptr)
    {
        LOG (ERROR) << "UseCaseDefinition cannot be null";
        return VerificationStatus::UNDEFINED;
    }

    // check module parameters

    // modulation frequency limit is checked by the imager

    // check if the image size is within the allowed maximal size
    uint16_t ucWidth, ucHeight;
    useCase->getImage (ucWidth, ucHeight);
    uint16_t maxWidth = m_config->getMaxImageWidth();
    uint16_t maxHeight = m_config->getMaxImageHeight();
    if (ucWidth > maxWidth || ucHeight > maxHeight)
    {
        LOG (ERROR) << "Specified image size is bigger than the maximal allowed image size";
        return VerificationStatus::REGION;
    }

    uint16_t roiCMin, roiRMin;
    const auto lensRet = m_roiLensCenter->verifyUseCase (*useCase);
    if (lensRet != VerificationStatus::SUCCESS)
    {
        LOG (ERROR) << "UseCase " << useCase->getTypeName() << " is not supported by the lens calibration, error " << (int) lensRet;
        return lensRet;
    }

    const auto fcRet = m_frameCollector->verifyUseCase (*useCase);
    if (fcRet != VerificationStatus::SUCCESS)
    {
        LOG (ERROR) << "UseCase " << useCase->getTypeName() << " is not supported by the frame collector, error " << (int) fcRet;
        return fcRet;
    }

    uint16_t flowControlRate = IFlowControlStrategy::NoFlowControl;
    if (m_flowControl)
    {
        flowControlRate = m_flowControl->getRawFrameRate (*useCase);
    }

    m_roiLensCenter->getRoiCorner (*useCase, roiCMin, roiRMin);
    const auto ret = m_imager->verifyUseCase (*useCase, roiCMin, roiRMin, flowControlRate);
    if (ret == VerificationStatus::SUCCESS)
    {
        LOG (INFO) << "UseCase " << useCase->getTypeName() << " is supported by camera core";
    }
    else
    {
        LOG (ERROR) << "UseCase " << useCase->getTypeName() << " is not supported by the used imager, error " << (int) ret;
    }
    return ret;
}

void CameraCore::setUseCase (royale::usecase::UseCaseDefinition *useCase, UpdateComponent component)
{
    if (useCase == nullptr)
    {
        LOG (ERROR) << "UseCaseDefinition cannot be null";
        throw RuntimeError ("UseCaseDefinition cannot be null");
    }

    // check if use case is supported
    auto verificationStatus = verifyUseCase (useCase);
    if (verificationStatus != VerificationStatus::SUCCESS)
    {
        throw Exception ("use case not supported");
    }

    // bring camera to ready state before setting up new use case
    auto activeModeEnabled = false;
    if (isCapturing())
    {
        activeModeEnabled = true;
        stopCapture();
    }

    // update usecase on different components
    if (static_cast<uint16_t> (component) & static_cast<uint16_t> (UpdateComponent::Imager))
    {
        uint16_t roiCMin, roiRMin;
        m_roiLensCenter->getRoiCorner (*useCase, roiCMin, roiRMin);

        uint16_t flowControlRate = IFlowControlStrategy::NoFlowControl;
        if (m_flowControl)
        {
            flowControlRate = m_flowControl->getRawFrameRate (*useCase);
        }

        m_imager->executeUseCase (*useCase, roiCMin, roiRMin, flowControlRate);
    }

    if (static_cast<uint16_t> (component) & static_cast<uint16_t> (UpdateComponent::FrameCollector))
    {
        auto blockSizes = m_imager->getMeasurementBlockSizes();
        m_frameCollector->executeUseCase (*m_bridgeReceiver, useCase, blockSizes);
    }

    // if the camera was capturing before, bring again into capture mode
    if (activeModeEnabled)
    {
        startCapture();
    }
}

std::string CameraCore::getImagerSerialNumber() const
{
    return m_imager->getSerialNumber();
}

std::vector<uint8_t> CameraCore::getCalibrationData() const
{
    if (m_storage == nullptr)
    {
        throw LogicError ("Device doesn't have non-volatile storage");
    }
    return m_storage->getCalibrationData().toStdVector();
}

void CameraCore::flashCalibrationData (const std::vector<uint8_t> &data)
{
    if (!m_storage)
    {
        LOG (ERROR) << "Trying to flash but there is no flash";
        throw LogicError ("Trying to flash but there is no flash");
    }
    m_storage->writeCalibrationData (data);
}

void CameraCore::flashData (const std::vector<uint8_t> &data)
{
    if (!m_storage)
    {
        LOG (ERROR) << "Trying to flash but there is no flash";
        throw LogicError ("Trying to flash but there is no flash");
    }
    auto underlyingAccess = std::dynamic_pointer_cast<royale::pal::IStorageAccessUnderlying> (m_storage);
    if (!underlyingAccess)
    {
        LOG (ERROR) << "Trying to flash but this device's storage doesn't support this";
        throw LogicError ("Trying to flash but this device's storage doesn't support this");
    }
    auto writeAccess = underlyingAccess->getUnderlyingWriteAccess();
    if (!writeAccess)
    {
        LOG (ERROR) << "Trying to flash but this device's storage isn't writable like this";
        throw LogicError ("Trying to flash but this device's storage isn't writable like this");
    }
    writeAccess->writeStorage (data);
}

bool CameraCore::isCapturing() const
{
    return m_isCapturing;
}

bool CameraCore::isConnected() const
{
    return m_bridgeReceiver->isConnected();
}

IFrameCaptureReleaser *CameraCore::getCaptureReleaser()
{
    return m_frameCollector.get();
}

void CameraCore::setCaptureListener (IFrameCaptureListener *listener)
{
    m_frameCollector->setCaptureListener (listener);
}

void CameraCore::setEventListener (royale::IEventListener *listener)
{
    m_eventQueueExt.setEventListener (listener);
}

void CameraCore::reconfigureImagerExposureTimes (const royale::usecase::UseCaseDefinition *useCase, const std::vector<uint32_t> &exposureTimes)
{
    uint16_t lastReconfigIndex = 0;
    if (m_frameCollector->pendingReportedExposureTimes())
    {
        throw RuntimeError ("Busy", "Busy", royale::CameraStatus::DEVICE_IS_BUSY);
    }

    std::vector<uint32_t> imagerExposureTimes;
    for (const auto &rfs : useCase->getRawFrameSets())
    {
        imagerExposureTimes.push_back (exposureTimes.at (rfs.exposureGroupIdx));
    }

    m_imager->reconfigureExposureTimes (imagerExposureTimes, lastReconfigIndex);
    m_frameCollector->setReportedExposureTimes (lastReconfigIndex, exposureTimes);
}

void CameraCore::reconfigureImagerTargetFrameRate (uint16_t targetFrameRate)
{
    uint16_t lastReconfigIndex = 0;
    if (m_frameCollector->pendingReportedExposureTimes())
    {
        throw RuntimeError ("Busy", "Busy", royale::CameraStatus::DEVICE_IS_BUSY);
    }
    m_imager->reconfigureTargetFrameRate (targetFrameRate, lastReconfigIndex);
    // Do we need to lock out further changes here?
}

void CameraCore::setLensOffset (int16_t pixelColumn, int16_t pixelRow)
{
    m_roiLensCenter->setLensOffset (pixelColumn, pixelRow);
}

void CameraCore::writeRegisters (const royale::Vector<royale::Pair<royale::String, uint64_t>> &registers)
{
    m_imager->writeRegisters (registers);
}

void CameraCore::readRegisters (royale::Vector<royale::Pair<royale::String, uint64_t>> &registers)
{
    m_imager->readRegisters (registers);
}

royale::Vector<royale::Pair<royale::String, royale::String>> CameraCore::getBridgeInfo()
{
    return m_bridgeReceiver->getBridgeInfo();
}

royale::Vector<royale::Pair<royale::String, royale::String>> CameraCore::getStorageInfo()
{
    decltype (getStorageInfo()) info;

    if (m_storage)
    {
        std::stringstream ss;
        auto moduleId = m_storage->getModuleIdentifier();
        for (auto curInt : moduleId)
        {
            ss << std::setfill ('0') << std::setw (2) << std::hex << static_cast<unsigned> (curInt);
        }
        info.emplace_back ("MODULE_IDENTIFIER", ss.str().c_str());

        auto moduleIdHash = calculateCRC32 (moduleId.data(), moduleId.size());
        std::stringstream ss2;
        ss2 << moduleIdHash;

        info.emplace_back ("MODULE_IDENTIFIER_HASH", ss2.str().c_str());
        info.emplace_back ("MODULE_SERIAL", m_storage->getModuleSerialNumber());
        info.emplace_back ("MODULE_SUFFIX", m_storage->getModuleSuffix());
    }
    else
    {
        info.emplace_back ("MODULE_STORAGE", "UNAVAILABLE");
    }

    return info;
}

void CameraCore::setExternalTrigger (bool useExternalTrigger)
{
    m_imager->setExternalTrigger (useExternalTrigger);
}

void CameraCore::onEvent (std::unique_ptr<royale::IEvent> &&event)
{
    if (m_isCapturing &&
            (event->type() == EventType::ROYALE_EYE_SAFETY ||
             (event->type() == EventType::ROYALE_OVER_TEMPERATURE &&
              event->severity() == EventSeverity::ROYALE_ERROR)))
    {
        LOG (ERROR) << "Stopping capture because of event: " << event->describe();
        if (m_access == CameraAccessLevel::L1 || m_access == CameraAccessLevel::L2)
        {
            stopCapture();
        }
    }

    m_eventQueueExt.onEvent (std::move (event));
}

royale::Vector<royale::Pair<royale::String, royale::String>> CameraCore::getCoreInfo()
{
    auto coreInfo = getBridgeInfo();
    auto storageInfo = getStorageInfo();

    for (const auto &curInfo : storageInfo)
    {
        coreInfo.emplace_back (curInfo);
    }

    // Add the information that was supplied with addCoreInfo calls
    for (const auto &curInfo : m_additionalCoreInfo)
    {
        coreInfo.emplace_back (curInfo);
    }

    return coreInfo;
}

void CameraCore::addToCoreInfo (const royale::Pair<royale::String, royale::String> &info)
{
    m_additionalCoreInfo.emplace_back (info);
}
