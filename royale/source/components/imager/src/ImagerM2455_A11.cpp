/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerM2455_A11.hpp>
#include <imager/M2455/ImagerRegisters.hpp>
#include <imager/M2455/PseudoDataInterpreter.hpp>

#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/WrongState.hpp>

using namespace royale::imager;
using namespace royale::imager::M2455_A11;
using namespace royale::common;

ImagerM2455_A11::ImagerM2455_A11 (const ImagerParameters &params) :
    ImagerM2453 (params)
{
}

std::unique_ptr<IPseudoDataInterpreter> ImagerM2455_A11::createPseudoDataInterpreter()
{
    std::unique_ptr<IPseudoDataInterpreter> pseudoDataInter (new M2455::PseudoDataInterpreter (m_usesInternalCurrentMonitor));
    return pseudoDataInter;
}

void ImagerM2455_A11::reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex)
{
    if (ImagerState::Capturing != m_imagerState)
    {
        throw WrongState();
    }

    // This isn't the same as the M2453 B11's MB0_FRAMETIME implementation.
    // For M2455, the register to change seems to be 0xA804, MINFRAMETIME.
    //
    // It looks like it may be similar, however, where the M2453 B11 used
    //     auto frametime = m_systemFrequency / (static_cast<uint32_t> (targetFrameRate) * 512);
    // my understanding is that the M2455 will use
    //     auto frametime = m_systemFrequency / (static_cast<uint32_t> (targetFrameRate) * 128);
    // which would give a minimum configurable FPS of around 2.8, compared to the M2453's 0.72

    throw NotImplemented();
}

std::vector < uint16_t > ImagerM2455_A11::getSerialRegisters()
{
    std::vector < uint16_t > efuseValues (4);
    m_bridge->readImagerBurst (M2455_A11::ANAIP_EFUSEVAL1, efuseValues);
    return efuseValues;
}

void ImagerM2455_A11::initialize()
{

    uint16_t regDs = 0;

    m_bridge->readImagerRegister (M2455::ANAIP_DESIGNSTEP, regDs);

    if (0x0A11 != regDs &&
            0x0A12 != regDs &&
            0x0A13 != regDs)
    {
        throw Exception ("wrong design step");
    }

    ImagerM2453::initialize();
}
