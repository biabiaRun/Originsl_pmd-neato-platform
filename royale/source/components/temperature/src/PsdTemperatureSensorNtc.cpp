/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <temperature/PsdTemperatureSensorNtc.hpp>

#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/RuntimeError.hpp>

using namespace royale::sensors;
using namespace royale::common;

PsdTemperatureSensorNtc::PsdTemperatureSensorNtc (std::unique_ptr<const IPseudoDataInterpreter> psdInterpreter,
        std::unique_ptr <NtcTempAlgo> ntcAlgorithm,
        PseudoDataPhaseSync phaseSyncConfig) :
    m_psdInterpreter (std::move (psdInterpreter)),
    m_ntcAlgorithm (std::move (ntcAlgorithm)),
    m_phaseSyncConfig (phaseSyncConfig)
{
}

PsdTemperatureSensorNtc::~PsdTemperatureSensorNtc()
{
}

float PsdTemperatureSensorNtc::calcTemperature (const std::vector<ICapturedRawFrame *> &frames)
{
    const ICapturedRawFrame *frameToUse;

    switch (m_phaseSyncConfig)
    {
        case IPsdTemperatureSensor::PseudoDataPhaseSync::FIRST:
            {
                frameToUse = frames.front();
                break;
            }
        case IPsdTemperatureSensor::PseudoDataPhaseSync::SECOND:
            {
                if (frames.size() > 1)
                {
                    frameToUse = frames[1];
                }
                else
                {
                    throw LogicError ("There is no second phase to collect NTC data");
                }
                break;
            }
        case IPsdTemperatureSensor::PseudoDataPhaseSync::LAST:
            {
                frameToUse = frames.back();
                break;
            }
        case IPsdTemperatureSensor::PseudoDataPhaseSync::NOT_SYNCHRONIZED:
            {
                // For Pseudodata sensors we need this information.
                throw RuntimeError ("phaseSyncConfig set to NOT_SYNCHRONIZED");
            }
        default:
            {
                // There is no valid phaseConfig. This shouldn't happen.
                throw LogicError ("Invalid phaseSyncConfig for the NTC readout");
            }
    }

    uint16_t offset, vRef1, vRef2, vNtc1, vNtc2;
    m_psdInterpreter->getTemperatureRawValues (*frameToUse, vRef1, vNtc1, vRef2, vNtc2, offset);
    return m_ntcAlgorithm->calcTemperature (vRef1, vNtc1, vRef2, vNtc2, offset);
}
