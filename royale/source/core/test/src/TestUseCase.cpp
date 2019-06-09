/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <gtest/gtest.h>
#include <usecase/UseCase.hpp>
#include <usecase/UseCaseFourPhase.hpp>
#include <usecase/UseCaseMixedXHt.hpp>

using namespace royale;
using namespace royale::usecase;

TEST (TestUseCase, IncorrectCtor)
{
    royale::Vector<royale::ProcessingParameterMap> zeroMaps;
    royale::Vector<royale::ProcessingParameterMap> oneMap { {} };
    royale::Vector<royale::ProcessingParameterMap> twoMaps { {}, {} };

    // all empty
    {
        EXPECT_ANY_THROW (UseCase uc ("", nullptr, zeroMaps));
    }

    // empty name
    {
        std::shared_ptr<UseCaseDefinition> definition {new UseCaseFourPhase (5, 30000000, { 50u, 1000u }, 1000u, 0) };
        EXPECT_ANY_THROW (UseCase uc ("", std::move (definition), oneMap));
    }

    // empty definition
    {
        EXPECT_ANY_THROW (UseCase uc ("UseCase1", nullptr, oneMap));
    }

    // empty map
    {
        std::shared_ptr<UseCaseDefinition> definition {new UseCaseFourPhase (5, 30000000, { 50u, 1000u }, 1000u, 0) };
        EXPECT_ANY_THROW (UseCase uc ("UseCase1", std::move (definition), zeroMaps));
    }

    // one stream, two sets of parameters
    {
        std::shared_ptr<UseCaseDefinition> definition {new UseCaseFourPhase (5, 30000000, { 50u, 1000u }, 1000u, 0) };
        EXPECT_ANY_THROW (UseCase uc ("UseCase1", std::move (definition), twoMaps));
    }

    // two streams, one set of parameters
    {
        std::shared_ptr<UseCaseDefinition> definition {new UseCaseMixedXHt (5, 1, 30000000, 30000000, 20200000,
            { 50u, 1000u }, { 50u, 1000u }, 1000u, 1000u, 1000u, 1000u, 1000u) };
        EXPECT_ANY_THROW (UseCase uc ("UseCase1", std::move (definition), oneMap));
    }
}

TEST (TestUseCase, UseCaseList)
{
    royale::Vector<royale::ProcessingParameterMap> tmpMap { {} };
    UseCaseList list;
    std::shared_ptr<UseCaseDefinition> uc1 { new UseCaseFourPhase (5, 30000000, { 50u, 1000u }, 1000u, 0) };
    std::shared_ptr<UseCaseDefinition> uc2 { new UseCaseFourPhase (35, 80000000, { 50u, 1000u }, 1000u, 0) };
    list.push_back (UseCase ("first", uc1, tmpMap));
    list.push_back (UseCase ("second", uc2, tmpMap, CallbackData::Raw, CameraAccessLevel::L3));

    EXPECT_STREQ (list[0].getName().c_str(), "first");
    EXPECT_STREQ (list[1].getName().c_str(), "second");

    EXPECT_EQ (list[0].getDefinition()->getTargetRate(), 5u);
    EXPECT_EQ (list[1].getDefinition()->getTargetRate(), 35u);

    EXPECT_EQ (CameraAccessLevel::L1, list[0].getAccessLevel ());
    EXPECT_EQ (CameraAccessLevel::L3, list[1].getAccessLevel ());

    EXPECT_EQ (CallbackData::Depth, list[0].getCallbackData());
    EXPECT_EQ (CallbackData::Raw, list[1].getCallbackData());
}
