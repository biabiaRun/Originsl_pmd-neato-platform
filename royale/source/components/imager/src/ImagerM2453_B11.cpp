/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerM2453_B11.hpp>
#include <imager/M2453/ImagerRegisters.hpp>
#include <imager/M2453/PseudoDataInterpreter_B11.hpp>

#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/WrongState.hpp>

using namespace royale::imager;
using namespace royale::imager::M2453_B11;
using namespace royale::common;


ImagerM2453_B11::ImagerM2453_B11 (const ImagerParameters &params) :
    ImagerM2453 (params)

{
}

std::unique_ptr<IPseudoDataInterpreter> ImagerM2453_B11::createPseudoDataInterpreter()
{
    std::unique_ptr<IPseudoDataInterpreter> pseudoDataInter (new M2453_B11::PseudoDataInterpreterB11 (m_usesInternalCurrentMonitor));
    return pseudoDataInter;
}

void ImagerM2453_B11::reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex)
{
    if (ImagerState::Capturing != m_imagerState)
    {
        throw WrongState();
    }

    if (m_systemFrequency == 0)
    {
        throw LogicError ("Imager does not support frame rate changes!");
    }

    // Prevent exposure changes if there is still a change pending.
    // (we might also block here)
    if (configChangePending())
    {
        throw RuntimeError ("Can't update exposure times while config change is still pending");
    }

    auto frametime = m_systemFrequency / (static_cast<uint32_t> (targetFrameRate) * 512);

    if (frametime > std::numeric_limits<uint16_t>::max())
    {
        throw RuntimeError ("Frame rate too low!");
    }

    m_bridge->writeImagerRegister (M2453_B11::MB0_FRAMETIME, static_cast<uint16_t> (frametime));

    reconfigIndex = triggerUseCaseChange();
}

std::vector < uint16_t > ImagerM2453_B11::getSerialRegisters()
{
    std::vector < uint16_t > efuseValues (4);
    m_bridge->readImagerBurst (M2453_B11::ANAIP_EFUSEVAL1, efuseValues);
    return efuseValues;
}

void ImagerM2453_B11::initialize()
{

    uint16_t regDs = 0;

    m_bridge->readImagerRegister (M2453::ANAIP_DESIGNSTEP, regDs);

    if (0x0B11 != regDs &&
            0x0B12 != regDs)
    {
        throw Exception ("wrong design step");
    }

    ImagerM2453::initialize();
}
