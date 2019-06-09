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

// Salome (K9 / 60x45 / 945nm / 202 / PO)
// 0x01, 0x02, 0x23, 0x00, 0x00, 0x60, 0x62, 0x00, 0x20, 0x00, 0x03, 0x01, 0x04, 0x00, 0x00, 0x00
// Module ID hash : 0x1376DA3C

const ModuleConfig royale::config::moduleconfig::Salome940nm
{
    royale::config::CoreConfig
    {
        { 112, 86 },
        { 224, 172 },
        royale::usecase::UseCaseList{
            royale::usecase::UseCaseList{
                royale::usecase::UseCase{
                    "MODE_9_5FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseEightPhase> ("{DD2CA51D-E620-4C5C-9E0A-228ED6A6DCB6}",
                    5u, royale::Pair<uint32_t, uint32_t> { 1u, 1300u }, 1300u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_9_1FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseEightPhase> ("{4D311C79-9137-4430-ACE4-CB2C172CA8A3}",
                    1u, royale::Pair<uint32_t, uint32_t> { 1u, 1800u }, 1800u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_9_2FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseEightPhase> ("{BE08A2BB-7638-41AA-9823-CD16B029590D}",
                    2u, royale::Pair<uint32_t, uint32_t> { 1u, 1800u }, 1800u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_9_3FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseEightPhase> ("{D1C6B0B3-DA46-41E4-A714-027569772FCA}",
                    3u, royale::Pair<uint32_t, uint32_t> { 1u, 1700u }, 1700u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_9_10FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseEightPhase> ("{900066BB-2A15-42ED-A8AF-0C24FD2F403C}",
                    10u, royale::Pair<uint32_t, uint32_t> { 1u, 750u }, 750u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_9_15FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseEightPhase> ("{F49B7C0E-1D61-426D-B977-8E2F13DFB2B2}",
                    1u, royale::Pair<uint32_t, uint32_t> { 1u, 500u }, 500u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_9_25FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseEightPhase> ("{BA8DB44D-4BCE-4BC9-BDF7-EE3294DEFBCB}",
                    1u, royale::Pair<uint32_t, uint32_t> { 1u, 300u }, 300u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_5_5FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseFourPhase> ("{66017C4C-415A-447B-B85B-E905DF43DD84}",
                    5u, royale::Pair<uint32_t, uint32_t> { 1u, 2400u }, 2400u),
                    { royale::moduleconfig::CommonProcessingParams1Frequency },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_5_10FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseFourPhase> ("{ABB9489A-EB27-4042-8399-1E905CD88D29}",
                    10u, royale::Pair<uint32_t, uint32_t> { 1u, 1400u }, 1400u),
                    { royale::moduleconfig::CommonProcessingParams1Frequency },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_5_15FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseFourPhase> ("{7871CA22-7CF1-4ED4-B014-40B59E1754CF}",
                    15u, royale::Pair<uint32_t, uint32_t> { 1u, 1000u }, 1000u),
                    { royale::moduleconfig::CommonProcessingParams1Frequency },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_5_25FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseFourPhase> ("{954FC8CE-514D-4A4F-9325-BE4F48BDD5C7}",
                    25u, royale::Pair<uint32_t, uint32_t> { 1u, 600u }, 600u),
                    { royale::moduleconfig::CommonProcessingParams1Frequency },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_5_30FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseFourPhase> ("{45FA880E-6E01-4339-8E49-85CD616E33BE}",
                    30u, royale::Pair<uint32_t, uint32_t> { 1u, 500u }, 500u),
                    { royale::moduleconfig::CommonProcessingParams1Frequency },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_5_35FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseFourPhase> ("{4E665A5C-D7FD-46FA-ACD0-D17463060BD2}",
                    35u, royale::Pair<uint32_t, uint32_t> { 1u, 400u }, 400u),
                    { royale::moduleconfig::CommonProcessingParams1Frequency },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_5_45FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseFourPhase> ("{72478D89-4558-4F22-AD9C-818636A4CB06}",
                    45u, royale::Pair<uint32_t, uint32_t> { 1u, 325u }, 325u),
                    { royale::moduleconfig::CommonProcessingParams1Frequency },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_CALIBRATION",
                    std::make_shared<salome::NameIsIdentifierUseCaseCalibration> ("{1AC6BD16-EFA3-418D-931F-34EECD0908B8}",
                    5u, royale::Pair<uint32_t, uint32_t> { 1u, 1200u }, 1200u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Raw,
                    royale::CameraAccessLevel::L3
                },
            },
        },
        royale::config::BandwidthRequirementCategory::USB3_THROTTLING,
        royale::config::FrameTransmissionMode::SUPERFRAME,
        "Salome940nm",
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
