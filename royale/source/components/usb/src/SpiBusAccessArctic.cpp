/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/pal/SpiBusAccessArctic.hpp>
#include <common/EndianConversion.hpp>

using namespace royale::common;
using namespace royale::usb::bridge;
using namespace royale::usb::bridge::arctic;
using namespace royale::usb::pal::arctic;
using namespace royale::pal;

SpiBusAccessArctic::SpiBusAccessArctic (std::shared_ptr<royale::usb::bridge::UvcExtensionArctic> extension) :
    m_extension {extension}
{
}

std::unique_lock<std::recursive_mutex> SpiBusAccessArctic::selectDevice (uint8_t id)
{
    // This locks more than just the SPI
    auto lock = m_extension->getUvcExtensionAccess()->lockVendorExtension();
    m_extension->checkedSet (VendorRequest::SPI_SELECT_DEVICE, id, 0);
    return lock;
}

UvcExtensionArctic &SpiBusAccessArctic::getExtension()
{
    return *m_extension;
}

ISpiBusAccess::ReadSize SpiBusAccessArctic::maximumReadSize ()
{
    // The Arctic firmware allows the full buffer size for both operations, with a clock-stretch
    // between the write and the read. So it's not (MAXIMUM_DATA_SIZE split between write and
    // read), instead having the full amount in each direction is possible, half duplex.
    return {royale::usb::bridge::arctic::MAXIMUM_DATA_SIZE, royale::usb::bridge::arctic::MAXIMUM_DATA_SIZE};
}

std::size_t SpiBusAccessArctic::maximumWriteSize ()
{
    return royale::usb::bridge::arctic::MAXIMUM_DATA_SIZE;
}

void SpiBusAccessArctic::readSpi (const std::vector<uint8_t> &transmit, std::vector<uint8_t> &receive)
{
    m_extension->checkedSetGet (VendorRequest::SPI_READ, 0, 0, transmit, receive);
}

void SpiBusAccessArctic::writeSpi (const std::vector<uint8_t> &transmit)
{
    m_extension->checkedSet (VendorRequest::SPI_WRITE, 0, 0, transmit);
}
