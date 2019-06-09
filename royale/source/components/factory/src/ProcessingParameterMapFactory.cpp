/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <factory/ProcessingParameterMapFactory.hpp>

#include <common/exceptions/LogicError.hpp>

using namespace royale::common;
using namespace royale::factory;

namespace
{
    const auto EMPTY_PRODUCT = std::vector<uint8_t> {};
}

ProcessingParameterMapFactory::ProcessingParameterMapFactory (const royale::Vector<ProcessingParameterMapFactory::value_type> &mapOfMaps,
        const royale::processing::ProcessingParameterId &defaultId) :
    m_maps (royale::Vector<char>::toStdMap (mapOfMaps)),
    m_defaultId (defaultId)
{
    if (m_maps.find (key_type {EMPTY_PRODUCT, defaultId}) == m_maps.end())
    {
        throw LogicError ("ProcessingParameterMapFactory: No mapping for the defaultId");
    }
    if (m_maps.size() != mapOfMaps.size())
    {
        throw LogicError ("ProcessingParameterMapFactory: Duplicate key in the mapOfMaps");
    }
}

royale::Vector<royale::ProcessingParameterMap> ProcessingParameterMapFactory::getParameterMaps (
    const royale::Vector<uint8_t> &productId,
    const royale::processing::ProcessingParameterId &processingId) const
{
    auto params = m_maps.find (key_type {productId.toStdVector(), processingId});
    if (params == m_maps.end())
    {
        params = m_maps.find (key_type {EMPTY_PRODUCT, processingId});
    }
    if (params == m_maps.end())
    {
        // The defaultValue can always be found, otherwise the constructor would have thrown
        params = m_maps.find (key_type {EMPTY_PRODUCT, m_defaultId});
    }
    return params->second;
}
