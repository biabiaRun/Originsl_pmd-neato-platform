/****************************************************************************\
 * Copyright (C) 2019 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <BridgeImagerL4.hpp>

#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/NarrowCast.hpp>

#include <thread>

using namespace royale;
using namespace royale::common;
using namespace royale::hal;
using namespace spiFlashTool;

BridgeImagerL4::BridgeImagerL4 (std::shared_ptr <royale::ICameraDevice> camera) :
    m_cameraDevice {camera}
{
    CameraAccessLevel level;
    auto status = camera->getAccessLevel (level);
    if (CameraStatus::SUCCESS != status)
    {
        throw RuntimeError ("getAccessLevel failed");
    }
    if (CameraAccessLevel::L3 == level)
    {
        throw LogicError ("L3 means that there's already an IImager implementation controlling the imager, but the IBridgeImager interface suggests there's exclusive access");
    }
    if (CameraAccessLevel::L4 != level)
    {
        throw LogicError ("Insufficient permissions");
    }
}

BridgeImagerL4::~BridgeImagerL4() = default;

void BridgeImagerL4::setImagerReset (bool)
{
    // As noted in the .hpp file, this is silently ignored, which is sufficient for the flash tools.
    // To convert BridgeImagerL4 to be a generic tool, this would need support to be added to
    // Royale's L4 API.
}

void BridgeImagerL4::readImagerRegister (uint16_t regAddr, uint16_t &value)
{
    std::vector<uint16_t> values {0};
    readImagerBurst (regAddr, values);
    value = values.at (0);
}

void BridgeImagerL4::writeImagerRegister (uint16_t regAddr, uint16_t value)
{
    std::vector<uint16_t> values {value};
    writeImagerBurst (regAddr, values);
}

void BridgeImagerL4::readImagerBurst (uint16_t firstRegAddr, std::vector<uint16_t> &values)
{
    royale::Vector<royale::Pair<royale::String, uint64_t>> registers;
    for (size_t idx = 0u; idx < values.size(); idx++)
    {
        registers.push_back ({String::fromUInt (narrow_cast<uint16_t> (firstRegAddr + idx)), 0});
    }

    auto status = m_cameraDevice->readRegisters (registers);
    if (CameraStatus::SUCCESS != status)
    {
        throw RuntimeError ("I/O to the imager failed");
    }

    for (size_t idx = 0; idx < registers.size(); idx++)
    {
        values.at (idx) = narrow_cast<uint16_t> (registers.at (idx).second);
    }
}

void BridgeImagerL4::writeImagerBurst (uint16_t firstRegAddr, const std::vector<uint16_t> &values)
{
    royale::Vector<royale::Pair<royale::String, uint64_t>> registers;
    for (size_t idx = 0u; idx < values.size(); idx++)
    {
        registers.push_back ({String::fromUInt (narrow_cast<uint16_t> (firstRegAddr + idx)), values.at (idx) });
    }

    auto status = m_cameraDevice->writeRegisters (registers);
    if (CameraStatus::SUCCESS != status)
    {
        throw RuntimeError ("I/O to the imager failed");
    }
}

void BridgeImagerL4::sleepFor (std::chrono::microseconds sleepDuration)
{
    std::this_thread::sleep_for (sleepDuration);
}
