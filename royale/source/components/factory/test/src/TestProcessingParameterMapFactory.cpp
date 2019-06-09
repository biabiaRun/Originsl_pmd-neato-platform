/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <gtest/gtest.h>
#include <common/exceptions/LogicError.hpp>
#include <factory/ProcessingParameterMapFactory.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::factory;
using namespace royale::processing;

TEST (TestProcessingParameterMapFactory, ProductSpecificConfig)
{
    const auto product = std::vector<uint8_t> {'p', 'r', 'o', 'd', 'u', 'c', 't'};
    const auto generic = std::vector<uint8_t> {};
    const auto idOne = ProcessingParameterId {"use case one"};
    const auto idTwo = ProcessingParameterId {"use case two"};
    const auto idMixed = ProcessingParameterId {"use case mixed"};

    /**
     * An override of paramsUCOne for a specific product.
     */
    const royale::Vector<royale::ProcessingParameterMap> paramsProductUCOne {{{ ProcessingFlag::LowerSaturationThreshold_Int, 1 }}};
    const royale::Vector<royale::ProcessingParameterMap> paramsUCOne {{{ ProcessingFlag::LowerSaturationThreshold_Int, 2 }}};
    const royale::Vector<royale::ProcessingParameterMap> paramsUCTwo {{{ ProcessingFlag::LowerSaturationThreshold_Int, 3 }}};
    const royale::Vector<royale::ProcessingParameterMap> paramsMixed {{{ ProcessingFlag::LowerSaturationThreshold_Int, 4 }, {ProcessingFlag::LowerSaturationThreshold_Int, 5}}};

    ProcessingParameterMapFactory::key_type tmpKey {product, idOne};

    royale::Vector<ProcessingParameterMapFactory::value_type> maps
    {
        {{product, idOne}, paramsProductUCOne},
        {{generic, idOne}, paramsUCOne},
        {{generic, idTwo}, paramsUCTwo},
        {{generic, idMixed}, paramsMixed}
    };

    std::shared_ptr<ProcessingParameterMapFactory> paramFactory;

    ASSERT_NO_THROW (paramFactory = std::make_shared<ProcessingParameterMapFactory> (maps, idTwo));
    ASSERT_EQ (paramFactory->getParameterMaps (product, idOne), paramsProductUCOne);
    ASSERT_EQ (paramFactory->getParameterMaps ({}, idOne), paramsUCOne);
    ASSERT_EQ (paramFactory->getParameterMaps (product, idTwo), paramsUCTwo);
    ASSERT_EQ (paramFactory->getParameterMaps ({}, idTwo), paramsUCTwo);
    ASSERT_EQ (paramFactory->getParameterMaps (product, idMixed), paramsMixed);
    ASSERT_EQ (paramFactory->getParameterMaps ({}, idMixed), paramsMixed);
}
