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

namespace royale
{
    namespace pal
    {
        /**
         * Low-level miscellaneous controls, such as resetting the device.
         *
         * Some functions may be unsupported on some devices.
         */
        class IDeviceControl
        {
        public:
            virtual ~IDeviceControl() = default;

            /**
             * Reset many options which can be changed by other methods and interfaces back to the
             * default values.  For example all GPIOs that can be controlled by Royale are reset, if
             * a device supports II2cBusAccess::setBusSpeed then this will undo those changes, etc.
             *
             * This may be less than a full power-cycle of the device.  Only peripherals that are
             * known to the firmware are likely to be reset, even ones that are known to Royale (for
             * example the temperature sensors) are likely to be left unchanged.
             *
             * The imager will be reset.
             *
             * For devices which are an integrated part of the device's main processor, for example
             * the Cypress CX3's MIPI receiver, the integrated peripheral is likely to be reset.
             * Communication buses are likely to be reset, other devices may be unchanged.
             *
             * Depending on the implementation, this may require the device to be re-opened via
             * CameraManager (all further I/O calls to the current ICameraDevice instance may throw
             * Disconnected exceptions).
             */
            virtual void resetDevice() = 0;

            /**
             * If the device supports resetting to the state after a cold boot of the device,
             * trigger that reset.  If this is unsupported then no reset is done, even for devices
             * where resetDevice() would succeed.
             *
             * If this fails to reset the device, an exception will be thrown.  Even if it succeeds,
             * there's a possibility that an exception will be thrown.
             */
            virtual void resetDeviceHard() = 0;

            /**
             * Disable the SPI master, and set all of its IO pins to high impedence.
             * This may allow a different device to act as the SPI bus master.
             *
             * @param disable true to turn the SPI off, false to re-enable it
             */
            virtual void setSpiDisableAndHighZ (bool disable) = 0;
        };
    }
}
