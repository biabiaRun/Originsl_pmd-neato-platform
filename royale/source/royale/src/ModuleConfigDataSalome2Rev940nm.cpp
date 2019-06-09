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

// Salome rev 2(K9 / 60x45 / 945nm / 202 / PO / ECM)
// Module Code : 0102-2600-0060-6300-2000-0302-0400-0000
// Salome rev 2(K9 / 60x45 / 945nm / 202 / PO / ICM)
// Module Code : 0102-2600-1060-6300-2000-0302-0400-0000
// Salome rev 2(Leica / 60x45 / 945nm / 202 / PO / ECM)
// Module Code : 0102-2600-0100-6300-2000-0302-0400-0000
// Salome rev 2(Leica / 60x45 / 945nm / 202 / PO / ICM)
// Module Code : 0102-2600-1100-6300-2000-0302-0400-0000

const ModuleConfig royale::config::moduleconfig::Salome2Rev940nm
{
    royale::config::CoreConfig
    {
        { 112, 86 },
        { 224, 172 },
        royale::usecase::UseCaseList{
            royale::usecase::UseCaseList{
                royale::usecase::UseCase{
                    "MODE_9_5FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseEightPhase> ("{07497ae4-8360-4709-8dc9-7fc77bb07bb4}",
                    5u, royale::Pair<uint32_t, uint32_t> { 1u, 1500u }, 1500u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_9_1FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseEightPhase> ("{a3c3a3e0-794d-47e5-9f35-669909369976}",
                    1u, royale::Pair<uint32_t, uint32_t> { 1u, 2500u }, 2500u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_9_2FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseEightPhase> ("{44df9ce3-69f1-4f41-8dd5-68ebe785b47b}",
                    2u, royale::Pair<uint32_t, uint32_t> { 1u, 2200u }, 2200u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_9_3FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseEightPhase> ("{96109b62-3608-4e4a-b278-32946ccca144}",
                    3u, royale::Pair<uint32_t, uint32_t> { 1u, 2000u }, 2000u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_9_10FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseEightPhase> ("{7e167d85-7b69-4cb9-afbf-06c540f7e328}",
                    10u, royale::Pair<uint32_t, uint32_t> { 1u, 850u }, 850u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_9_15FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseEightPhase> ("{d30f4ee7-7fdf-4db2-8dcf-46bf9ffe7688}",
                    1u, royale::Pair<uint32_t, uint32_t> { 1u, 550u }, 550u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_9_25FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseEightPhase> ("{38679324-497b-4129-af3a-2dc60afc5635}",
                    1u, royale::Pair<uint32_t, uint32_t> { 1u, 350u }, 350u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_5_5FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseFourPhase> ("{c200bc13-a39a-4e58-8960-217ef7d2801e}",
                    5u, royale::Pair<uint32_t, uint32_t> { 1u, 3000u }, 3000u),
                    { royale::moduleconfig::CommonProcessingParams1Frequency },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_5_10FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseFourPhase> ("{e324e101-2e54-4fbd-b4a4-944598daa4cc}",
                    10u, royale::Pair<uint32_t, uint32_t> { 1u, 1700u }, 1700u),
                    { royale::moduleconfig::CommonProcessingParams1Frequency },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_5_15FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseFourPhase> ("{66687b3d-6116-4bcd-a68f-e5401ff758c5}",
                    15u, royale::Pair<uint32_t, uint32_t> { 1u, 1100u }, 1100u),
                    { royale::moduleconfig::CommonProcessingParams1Frequency },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_5_25FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseFourPhase> ("{517c5fbb-afbf-4d00-9781-dbd50c12b2d3}",
                    25u, royale::Pair<uint32_t, uint32_t> { 1u, 700u }, 700u),
                    { royale::moduleconfig::CommonProcessingParams1Frequency },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_5_30FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseFourPhase> ("{49a53b34-9264-4c1f-88f2-a5f34946fff6}",
                    30u, royale::Pair<uint32_t, uint32_t> { 1u, 600u }, 600u),
                    { royale::moduleconfig::CommonProcessingParams1Frequency },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_5_35FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseFourPhase> ("{70601868-ff44-4225-a3f2-7c7ed0e94ca0}",
                    35u, royale::Pair<uint32_t, uint32_t> { 1u, 500u }, 500u),
                    { royale::moduleconfig::CommonProcessingParams1Frequency },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_5_45FPS",
                    std::make_shared<salome::NameIsIdentifierUseCaseFourPhase> ("{4970b003-c22c-4277-aad2-1c9b158f257b}",
                    45u, royale::Pair<uint32_t, uint32_t> { 1u, 400u }, 400u),
                    { royale::moduleconfig::CommonProcessingParams1Frequency },
                    royale::CallbackData::Depth
                },
                royale::usecase::UseCase{
                    "MODE_CALIBRATION",
                    std::make_shared<salome::NameIsIdentifierUseCaseCalibration> ("{a967b508-07fd-4791-8536-90162e372769}",
                    5u, royale::Pair<uint32_t, uint32_t> { 1u, 1200u }, 1200u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Raw,
                    royale::CameraAccessLevel::L3
                },
            },
        },
        royale::config::BandwidthRequirementCategory::USB3_THROTTLING,
        royale::config::FrameTransmissionMode::SUPERFRAME,
        "Salome Rev2 940nm",
        65.0f,
        60.0f,
        true
    },
    salome::rev2ImagerConfig,
    salome::illuConfig,
    salome::tempsensorConfig,
    salome::flashConfig,
    salome::sensorMap
};
