/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <factory/FlowControlStrategyFactory.hpp>

#include <config/FlowControlStrategyFixed.hpp>
#include <config/FlowControlStrategy100k.hpp>

#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/ResourceError.hpp>

using namespace royale::factory;
using namespace royale::common;
using namespace royale::config;


std::shared_ptr<royale::common::IFlowControlStrategy> FlowControlStrategyFactory::createFlowControl (
    std::shared_ptr<royale::hal::IBridgeDataReceiver> bridgeDataReceiver,
    royale::config::BandwidthRequirementCategory bandwidthRequirementCategory)
{
    auto busTransferSpeed = bridgeDataReceiver->getPeakTransferSpeed();
    bool isUsb3SuperSpeed = (busTransferSpeed >= 250e6f);

    switch (bandwidthRequirementCategory)
    {
        case BandwidthRequirementCategory::NO_THROTTLING:
            return nullptr;  // no flow control required
        case BandwidthRequirementCategory::USB2_THROTTLING:
            if (isUsb3SuperSpeed)
            {
                return nullptr;  // no flow control required
            }
            else
            {
                return std::make_shared<royale::common::FlowControlStrategyFixed> (230);
            }
        case BandwidthRequirementCategory::USB3_THROTTLING:
            if (isUsb3SuperSpeed)
            {
                /*
                * \todo ROYAL-669 may one day replace this by a PID based flow control.
                */
                return std::make_shared<royale::common::FlowControlStrategy100k> (190, 320);
            }
            else
            {
                throw ResourceError ("Insufficient Bandwidth", "This camera must be connected to a USB3 port");
            }
        default:
            throw LogicError ("Bandwidth requirement category not supported.");
    }
}

royale::Vector<std::shared_ptr<royale::common::IFlowControlStrategy>> FlowControlStrategyFactory::enumerateStrategies (
            royale::config::BandwidthRequirementCategory bandwidthRequirementCategory)
{
    switch (bandwidthRequirementCategory)
    {
        case BandwidthRequirementCategory::USB2_THROTTLING:
            return {std::make_shared<royale::common::FlowControlStrategyFixed> (230) };
        case BandwidthRequirementCategory::USB3_THROTTLING:
            return {std::make_shared<royale::common::FlowControlStrategy100k> (190, 320) };
        case BandwidthRequirementCategory::NO_THROTTLING:
        default:
            return {};
    }
}
