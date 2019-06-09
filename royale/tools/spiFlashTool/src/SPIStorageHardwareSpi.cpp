/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <SPIStorageHardwareSpi.hpp>
#include <iomanip>
#include <common/RoyaleLogger.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::storage;
using namespace spiFlashTool::storage;

SPIStorageHardwareSpi::SPIStorageHardwareSpi (std::shared_ptr<royale::pal::ISpiBusAccess> spiAccess,
        const SensorRoutingConfigSpi &busAddress,
        IProgressReportListener *progressListener,
        std::size_t accessOffset) :
    SPIStorageBase (progressListener)
{
    const auto config = royale::config::FlashMemoryConfig().setAccessOffset (accessOffset);
    m_flash = std::make_shared<SpiGenericFlash> (config, spiAccess, busAddress);
}

void SPIStorageHardwareSpi::readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer)
{
    LOG (DEBUG) << "SPIStorage::readStorage, addr=0x" << std::hex << startAddr
                << ", size=" << std::dec << recvBuffer.size();
    m_flash->readStorage (startAddr, recvBuffer);
}

void SPIStorageHardwareSpi::writeStorage (const std::vector<uint8_t> &buffer)
{
    writeStorage (0, buffer);
}

void SPIStorageHardwareSpi::writeStorage (std::size_t startAddr, const std::vector<uint8_t> &buffer)
{
    LOG (DEBUG) << "SPIStorage::writeStorage, size=" << std::dec << buffer.size();
    m_flash->writeSectorBased (startAddr, buffer);
}
