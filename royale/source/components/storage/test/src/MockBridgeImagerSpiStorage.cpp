/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/EndianConversion.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/NarrowCast.hpp>

#include <MockBridgeImagerSpiStorage.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <thread>

using namespace royale::common;
using namespace royale::config;
using namespace royale::stub::storage;
using namespace testing;

namespace
{
    // Simulating an M2452 B11
    const std::size_t IMAGER_REG_COUNT = 64 * 1024;
    const uint16_t REG_FIRMWARE_PAGE1_START = 0xA800;

    const uint16_t TRANSFER_BUFFER_START = 0x0000;
    const std::size_t MAX_TRANSFER_SIZE_WORDS = 256 / sizeof (uint16_t);

    const std::size_t FLASH_SECTOR_SIZE_WORDS = 4096 / sizeof (uint16_t);

    const uint16_t REG_ENABLE_iSM = 0xB400;
    const uint16_t REG_RDY = 0x9800;
    const uint16_t REG_CONTINUE = 0x9801;
    const uint16_t REG_BYTE_COUNT = 0xB040;
    const uint16_t REG_COMMAND_ADR2 = 0xB041;
    const uint16_t REG_ADR1 = 0xB042;
    const uint16_t REG_MODE = 0xB043;
    const uint16_t REG_DESIGN_STEP = 0xA0A5;
    const uint16_t VAL_DESIGN_STEP = 0x0B11;

    const uint16_t MASK_COMMAND = 0xff00;
    enum VAL_COMMAND : uint16_t
    {
        READ_DATA = 0x0300,
        WRITE_PAGE = 0x0200,
        ERASE_SECTOR = 0x2000
    };

    const uint16_t MASK_ADR2 = 0xff;

    enum VAL_MODE : uint16_t
    {
        STANDALONE = 1,
        CONTINOUS = 2
    };

    /**
     * If m_willBrownout is true, accessing m_imagerRegisters.at (BROWNOUT_TRIGGER) will change to
     * the failure mode.
     */
    const std::size_t BROWNOUT_TRIGGER = 1050 / sizeof (uint16_t);
}

ImagerAsBridgeType MockBridgeImagerSpiStorage::getSimulatedImagerType ()
{
    return ImagerAsBridgeType::M2452_SPI;
}

MockBridgeImagerSpiStorage::MockBridgeImagerSpiStorage (std::size_t memorySize) :
    m_imagerRegisters (IMAGER_REG_COUNT, 0),
    m_cells (memorySize / 2)
{
    if (memorySize % 2 != 0)
    {
        throw LogicError ("Odd memory size");
    }

    m_designStep = VAL_DESIGN_STEP;
    m_imagerRegisters.at (REG_DESIGN_STEP) = VAL_DESIGN_STEP;
}

MockBridgeImagerSpiStorage::~MockBridgeImagerSpiStorage() = default;

void MockBridgeImagerSpiStorage::setMustReset ()
{
    m_hasBeenReset = false;
}

void MockBridgeImagerSpiStorage::setImagerReset (bool state)
{
    if (state)
    {
        m_hasBeenReset = true;
        m_imagerRegisters.clear();
        m_imagerRegisters.resize (IMAGER_REG_COUNT, 0);
        m_imagerRegisters.at (REG_DESIGN_STEP) = m_designStep;
    }
}

void MockBridgeImagerSpiStorage::setDesignStep (uint16_t designStep)
{
    m_designStep = designStep;
}

std::size_t MockBridgeImagerSpiStorage::setWillBrownoutDuringOperation ()
{
    m_willBrownout = true;
    return BROWNOUT_TRIGGER * sizeof (uint16_t);
}

void MockBridgeImagerSpiStorage::readImagerRegister (uint16_t regAddr, uint16_t &value)
{
    EXPECT_TRUE (m_hasBeenReset);
    ASSERT_GE (IMAGER_REG_COUNT, regAddr);
    value = m_imagerRegisters.at (regAddr);
}

void MockBridgeImagerSpiStorage::writeImagerRegister (uint16_t regAddr, uint16_t value)
{
    EXPECT_TRUE (m_hasBeenReset);
    ASSERT_GE (IMAGER_REG_COUNT, regAddr);
    m_imagerRegisters.at (regAddr) = value;
}

void MockBridgeImagerSpiStorage::readImagerBurst (uint16_t firstRegAddr, std::vector<uint16_t> &values)
{
    EXPECT_TRUE (m_hasBeenReset);
    EXPECT_FALSE (values.empty());
    const auto lastRegAddr = firstRegAddr + values.size() - 1;
    ASSERT_GE (IMAGER_REG_COUNT, lastRegAddr);

    // Reading a burst of registers without checking the iSM status separately seems suspicious
    ASSERT_TRUE (firstRegAddr > REG_ENABLE_iSM || lastRegAddr < REG_ENABLE_iSM) << "Would touch the iSM during a burst";

    for (auto i = 0u; i < values.size(); i++)
    {
        auto regAddr = static_cast<uint16_t> (firstRegAddr + i);
        values.at (i) = m_imagerRegisters.at (regAddr);
    }
}

void MockBridgeImagerSpiStorage::writeImagerBurst (uint16_t firstRegAddr, const std::vector<uint16_t> &values)
{
    EXPECT_TRUE (m_hasBeenReset);
    EXPECT_FALSE (values.empty());
    const auto lastRegAddr = firstRegAddr + values.size() - 1;
    ASSERT_GE (IMAGER_REG_COUNT, lastRegAddr);

    // Enabling the iSM during a burst of registers is probably undefined behavior or at least not
    // thread-safe on the device
    ASSERT_TRUE (firstRegAddr > REG_ENABLE_iSM || lastRegAddr < REG_ENABLE_iSM) << "Would touch the iSM during a burst";

    for (auto i = 0u; i < values.size(); i++)
    {
        auto regAddr = static_cast<uint16_t> (firstRegAddr + i);
        m_imagerRegisters.at (regAddr) = values.at (i);
    }
}

void MockBridgeImagerSpiStorage::checkRunIsm()
{
    if (! m_imagerRegisters.at (REG_ENABLE_iSM))
    {
        return;
    }

    ASSERT_NE (0u, m_imagerRegisters.at (REG_FIRMWARE_PAGE1_START)) << "Didn't load firmware before starting ISM";

    if (m_imagerRegisters.at (REG_MODE) == VAL_MODE::CONTINOUS && ! m_imagerRegisters.at (REG_RDY))
    {
        return;
    }

    // LOG (DEBUG) << "MBISS running iSM with cmd and addr: "
    //     << std::hex << std::setfill ('0') << std::setw (4) << unsigned (m_imagerRegisters.at (REG_COMMAND_ADR2)) << " "
    //     << std::hex << std::setfill ('0') << std::setw (4) << unsigned (m_imagerRegisters.at (REG_ADR1));

    ASSERT_EQ (0, m_imagerRegisters.at (REG_ADR1) % 2);
    const std::size_t wordAddress = ( ( (m_imagerRegisters.at (REG_COMMAND_ADR2) & MASK_ADR2) << 16)
                                      | m_imagerRegisters.at (REG_ADR1)) / 2;

    ASSERT_EQ (0, m_imagerRegisters.at (REG_BYTE_COUNT) % 2);
    const std::size_t wordCount = m_imagerRegisters.at (REG_BYTE_COUNT) / 2;

    if (m_willBrownout && wordAddress <= BROWNOUT_TRIGGER && wordAddress + wordCount > BROWNOUT_TRIGGER)
    {
        LOG (DEBUG) << "Simulating brownout";
        return;
    }

    switch (m_imagerRegisters.at (REG_COMMAND_ADR2) & MASK_COMMAND)
    {
        case READ_DATA:
            {
                ASSERT_GE (MAX_TRANSFER_SIZE_WORDS, wordCount);
                ASSERT_NE (0u, wordCount);
                ASSERT_LE (wordAddress + wordCount - 1, m_cells.size());
                for (auto i = 0u; i < wordCount; i++)
                {
                    m_imagerRegisters.at (TRANSFER_BUFFER_START + i) = m_cells.at (wordAddress + i);
                }
                break;
            }
        case WRITE_PAGE:
            {
                ASSERT_GE (MAX_TRANSFER_SIZE_WORDS, wordCount);
                ASSERT_NE (0u, wordCount);
                ASSERT_LE (wordAddress + wordCount - 1, m_cells.size());
                for (auto i = 0u; i < wordCount; i++)
                {
                    // A previous call to erase() should have set the contents of m_cells.at (...) to
                    // 0xff. To check that, this simulates by only allowing bits to be cleared, not set.
                    m_cells.at (wordAddress + i) = static_cast<uint16_t> (m_cells.at (wordAddress + i) & m_imagerRegisters.at (TRANSFER_BUFFER_START + i));
                }
                break;
            }
        case ERASE_SECTOR:
            {
                ASSERT_EQ (0u, wordAddress % FLASH_SECTOR_SIZE_WORDS);
                for (auto i = 0u; i < FLASH_SECTOR_SIZE_WORDS; i++)
                {
                    m_cells.at (wordAddress + i) = 0xffff;
                }
                break;
            }
        default:
            {
                ADD_FAILURE () << "MockBridgeImagerSpiStorage does not support this function";
            }
    }

    if (m_imagerRegisters.at (REG_MODE) == VAL_MODE::CONTINOUS)
    {
        // Increment the address
        uint32_t adr1 = m_imagerRegisters.at (REG_ADR1) + m_imagerRegisters.at (REG_BYTE_COUNT);
        uint32_t adr2 = (m_imagerRegisters.at (REG_COMMAND_ADR2) & MASK_ADR2) + (adr1 >> 16);
        ASSERT_EQ (0u, adr2 & MASK_COMMAND);
        adr2 |= m_imagerRegisters.at (REG_COMMAND_ADR2) & MASK_COMMAND;
        m_imagerRegisters.at (REG_ADR1) = narrow_cast<uint16_t> (adr1);
        m_imagerRegisters.at (REG_COMMAND_ADR2) = narrow_cast<uint16_t> (adr2);

        // signal that the iSM is ready for more data
        m_imagerRegisters[REG_RDY] = 0;
    }

    if (m_imagerRegisters.at (REG_MODE) == VAL_MODE::STANDALONE || ! m_imagerRegisters.at (REG_CONTINUE))
    {
        // signal that the command is done
        m_imagerRegisters[REG_ENABLE_iSM] = 0;
    }
}

void MockBridgeImagerSpiStorage::sleepFor (std::chrono::microseconds sleepDuration)
{
    checkRunIsm();
}
