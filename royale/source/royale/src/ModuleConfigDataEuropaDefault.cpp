/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag & Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <modules/ModuleConfigEuropaCommon.hpp>
#include <modules/CommonProcessingParameters.hpp>

const ModuleConfig royale::config::moduleconfig::EuropaDefault
{
    royale::config::CoreConfig
    {
        { 224, 84 },
        { 448, 168 },
        royale::usecase::UseCaseList{
            royale::usecase::UseCaseList{
                royale::usecase::UseCase{
                    "MODE_CALIBRATION",
                    std::make_shared<royale::europa::NameIsIdentifierUseCaseCalibration> ("{4cdd2641-c71c-4782-a7d4-d33367685903}",
                    5u, royale::Pair<uint32_t, uint32_t> { 1u, 1200u }, 1200u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Raw,
                    royale::CameraAccessLevel::L3
                },
            },
        },
        royale::config::BandwidthRequirementCategory::USB3_THROTTLING,
        royale::config::FrameTransmissionMode::SUPERFRAME,
        "UnknownEuropa",
        10000000000.0f, /* \todo ROYAL-3670 calibrate the temperature sensor*/
        10000000000.0f, /* \todo ROYAL-3670 for now just allow any temperature */
        true
    },
    royale::europa::imagerConfig,
    royale::europa::illuConfig,
    royale::europa::tempsensorConfig,
    royale::europa::flashConfig,
    royale::europa::sensorMap
};
