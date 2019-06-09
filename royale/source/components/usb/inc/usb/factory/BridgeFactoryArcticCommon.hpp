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

#include <usb/bridge/IUvcExtensionAccess.hpp>
#include <usb/factory/IBridgeFactory.hpp>
#include <usb/factory/IBridgeFactoryMira.hpp>

#include <config/ICoreConfig.hpp>
#include <hal/IBridgeDataReceiver.hpp>
#include <hal/IBridgeImager.hpp>
#include <pal/IDeviceControl.hpp>
#include <pal/IGpioAccess.hpp>
#include <pal/II2cBusAccess.hpp>
#include <pal/ISpiBusAccess.hpp>
#include <usb/pal/IDeviceStatus.hpp>

#include <memory>

namespace royale
{
    namespace factory
    {
        /**
         * The UVC and Amundsen bridge factories create an instance of IUvcExtensionAccess,
         * which the IBridgeImager, II2cBusAccess, etc can be constructed on top of.
         *
         * This creates those objects, for devices where the protocol running over the
         * IUvcExtensionAccess is the Arctic protocol.
         */
        class BridgeFactoryArcticCommon : public
            IBridgeFactoryMira,
            IBridgeFactoryImpl<royale::pal::IDeviceControl>,
            IBridgeFactoryImpl<royale::pal::IGpioAccess>,
            IBridgeFactoryImpl<royale::pal::ISpiBusAccess>,
            IBridgeFactoryImpl<royale::usb::pal::IDeviceStatus>
        {
        public:
            BridgeFactoryArcticCommon (const BridgeFactoryArcticCommon &) = delete;
            const BridgeFactoryArcticCommon &operator= (const BridgeFactoryArcticCommon &) = delete;
            ~BridgeFactoryArcticCommon();

        protected:
            BridgeFactoryArcticCommon ();

            /**
             * Common implementation of initialize (SensorMap), which is expected to be called from
             * the subclass' initialize() method.
             *
             * This common method only accesses methods accessible through the named interfaces.
             * For example it doesn't try to cast the arguments to subclasses that have
             * openConnection() or initialize() methods; if the bridge requires such methods to be
             * be called, then the factory subclass must call them itself.
             *
             * The IBridgeDataReceiver and IUvcExtensionAccess may both point to the same object,
             * BridgeFactoryArcticCommon supports this because it doesn't break the encapsulation.
             */
            void initialize (std::shared_ptr<royale::hal::IBridgeDataReceiver> dataReceiver,
                             std::shared_ptr<royale::usb::bridge::IUvcExtensionAccess> extensionAccess);

            // From IBridgeFactory, must be overriden by the subclass.
            void initialize () override = 0;

        private:
            void createImpl (std::shared_ptr<royale::hal::IBridgeImager> &) override;
            void createImpl (std::shared_ptr<royale::hal::IBridgeDataReceiver> &) override;
            void createImpl (std::shared_ptr<royale::pal::IDeviceControl> &) override;
            void createImpl (std::shared_ptr<royale::pal::IGpioAccess> &) override;
            void createImpl (std::shared_ptr<royale::pal::II2cBusAccess> &) override;
            void createImpl (std::shared_ptr<royale::pal::ISpiBusAccess> &) override;
            void createImpl (std::shared_ptr<royale::usb::pal::IDeviceStatus> &) override;

        private:
            std::shared_ptr<royale::hal::IBridgeDataReceiver>                    m_bridge;
            std::shared_ptr<royale::hal::IBridgeImager>                          m_bridgeImager;
            std::shared_ptr<royale::pal::IDeviceControl>                         m_deviceControl;
            std::shared_ptr<royale::pal::IGpioAccess>                            m_gpioAccess;
            std::shared_ptr<royale::pal::II2cBusAccess>                          m_i2cAccess;
            std::shared_ptr<royale::pal::ISpiBusAccess>                          m_spiAccess;
            std::shared_ptr<royale::usb::pal::IDeviceStatus>                     m_deviceStatus;
        };
    }
}
