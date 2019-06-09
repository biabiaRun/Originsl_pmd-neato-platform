/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <modules/ModuleConfigPicoMonstarCommon.hpp>
#include <usecase/UseCaseCalibration.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::config;
using namespace royale::imager::M2450_A12;

const ModuleConfig royale::config::moduleconfig::PicoMonstarDefault
{
    royale::config::CoreConfig{
        { 176, 143 },  // lens center should be constant
        { 352, 287 },  // resolution
        royale::usecase::UseCaseList{
            royale::usecase::UseCase{
                "MODE_10_5FPS_2000",
                std::make_shared<royale::usecase::UseCaseCalibration> (
                5u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 2000u }, 1000u, 1000u, 1000u, 1000u,
                true, picomonstar::ssc_freq, picomonstar::ssc_kspread, picomonstar::ssc_delta_80320kHz, picomonstar::ssc_freq,
                picomonstar::ssc_kspread, picomonstar::ssc_delta_60240kHz
                ),
                { picomonstar::ProcessingParams2Frequencies },
                royale::CallbackData::Raw,
                royale::CameraAccessLevel::L3
            }
        },
        royale::config::BandwidthRequirementCategory::USB3_THROTTLING,
        royale::config::FrameTransmissionMode::INDIVIDUAL,
        "PICOMONSTARDEFAULT",
        65.0f,
        60.0f,
        true
    },
    picomonstar::imagerConfig,
    picomonstar::illuConfig,
    picomonstar::tempsensorConfig,
    picomonstar::flashConfig,
    picomonstar::sensorMap
};
