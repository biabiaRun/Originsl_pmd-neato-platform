/****************************************************************************\
* Copyright (C) 2016 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#include "common/NtcTempAlgo.hpp"
#include <cmath>
#include <iostream>
#include <sstream>
#include "common/exceptions/InvalidValue.hpp"

using namespace royale::common;

const float NtcTempAlgo::celsiusKelvinFactor = 273.15f;

NtcTempAlgo::NtcTempAlgo (float resistorR1, float resistorRntc0, float referenceTemperature, float thermistorBeta) :
    m_resistorRatio (resistorR1 / resistorRntc0),
    m_reciRefTp (1.0f / (referenceTemperature + celsiusKelvinFactor)),
    m_reciTBeta (1.0f / thermistorBeta)
{

}

float NtcTempAlgo::calcTemperature (uint16_t vRef1, uint16_t vNtc1, uint16_t vRef2, uint16_t vNtc2, uint16_t offset) const
{
    auto refVoltage = static_cast<float> (vRef1 - vRef2);
    auto ntcVoltage = static_cast<float> (vNtc1 - vNtc2);
    return calcTemperature (refVoltage, ntcVoltage);
}

float NtcTempAlgo::calcTemperature (float refVoltage, float ntcVoltage) const
{
    checkDifferences (refVoltage, ntcVoltage);
    float tempKelvin = 1.0f / (m_reciTBeta * logf (calcResistorVoltageRatio (refVoltage, ntcVoltage)) + m_reciRefTp);
    float tempCelsius = tempKelvin - celsiusKelvinFactor;
    return tempCelsius;
}

float NtcTempAlgo::calcResistorVoltageRatio (float refVoltage, float ntcVoltage) const
{
    float vDiffRatio = refVoltage / ntcVoltage - 1.0f;
    float resistorVoltageRatio = m_resistorRatio / vDiffRatio;
    return resistorVoltageRatio;
}

void NtcTempAlgo::checkDifferences (float refVoltage, float ntcVoltage) const
{
    if (ntcVoltage >= refVoltage ||
            ntcVoltage <= 0.0f)
    {
        std::stringstream dt;
        dt << __FILE__ << ":" << __LINE__
           << ": Invalid value in NtcTempAlgo ntcVoltage > refVoltage."
           << " refVoltage = " << refVoltage
           << " ntcVoltage = " << ntcVoltage;
        std::string du = "received invalid temperature values";
        throw InvalidValue (dt.str(), du);
    }
}
