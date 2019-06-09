/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
/* Common settings for the Pico Monster family of devices */

#pragma once

/* Some GPIOs require CyU3PDeviceGpioOverride to be called.  This evaluates to true for GPIOs which
 * should be overridden if they are used.  The FX3 API does this as a sanity check to prevent
 * hardware damage, please be careful of adding GPIOs to it.
 */
#define GPIOS_TO_OVERRIDE(gpio) (gpio == 17 || gpio == 18 || gpio == 19)
/* GPIOs accessible via the GPIO_LOGICAL vendor command (Royale's IGpioAccess interface), other than
 * the RESET and STANDBY GPIOs, are selected by defining PIN_name_GPIO.
 *
 * These will be left in the highZ state, until a GPIO_LOGICAL command is received for this GPIO.
 */
#define PIN_IO_0_GPIO 17
//#define PIN_IO_1_GPIO Not used
#define PIN_IO_2_GPIO 19
#define PIN_IO_3_GPIO 44
#define PIN_IO_4_GPIO 45

/**
 * As soon as the GPIO block is powered up, set this GPIO low (it will be HighZ until that stage).
 * This happens before the CSI-2 block is powered up.
 *
 * This may be a pin that's also controlled by the GPIO_LOGICAL vendor command, if it is then it
 * will be LOW instead of HighZ until Royale sets it to a different value.
 */
#define BOOT_EARLY_FLAG_GPIO_LOW 25

/* The CX3's MIPI block has a dedicated imager-reset pin. If IMAGER_RESET_GPIO is defined, an
 * additional reset pin will also be supported, commands to set the imager's reset pin will change
 * the outputs of both pins.
 */
#define IMAGER_RESET_GPIO 18
