/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerRegisterAccess.hpp>

#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/Timeout.hpp>
#include <common/exceptions/WrongState.hpp>
#include <common/NarrowCast.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <thread>

using namespace std;
using namespace royale::imager;
using namespace royale::common;

namespace
{
    /**
    * The number of times that pollUntil sleeps for pollingInterval, before it throws a
    * Timeout.
    */
    const auto MAX_POLL_RETRIES = 4;
}

ImagerRegisterAccess::ImagerRegisterAccess (const std::shared_ptr<royale::hal::IBridgeImager> &bridge) :
    m_bridge{ bridge }
{
    if (m_bridge == nullptr)
    {
        throw LogicError ("nullref exception");
    }
}

void ImagerRegisterAccess::transferRegisterMapAuto (const TimedRegisterList &registerMap)
{
    if (registerMap.empty())
    {
        return;
    }

    auto firstAddress = registerMap.begin()->address;
    auto sleepTime = registerMap.begin()->sleepTime;
    // nextAddress is a uint32_t because it shouldn't wrap round - a write to 0xffff followed by
    // a write to 0x0000 should not be treated as a pair of consecutive addresses
    uint32_t nextAddress = firstAddress;
    std::vector<uint16_t> regBatch{};

    for (const auto &mapEntry : registerMap)
    {
        //check for consecutive order
        if (nextAddress != mapEntry.address || sleepTime)
        {
            if (regBatch.size())
            {
                //a consecutive batch is ready to be written
                m_bridge->writeImagerBurst (firstAddress, regBatch);

                if (sleepTime)
                {
                    m_bridge->sleepFor (std::chrono::microseconds (sleepTime));
                }

                //clear batch and proceed with next one
                regBatch.clear();
                firstAddress = mapEntry.address;
            }

            nextAddress = mapEntry.address + 1;
        }
        else
        {
            nextAddress++;
        }

        //it was a consecutive address entry, stage for burst write
        regBatch.push_back (mapEntry.value);
        sleepTime = mapEntry.sleepTime;
    }

    if (regBatch.size())
    {
        m_bridge->writeImagerBurst (firstAddress, regBatch);

        if (sleepTime)
        {
            m_bridge->sleepFor (std::chrono::microseconds (sleepTime));
        }
    }
}

void ImagerRegisterAccess::pollUntil (uint16_t reg,
                                      const uint16_t expectedVal,
                                      const std::chrono::microseconds firstSleep,
                                      const std::chrono::microseconds pollSleep)
{
    m_bridge->sleepFor (firstSleep);
    bool done = false;

    uint16_t polledVal;
    m_bridge->readImagerRegister (reg, polledVal);
    if (polledVal == expectedVal)
    {
        done = true;
    }

    int retries = 0;
    while (!done && retries < MAX_POLL_RETRIES)
    {
        LOG (DEBUG) << "Additional sleep in pollUntil";
        retries++;
        m_bridge->sleepFor (pollSleep);
        m_bridge->readImagerRegister (reg, polledVal);
        if (polledVal == expectedVal)
        {
            done = true;
        }
    }

    if (!done)
    {
        throw Timeout ("Expected value not read even after polling");
    }
}

void ImagerRegisterAccess::writeRegisters (const std::vector<uint16_t> &registerAddresses,
        const std::vector<uint16_t> &registerValues)
{
    if (registerAddresses.size() != registerValues.size())
    {
        throw LogicError ("vector length mismatch of arguments");
    }

    size_t index = 0;
    uint16_t lastAddress = 0;
    bool continuous = true;
    for (const auto regAdr : registerAddresses)
    {
        if (index)
        {
            if (regAdr - lastAddress != 1)
            {
                continuous = false;
            }
        }

        lastAddress = regAdr;
        index++;
    }

    if (continuous && registerAddresses.size() > 1)
    {
        m_bridge->writeImagerBurst (registerAddresses[0], registerValues);
    }
    else
    {
        for (auto i = 0u; i < registerValues.size(); ++i)
        {
            m_bridge->writeImagerRegister (registerAddresses[i], registerValues[i]);
        }
    }
}

void ImagerRegisterAccess::readRegisters (const std::vector<uint16_t> &registerAddresses,
        std::vector<uint16_t> &registerValues)
{
    if (registerAddresses.size() != registerValues.size())
    {
        throw LogicError ("vector length mismatch of arguments");
    }

    bool continuous = true;
    uint16_t lastAddress = 0;
    size_t index = 0;
    for (const auto regAdr : registerAddresses)
    {
        if (index)
        {
            if (regAdr - lastAddress != 1)
            {
                continuous = false;
            }
        }

        lastAddress = regAdr;
        index++;
    }

    if (continuous && registerAddresses.size())
    {
        m_bridge->readImagerBurst (registerAddresses[0], registerValues);
    }
    else
    {
        for (auto i = 0u; i < registerValues.size(); ++i)
        {
            m_bridge->readImagerRegister (registerAddresses[i], registerValues[i]);
        }
    }
}
