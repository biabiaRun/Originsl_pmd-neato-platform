/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/MakeUnique.hpp>
#include <common/SensorRoutingConfigI2c.hpp>
#include <config/CoreConfig.hpp>
#include <config/FlowControlStrategyFixed.hpp>
#include <config/TemperatureSensorConfig.hpp>
#include <device/CameraCore.hpp>
#include <device/CameraDevice.hpp>
#include <imager/M2452/PseudoDataInterpreter.hpp>
#include <pal/II2cBusAccess.hpp>
#include <pal/Access2I2cDeviceAdapter.hpp>
#include <factory/CoreConfigFactory.hpp>
#include <factory/ImagerFactory.hpp>
#include <factory/ProcessingParameterMapFactory.hpp>
#include <usecase/UseCaseEightPhase.hpp>
#include <storage/StorageFormatPolar.hpp>
#include <storage/StorageI2cEeprom.hpp>
#include <storage/NonVolatileStoragePersistent.hpp>
#include <temperature/TemperatureSensorTMP102.hpp>
#include <processing/ProcessingSpectre.hpp>
#include <SensorMap.hpp>
#include <modules/CommonProcessingParameters.hpp>
#include <common/RoyaleLogger.hpp>

#ifdef ROYALE_ENABLE_PLATFORM_CODE
#include <royalev4l/bridge/BridgeV4l.hpp>
#include <royalev4l/PixelFormat.hpp>
#include <buffer/BufferDataFormat.hpp>
#endif

#include <ModuleConfigCustom.hpp>

#include <BridgeImagerImpl.hpp>
#include <I2cAccessImpl.hpp>

#include <string>
#include <thread>
#include <chrono>
#include <memory>

#include <BaseConfig.hpp>
#include <ModuleConfigCustom.hpp>
#include <CameraFactory.hpp>

#include <record/CameraRecord.hpp>

#ifdef ROYALE_FACTORY_PLAYBACK
#include <record/CameraPlayback.hpp>
#endif

#ifndef ROYALE_ENABLE_PLATFORM_CODE
#include <royale/CameraManager.hpp>
#endif

using namespace platform;
using namespace royale;
using namespace royale::hal;
using namespace royale::pal;
using namespace royale::usecase;
using namespace royale::imager;
using namespace royale::processing;
using namespace royale::config;
using namespace royale::device;
using namespace royale::collector;
using namespace royale::common;
using namespace royale::storage;
using namespace royale::sensors;
using namespace royale::factory;


#ifdef ROYALE_ENABLE_PLATFORM_CODE
using namespace royale::buffer;
using namespace royale::v4l;
#endif

namespace
{
    std::shared_ptr<royale::factory::IProcessingParameterMapFactory> getProcessingParameterFactory()
    {
        using namespace royale;
        using namespace royale::moduleconfig;

        const std::vector<uint8_t> emptyProductId{};
        const royale::Vector<factory::ProcessingParameterMapFactory::value_type> defaultVector
        {
            { { emptyProductId, CommonId2Frequencies }, { CommonProcessingParams2Frequencies } },
            { { emptyProductId, CommonId1Frequency }, { CommonProcessingParams1Frequency } },
            { { emptyProductId, CommonIdMixedEsHt }, { CommonProcessingParams2Frequencies, CommonProcessingParams1Frequency } },
            { { emptyProductId, CommonIdMixedHtEs }, { CommonProcessingParams1Frequency, CommonProcessingParams2Frequencies } },

            { { emptyProductId, CommonIdLowNoiseExtended }, { CommonProcessingParamsLowNoiseExtended } },
            { { emptyProductId, CommonIdVideoExtended }, { CommonProcessingParamsVideoExtended } },
            { { emptyProductId, CommonIdVideoHalf }, { CommonProcessingParamsVideoHalf } },
            { { emptyProductId, CommonIdVideo }, { CommonProcessingParamsVideo } },
            { { emptyProductId, CommonIdFastAcquisition }, { CommonProcessingParamsFastAcquisition } },
            { { emptyProductId, CommonIdVeryFastAcquisition }, { CommonProcessingParamsVeryFastAcquisition } },
        };
        return std::make_shared<factory::ProcessingParameterMapFactory> (defaultVector, CommonId2Frequencies);
    }

#ifdef ROYALE_ENABLE_PLATFORM_CODE
    bool ds90ub95xDigitalReset (royale::pal::II2cDeviceAccess &i2cDev)
    {
        unsigned tries = 4;
        std::vector<uint8_t> temp (1);
        i2cDev.writeI2cAddress8 (0x01, {0x06});
        while (tries--)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (4));
            i2cDev.readI2cAddress8 (0x01, temp);
            if (!temp[0])
            {
                return true;
            }
            else if (tries == 0)
            {
                return false;
            }
        }
        return false;
    }

    void initDeserializer (royale::pal::II2cDeviceAccess &deser)
    {
        if (!ds90ub95xDigitalReset (deser))
        {
            LOG (WARN) << "Failed to reset deserializer" << std::endl;
        }

        deser.writeI2cAddress8 (0x16, { 0x31 });   //To configure GPIO6 to bring out RX Port 1 Lock indication
        deser.writeI2cAddress8 (0x1f, { 0x02 });   //CSI Speed 800Mbps Serial rate
        deser.writeI2cAddress8 (0x33, { 0x21 });   //Enable CSI output, CSI lane count : 2 Lane
        deser.writeI2cAddress8 (0x20, { 0x10 });   //Enabled FWD for RX Port 1, disabled FWD for RX Port 0
        deser.writeI2cAddress8 (0x4c, { 0x12 });   //Write/Read Enable for RX port 1
        deser.writeI2cAddress8 (0x58, { 0x5a });   //Enable I2C pass through to serializer
        deser.writeI2cAddress8 (0x5c, { 0x18 });   //Serializer alias ID
        deser.writeI2cAddress8 (0x5d, { 0x7a });   //Sensor Slave ID
        deser.writeI2cAddress8 (0x65, { 0x7a });   //Slave alias ID
        deser.writeI2cAddress8 (0x5e, { 0x92 });   //TMP Sensor Slave ID
        deser.writeI2cAddress8 (0x66, { 0x92 });   //TMP Sensor Slave alias ID
        deser.writeI2cAddress8 (0x5f, { 0xA0 });   //TMP Sensor Slave ID
        deser.writeI2cAddress8 (0x67, { 0xA0 });   //TMP Sensor Slave alias ID
        deser.writeI2cAddress8 (0x60, { 0xA2 });   //TMP Sensor Slave ID
        deser.writeI2cAddress8 (0x68, { 0xA2 });   //TMP Sensor Slave alias ID
        deser.writeI2cAddress8 (0x61, { 0xA4 });   //TMP Sensor Slave ID
        deser.writeI2cAddress8 (0x69, { 0xA4 });   //TMP Sensor Slave alias ID
        deser.writeI2cAddress8 (0x7c, { 0x41 });   //FV active low
        deser.writeI2cAddress8 (0x6d, { 0x78 });   //CSI Mode
        deser.writeI2cAddress8 (0x02, { 0x3E });   //CSI Mode
        deser.writeI2cAddress8 (0x0A, { 0x13 });   //CSI Mode
        deser.writeI2cAddress8 (0x0B, { 0x25 });   //CSI Mode
    }

    void initSerializer (royale::pal::II2cDeviceAccess &serializer)
    {
        if (!ds90ub95xDigitalReset (serializer))
        {
            LOG (WARN) << "Failed to reset serializer" << std::endl;
        }

        serializer.writeI2cAddress8 (0x0E, {0x10});
        serializer.writeI2cAddress8 (0x0D, {0x00});
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        serializer.writeI2cAddress8 (0x0D, {0x01});
        std::this_thread::sleep_for (std::chrono::milliseconds (500));
        serializer.writeI2cAddress8 (0x02, {0x53});
        serializer.writeI2cAddress8 (0x03, {0x13});
        serializer.writeI2cAddress8 (0x05, {0x03});
        serializer.writeI2cAddress8 (0x0B, {0x13});
        serializer.writeI2cAddress8 (0x0C, {0x26});
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
    }
#endif
}

CameraFactory::CameraFactory()
{

}

std::unique_ptr<ICameraDevice> CameraFactory::createDevice (
    std::shared_ptr<royale::config::ModuleConfig> config,
    std::shared_ptr<royale::hal::IBridgeImager> bridgeImager,
    std::shared_ptr<royale::hal::IBridgeDataReceiver> bridgeDataReceiver,
    std::shared_ptr<royale::pal::II2cBusAccess> i2cAccess)
{
#ifdef ROYALE_ENABLE_PLATFORM_CODE
    std::shared_ptr<const royale::config::ICoreConfig> coreConfig = royale::factory::CoreConfigFactory (*config, getProcessingParameterFactory()) ();
    auto imagerConfig = std::make_shared<royale::config::ImagerConfig> (config->imagerConfig);

    // create an IImager using the imager factory
    std::shared_ptr<royale::hal::IImager> imager = ImagerFactory::createImager (bridgeImager, coreConfig, imagerConfig,
            config->illuminationConfig, false /* direct imager access */);

    // create the correct temperature sensor
    std::shared_ptr<royale::hal::ITemperatureSensor> tempSensor = nullptr;

    try
    {
        auto route = findSensorRoute (config->essentialSensors, SensorRole::TEMP_ILLUMINATION);

        // All sensors supported by the following need an I2C routing and I2C access
        auto i2cRoute = std::dynamic_pointer_cast<const SensorRoutingConfigI2c> (route);
        if (i2cRoute == nullptr)
        {
            //Can't create temperature sensor, no I2C routing found
            return nullptr;
        }

        auto device = std::make_shared<royale::pal::Access2I2cDeviceAdapter> (i2cAccess, *i2cRoute);

        tempSensor.reset (new TemperatureSensorTMP102 (device));
    }
    catch (...)
    {
        return nullptr;
    }

    // create the correct storage
    std::shared_ptr<royale::hal::INonVolatileStorage> flash = nullptr;
    try
    {
        auto route = findSensorRoute (config->essentialSensors, SensorRole::STORAGE_CALIBRATION);

        // All sensors supported by the following need an I2C routing and I2C access
        auto i2cRoute = std::dynamic_pointer_cast<const SensorRoutingConfigI2c> (route);
        if (i2cRoute == nullptr)
        {
            //Can't create temperature sensor, no I2C routing found
            return nullptr;
        }

        auto storage = std::make_shared<StorageI2cEeprom> (config->flashMemoryConfig, i2cAccess, i2cRoute->getAddress());
        flash.reset (new StorageFormatPolar (std::move (storage), config->flashMemoryConfig.identifierIsMandatory));
    }
    catch (...)
    {
        return nullptr;
    }

    // take the default flow control
    std::shared_ptr<royale::common::IFlowControlStrategy> flowControl = nullptr;
    uint16_t rawFrameRate = 0u;
    flowControl.reset (new FlowControlStrategyFixed (rawFrameRate));

    // Configure Deserializer
    auto deserializer = std::make_shared<royale::pal::Access2I2cDeviceAdapter> (i2cAccess, 0x30);
    initDeserializer (*deserializer);

    // Configure Serializer
    auto serializer = std::make_shared<royale::pal::Access2I2cDeviceAdapter> (i2cAccess, 0x0C);
    initSerializer (*serializer);

    std::unique_ptr<CameraCore> cameraModule = nullptr;

    try
    {
        cameraModule = royale::common::makeUnique<CameraCore> (
                           coreConfig,
                           imager,
                           bridgeDataReceiver,
                           tempSensor,
                           flash,
                           flowControl,
                           nullptr,
                           CameraAccessLevel::L2
                       );
    }
    catch (...)
    {
        return nullptr;
    }

    if (cameraModule == nullptr)
    {
        return nullptr;
    }

    // instantiate the correct processing engine
    std::shared_ptr<IProcessing> processing = nullptr;
    processing.reset (new ProcessingSpectre (cameraModule->getCaptureReleaser()));

    std::unique_ptr<CameraDevice> cameraDevice
    {
        new CameraDevice (
            CameraAccessLevel::L2,
            "A65",
            std::move (cameraModule),
            coreConfig,
            processing,
            royale::CallbackData::Depth)
    };

    cameraDevice->setExternalTrigger (true);

    // setup recording engine
    std::unique_ptr<royale::IRecord> recording
    {
        new royale::record::CameraRecord (cameraDevice.get(), processing.get(), nullptr, "A65", imagerConfig->imagerType)
    };

    cameraDevice->setRecordingEngine (std::move (recording));

    return std::move (cameraDevice);
#else
    return nullptr;
#endif
}

#ifdef ROYALE_FACTORY_PLAYBACK
std::unique_ptr<ICameraDevice> CameraFactory::createPlaybackDevice (const char *fileName)
{
    std::unique_ptr<ICameraDevice> cameraDevice = nullptr;

    cameraDevice.reset (new record::CameraPlayback (CameraAccessLevel::L1, fileName));

    return cameraDevice;
}
#endif

std::unique_ptr<ICameraDevice> CameraFactory::createCamera()
{
#ifdef ROYALE_ENABLE_PLATFORM_CODE
    // define your configuration
    auto config = getModuleConfigCustom();

    // create an IBridgeDataReceiver implementation in order to receive the data
    std::shared_ptr<royale::hal::IBridgeDataReceiver> bridgeReceiver =
        std::make_shared<v4l::bridge::BridgeV4l> ("/dev/video0", PIX_FMT_SBGGR12);
    static_cast<v4l::bridge::BridgeV4l *> (bridgeReceiver.get())->setTransferFormat (BufferDataFormat::S32V234);

    // create an instance of a I2CAccess
    std::shared_ptr<royale::pal::II2cBusAccess> i2cAccess = std::make_shared<I2cAccessImpl> ("/dev/i2c-0");

    auto imagerAdapter = std::make_shared<royale::pal::Access2I2cDeviceAdapter> (i2cAccess, 0x3d);
    // get an implementation of the IBridgeImager interface in order to talk to the imager
    std::shared_ptr<royale::hal::IBridgeImager> bridgeImager = std::make_shared<BridgeImagerImpl> (imagerAdapter);

    return createDevice (config, bridgeImager, bridgeReceiver, i2cAccess);
#else
    std::unique_ptr<ICameraDevice> cameraDevice = nullptr;

    CameraManager manager (ROYALE_ACCESS_CODE_LEVEL2);
    auto cams = manager.getConnectedCameraList();
    if (cams.empty())
    {
        return nullptr;
    }

    cameraDevice = manager.createCamera (cams[0]);

    return cameraDevice;
#endif
}

