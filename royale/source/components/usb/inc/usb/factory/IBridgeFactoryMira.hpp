/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <usb/factory/IBridgeFactory.hpp>

#include <hal/IBridgeImager.hpp>
#include <hal/IBridgeDataReceiver.hpp>
#include <pal/II2cBusAccess.hpp>

namespace royale
{
    namespace factory
    {
        /**
         * Collection of interfaces needed for bridge factories which work with CameraCoreBuilderMira
         *
         * Depending on the type of flash memory used, the bridge factory may
         * additionally need one of the following interfaces:
         * - IBridgeFactoryImpl<IBridgeWithPagedFlash>
         * - IBridgeFactoryImpl<ISpiBusAccess>
         *
         */
        class IBridgeFactoryMira : public
            IBridgeFactoryImpl<royale::hal::IBridgeImager>,
            IBridgeFactoryImpl<royale::hal::IBridgeDataReceiver>,
            IBridgeFactoryImpl<royale::pal::II2cBusAccess>
        {
        public:
            virtual ~IBridgeFactoryMira() = default;

            virtual void createImpl (std::shared_ptr<royale::hal::IBridgeImager> &) override = 0;
            virtual void createImpl (std::shared_ptr<royale::hal::IBridgeDataReceiver> &) override = 0;
            virtual void createImpl (std::shared_ptr<royale::pal::II2cBusAccess> &) override = 0;
        };

    }
}
