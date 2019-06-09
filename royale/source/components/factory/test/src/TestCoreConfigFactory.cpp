/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <gtest/gtest.h>
#include <common/exceptions/LogicError.hpp>
#include <config/CoreConfig.hpp>
#include <factory/CoreConfigFactory.hpp>
#include <factory/ProcessingParameterMapFactory.hpp>
#include <usecase/UseCaseFourPhase.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::config;
using namespace royale::factory;
using namespace royale::usecase;

TEST (TestCoreConfigFactory, InvalidUseCaseList)
{
    royale::Vector<royale::ProcessingParameterMap> tmpMap { {} };
    ProcessingParameterMapFactory::key_type tmpKey {{}, royale::processing::ProcessingParameterId {"temp"}};
    royale::Vector<ProcessingParameterMapFactory::value_type> tmpMaps {{{tmpKey, tmpMap}}};
    auto paramFactory = std::make_shared<ProcessingParameterMapFactory> (tmpMaps, royale::processing::ProcessingParameterId {"temp"});

    UseCaseList list;
    std::shared_ptr<UseCaseDefinition> uc1 { new UseCaseFourPhase (5, 30000000, { 50u, 1000u }, 1000u, 0) };
    std::shared_ptr<UseCaseDefinition> uc2 { new UseCaseFourPhase (35, 80000000, { 50u, 1000u }, 1000u, 0) };
    list.push_back (UseCase ("first", uc1, tmpMap));
    list.push_back (UseCase ("duplicate", uc2, tmpMap, CallbackData::Raw, CameraAccessLevel::L3));
    list.push_back (UseCase ("duplicate", uc2, tmpMap, CallbackData::Raw));

    CoreConfig coreConfig
    {
        {48, 48},
        {96, 96},
        list,
        BandwidthRequirementCategory::NO_THROTTLING,
        FrameTransmissionMode::SUPERFRAME,
        "Test with duplicates",
        65.f,
        60.f,
        false
    };

    CoreConfigFactory factory (coreConfig, paramFactory);

    EXPECT_THROW (factory(), LogicError);
}
