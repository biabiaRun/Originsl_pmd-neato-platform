/***************************************************************************
**                                                                        **
**  Copyright (C) 2016 Infineon Technologies                              **
**  All rights reserved.                                                  **
**                                                                        **
****************************************************************************/

#include <usb/bridge/ArcticProtocolConstants.hpp>

#include <common/EndianConversion.hpp>
#include <common/NarrowCast.hpp>
#include <common/RoyaleLogger.hpp>
#include <common/exceptions/InvalidValue.hpp>

#include <usb/pal/GpioAccessArctic.hpp>

using namespace royale::usb::bridge;
using namespace royale::usb::pal::arctic;
using namespace royale::usb::bridge::arctic;
using namespace royale::pal;
using namespace royale::common;

GpioAccessArctic::GpioAccessArctic (std::shared_ptr<UvcExtensionArctic> extension) :
    m_extension {std::move (extension) }
{
}

void GpioAccessArctic::setGpio (uint32_t id, GpioState state)
{
    ArcticGpioState enumState;
    switch (state)
    {
        case GpioState::LOW:
            enumState = ArcticGpioState::LOW;
            break;
        case GpioState::HIGH:
            enumState = ArcticGpioState::HIGH;
            break;
        case GpioState::Z:
            enumState = ArcticGpioState::Z;
            break;
        default:
            throw InvalidValue ("Unsupported GPIO state");
    }

    m_extension->checkedSet (VendorRequest::GPIO_LOGICAL, enumState, narrow_cast<uint16_t> (id));
}
