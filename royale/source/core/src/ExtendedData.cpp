/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <processing/ExtendedData.hpp>

using namespace royale;
using namespace royale::processing;

ExtendedData::ExtendedData() :
    m_depthData (nullptr),
    m_rawData (nullptr),
    m_intermediateData (nullptr)
{
}

ExtendedData::~ExtendedData()
{
}

void ExtendedData::setDepthData (const DepthData *data)
{
    m_depthData = data;
}

void ExtendedData::setRawData (const RawData *data)
{
    m_rawData = data;
}

void ExtendedData::setIntermediateData (const IntermediateData *data)
{
    m_intermediateData = data;
}

bool ExtendedData::hasDepthData() const
{
    return m_depthData != nullptr;
}

bool ExtendedData::hasRawData() const
{
    return m_rawData != nullptr;
}

bool ExtendedData::hasIntermediateData() const
{
    return m_intermediateData != nullptr;
}

const RawData *ExtendedData::getRawData() const
{
    return m_rawData;
}

const DepthData *ExtendedData::getDepthData() const
{
    return m_depthData;
}

const IntermediateData *ExtendedData::getIntermediateData() const
{
    return m_intermediateData;
}