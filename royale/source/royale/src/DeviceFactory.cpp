/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <DeviceFactory.hpp>

#include <device/CameraDevice.hpp>
#include <device/CameraCore.hpp>

#include <hal/ITemperatureSensor.hpp>
#include <collector/IFrameCollector.hpp>
#include <record/CameraRecord.hpp>

#ifdef USE_SPECTRE
#include <processing/ProcessingSpectre.hpp>
#endif

#include <processing/ProcessingSimple.hpp>

using royale::device::CameraCore;
using royale::device::CameraDevice;
using namespace royale::processing;

/**
* Creates an ICameraDevice using the coreBuilder.
*/
std::unique_ptr<royale::ICameraDevice> royale::DeviceFactory::createCameraDevice (
    royale::CameraAccessLevel accessLevel,
    royale::factory::ICameraCoreBuilder &coreBuilder)
{
    coreBuilder.setAccessLevel (accessLevel);

    auto cameraCore = coreBuilder.createCameraCore();
    auto coreConfig = coreBuilder.getICoreConfig();

    auto serialNumber = cameraCore->getImagerSerialNumber();

    std::shared_ptr<royale::processing::IProcessing> processing;

    auto cameraName = coreConfig->getCameraName();

#ifdef USE_SPECTRE
    processing.reset (new ProcessingSpectre (cameraCore->getCaptureReleaser()));
#else
    processing.reset (new ProcessingSimple (cameraCore->getCaptureReleaser()));
#endif

    auto imagerType = coreBuilder.getImagerType();

    std::unique_ptr<CameraDevice> cameraDevice
    {
        new CameraDevice (
            accessLevel,
            serialNumber,
            std::move (cameraCore),
            coreConfig,
            processing)
    };

    // setup recording engine
    std::unique_ptr<royale::IRecord> recording
    {
        new royale::record::CameraRecord (cameraDevice.get(), processing.get(), nullptr, cameraName, imagerType)
    };
    cameraDevice->setRecordingEngine (std::move (recording));
    return std::move (cameraDevice);
}
