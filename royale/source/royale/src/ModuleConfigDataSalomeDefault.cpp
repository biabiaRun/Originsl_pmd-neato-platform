/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <modules/ModuleConfigSalomeCommon.hpp>
#include <modules/CharacterizationHelper.hpp>
#include <modules/CommonProcessingParameters.hpp>

const ModuleConfig royale::config::moduleconfig::SalomeDefault
{
    royale::config::CoreConfig
    {
        { 112, 86 },
        { 224, 172 },
        royale::usecase::UseCaseList{
            royale::usecase::UseCaseList{
                royale::usecase::UseCase{
                    "MODE_CALIBRATION",
                    std::make_shared<salome::NameIsIdentifierUseCaseCalibration> ("{1AC6BD16-EFA3-418D-931F-34EECD0908B8}",
                    5u, royale::Pair<uint32_t, uint32_t> { 1u, 1200u }, 1200u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Raw,
                    royale::CameraAccessLevel::L3
                },
                royale::usecase::UseCase{
                    "MODE_BEAMPROFILE",
                    std::make_shared<salome::NameIsIdentifierUseCaseFourPhase> ("{72478D89-4558-4F22-AD9C-818636A4CB06}",
                    45u, royale::Pair<uint32_t, uint32_t> { 1u, 325u }, 325u),
                    { royale::moduleconfig::CommonProcessingParams1Frequency },
                    royale::CallbackData::Raw,
                    royale::CameraAccessLevel::L3
                },

            },
        },
        royale::config::BandwidthRequirementCategory::USB3_THROTTLING,
        royale::config::FrameTransmissionMode::SUPERFRAME,
        "SalomeDefault",
        65.0f,
        60.0f,
        true
    },
    salome::imagerConfig,
    salome::illuConfig,
    salome::tempsensorConfig,
    salome::flashConfig,
    salome::sensorMap
};
