/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <common/IFlowControlStrategy.hpp>
#include <hal/IBridgeDataReceiver.hpp>
#include <config/BandwidthRequirementCategory.hpp>

#include <memory>

namespace royale
{
    namespace factory
    {
        class FlowControlStrategyFactory
        {
        public:
            static std::shared_ptr<royale::common::IFlowControlStrategy> createFlowControl (
                std::shared_ptr<royale::hal::IBridgeDataReceiver> bridgeDataReceiver,
                royale::config::BandwidthRequirementCategory bandwidthRequirementCategory);

            /**
             * Return a possibly-incomplete list of all strategies that could be returned by
             * createAllFlowControls.  This is intended for sanity checks such as
             * UnitTestModuleConfig's tests.
             *
             * The list may be incomplete, for example if the createFlowControl calculates a
             * strategy from the exact bandwidth available then this may return a few example
             * values.
             */
            ROYALE_API static royale::Vector<std::shared_ptr<royale::common::IFlowControlStrategy>> enumerateStrategies (royale::config::BandwidthRequirementCategory bandwidthRequirementCategory);

        }; // class FlowControlStrategyFactory

    } // namespace factory

} // namespace royale
