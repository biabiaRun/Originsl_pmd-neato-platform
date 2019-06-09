/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/MakeUnique.hpp>

#include <factory/CameraCoreBuilderMira.hpp>
#include <factory/FlowControlStrategyFactory.hpp>
#include <factory/ImagerFactory.hpp>
#include <factory/NonVolatileStorageFactory.hpp>
#include <factory/PsdTemperatureSensorFactory.hpp>
#include <factory/TemperatureSensorFactory.hpp>

// This is only needed for L4 access
#include <factory/SoftwareDefinedImagerInterfaceAdapter.hpp>

#include <imager/ImagerEmpty.hpp>

using namespace royale::factory;
using namespace royale::common;

CameraCoreBuilderMira::CameraCoreBuilderMira()
{
}

std::unique_ptr<royale::device::CameraCore> CameraCoreBuilderMira::createCameraCore()
{
    if (m_bridgeFactory == nullptr)
    {
        throw LogicError ("Can't create CameraCore, no bridgeFactory!");
    }
    if (m_config == nullptr)
    {
        throw LogicError ("Can't create CameraCore, no CoreConfig!");
    }
    if (m_imagerConfig == nullptr)
    {
        throw LogicError ("Can't create CameraCore, no ImagerConfig!");
    }

    auto bridgeImager = m_bridgeFactory->create<royale::hal::IBridgeImager>();
    auto bridgeDataReceiver = m_bridgeFactory->create<royale::hal::IBridgeDataReceiver>();

    auto tempSensor = TemperatureSensorFactory::createTemperatureSensor (
                          *m_bridgeFactory,
                          m_temperatureSensorConfig,
                          findSensorRoute (m_essentialSensors, SensorRole::TEMP_ILLUMINATION).get()
                      );

    auto flash = NonVolatileStorageFactory::createFlash (
                     *m_bridgeFactory,
                     m_flashMemoryConfig,
                     findSensorRoute (m_essentialSensors, SensorRole::STORAGE_CALIBRATION).get()
                 );
    auto flowControl = FlowControlStrategyFactory::createFlowControl (bridgeDataReceiver, m_config->getBandwidthRequirementCategory());

    std::shared_ptr<royale::hal::IImager> imager;
    if (m_accessLevel != CameraAccessLevel::L4)
    {
        // create imager based on the given bridgeImager object
        imager = royale::factory::ImagerFactory::createImager (
                     bridgeImager,
                     m_config,
                     m_imagerConfig,
                     m_illuminationConfig,
                     m_accessLevel == CameraAccessLevel::L3);
    }
    else
    {
        // For L4, we need to override the imager with ImagerEmpty,
        // which has to be constructed with the PseudoDataInterpreter from the "real" imager.
        auto pdi = royale::factory::ImagerFactory::createPseudoDataInterpreter (m_imagerConfig->imagerType);
        imager = SoftwareDefinedImagerInterfaceAdapter::createImager (std::make_shared<royale::imager::ImagerEmpty> (bridgeImager, std::move (pdi)));
    }

    auto psdtempSensor = PsdTemperatureSensorFactory::createTemperatureSensor (*imager, m_temperatureSensorConfig);

    auto cameraCore = makeUnique< royale::device::CameraCore > (
                          m_config,
                          imager,
                          bridgeDataReceiver,
                          tempSensor,
                          flash,
                          flowControl,
                          std::move (psdtempSensor),
                          m_accessLevel);

    auto imagerName = royale::factory::ImagerFactory::getImagerTypeName (m_imagerConfig->imagerType);

    cameraCore->addToCoreInfo (royale::Pair<royale::String, royale::String> ("IMAGER", imagerName));

    return cameraCore;
}
