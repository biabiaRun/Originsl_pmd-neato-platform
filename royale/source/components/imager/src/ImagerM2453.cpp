/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerM2453.hpp>
#include <imager/M2453/ImagerRegisters.hpp>
#include <imager/M2453/PseudoDataInterpreter.hpp>

#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/NarrowCast.hpp>

using namespace std;
using namespace royale::imager;
using namespace royale::common;

namespace
{
    /**
    * Hardware limit for payload, in 8-bit bytes, please refer to Jama item MIB-SET-348
    * for detailed information. At the time of implementing the code using
    * the numBytesPerTransfer constant the Jama content was:
    *
    * The start address of the selected memory section has to be written to the SPI_WR_ADDR register.
    * Also the  length of the data packet has to be specified in the register SPI_LEN.
    * The maximum payload per SPI write is 256 Bytes. This means that SPI_LEN can be Maximum 259 (= 260 -1):
    * 1 Byte: command
    * 3 Bytes: address
    * 256 Bytes: payload
    */
    const std::size_t numBytesPerTransfer = 256;

    /** Hardware limit for payload, in 16-bit registers */
    const std::size_t maxPayloadSize = 128;
    const std::size_t regSize = 2;
    static_assert (numBytesPerTransfer == maxPayloadSize *regSize,
                   "payload size in bytes doesn't match payload size in registers");

    /**
    * Duration of the unconditional sleep after setting SPITRIG, before first polling for the
    * status.  The value 150us is chosen as the worst case value for the transfer of the command,
    * address and one full payload of numBytesPerTransfer bytes over the imager's SPI bus assuming
    * the System Clock is 160MHz, and that the SPI is configured to run at sys_clock/8.
    */
    const auto DEFAULT_TIME_BLOCK_TRANSFER = std::chrono::microseconds (150);

    /**
    * The conditional sleep between retries of reading the status.
    */
    const auto POLLING_INTERVAL = std::chrono::milliseconds (10);
}

ImagerM2453::ImagerM2453 (const ImagerParameters &params) :
    FlashDefinedImagerComponent (params)
{
}

ExpoTimeRegInfo ImagerM2453::getExpoTimeRegInfo()
{
    ExpoTimeRegInfo info;
    info.nSequenceEntries = M2453::nSequenceEntries;
    info.CFGCNT_S00_EXPOTIME_Address = M2453::CFGCNT_S00_EXPOTIME;
    info.CFGCNT_S01_EXPOTIME_Address = M2453::CFGCNT_S01_EXPOTIME;
    info.CFGCNT_FLAGS_Address = M2453::CFGCNT_FLAGS;
    return info;
}

void ImagerM2453::doFlashToImagerUseCaseTransfer (const IImagerExternalConfig::UseCaseData &useCaseData)
{
    const auto flashAddress = useCaseData.sequentialRegisterHeader.flashConfigAddress;
    const auto flashConfigSize = useCaseData.sequentialRegisterHeader.flashConfigSize;

    if (useCaseData.sequentialRegisterHeader.flashConfigSize % regSize)
    {
        throw LogicError ("Data size is not a multiple of the number of bytes per register");
    }
    if ( (useCaseData.sequentialRegisterHeader.imagerAddress != 0) &&
            (useCaseData.sequentialRegisterHeader.imagerAddress != M2453::CFGCNT))
    {
        // This would just need a change to doFlashToImagerBlockTransfer, but for now let's just
        // disallow it, as it's probably an error in the useCaseData.
        throw NotImplemented ("Loading a use case from SPI to an unexpected address");
    }
    auto registerCount = flashConfigSize / regSize;

    for (std::size_t i = 0u; i < registerCount; i += maxPayloadSize)
    {
        const auto blockSize = std::min (maxPayloadSize, registerCount - i);

        doFlashToImagerBlockTransfer (narrow_cast<uint32_t> (flashAddress + i * regSize),
                                      narrow_cast<uint16_t> (i),
                                      blockSize);
    }
}

void ImagerM2453::doFlashToImagerBlockTransfer (uint32_t flashAddress, uint16_t cfgcntOffset, size_t payloadSize)
{
    //spiLen == 0 means 1 byte, adding 3 to the payloadSize as there are two 16 bit words for command and address
    const auto spiLen = static_cast<uint16_t> ( (payloadSize << 1) + 3u);
    const auto readBufferAddress = M2453::CFGCNT;

    const auto spiEnable = 1u << 14;
    const auto spiClockDiv8 = 2u;
    m_bridge->writeImagerRegister (M2453::SPICFG, static_cast<uint16_t> (spiEnable | spiClockDiv8));

    //put the SPI query data into the pixel memory
    m_bridge->writeImagerRegister (M2453::SPIWRADDR, M2453::PIXMEM);
    m_bridge->writeImagerRegister (M2453::PIXMEM,
                                   static_cast<uint16_t> ( (0x03 << 8) | (flashAddress >> 16)));
    m_bridge->writeImagerRegister (static_cast<uint16_t> (M2453::PIXMEM + 1u),
                                   static_cast<uint16_t> (flashAddress & 0xFFFF));

    //data read from flash should be transferred to the configuration container
    m_bridge->writeImagerRegister (M2453::SPIRADDR, static_cast<uint16_t> (readBufferAddress + cfgcntOffset));
    m_bridge->writeImagerRegister (M2453::SPILEN, static_cast<uint16_t> ( (0x07 << 13) | spiLen));

    //initiate the transfer
    m_bridge->writeImagerRegister (M2453::SPITRIG, 2u);

    //wait for the transfer to finish
    m_registerAccess->pollUntil (M2453::SPISTATUS, 1u, DEFAULT_TIME_BLOCK_TRANSFER, POLLING_INTERVAL);
}
