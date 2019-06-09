/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
/* Settings Infineon / PMD's Animator bring-up board.  This is the "flagship" for CX3,
 * all customisation options are documented here, even ones that are not used for the
 * standard config.
 */

#pragma once

/* Some GPIOs require CyU3PDeviceGpioOverride to be called.  This evaluates to true for GPIOs which
 * should be overridden if they are used.  The FX3 API does this as a sanity check to prevent
 * hardware damage, please be careful of adding GPIOs to it.
 */
#define GPIOS_TO_OVERRIDE(gpio) (gpio == 17 || gpio == 18 || gpio == 19)

/* If a second SPI device is attached, the GPIO used as its chip select.
 * This line will be initialized to high state, if defined.
 */
#define SPI_DEVICE_1_GPIO 57

/* GPIOs accessible via the GPIO_LOGICAL vendor command (Royale's IGpioAccess interface), other than
 * the RESET and STANDBY GPIOs, are selected by defining PIN_name_GPIO.
 *
 * These will be left in the highZ state, until a GPIO_LOGICAL command is received for this GPIO.
 */
#define PIN_IO_0_GPIO 17
#define PIN_IO_1_GPIO 18
#define PIN_IO_2_GPIO 19
#define PIN_IO_3_GPIO 44
#define PIN_IO_4_GPIO 45

#define PIN_EN_3V3_GPIO 26
#define PIN_EN_1V5_GPIO 25
#define PIN_EN_VIL_GPIO 24
#define PIN_EN_1V8_GPIO 23

/* This GPIO, which should be one of the ones accessible via the GPIO_LOGICAL command, will be set
 * to HIGH or LOW (respectively) instead of HighZ state when the firmware is ready.
 *
 * The interaction of this with VENDOR_CMD_DEVICE_RESET is mostly implementation-defined. A hard
 * reset will always finish by setting this GPIO to the flag state, but any other option for that
 * command may leave the GPIO in either this state or HighZ state.
 */
// #define BOOT_COMPLETE_FLAG_GPIO_HIGH PIN_IO_2_GPIO
// #define BOOT_COMPLETE_FLAG_GPIO_LOW PIN_IO_4_GPIO

/**
 * This GPIO will be set to HIGH or LOW (respectively) as soon as the GPIO block is powered up.
 * This happens before the CSI-2 block is powered up.
 *
 * This may be a pin that's also controlled by the GPIO_LOGICAL vendor command, if it is then it
 * will be in the chosen state instead of HighZ until a GPIO_LOGICAL command sets it to a different
 * value.
 */
//#define BOOT_EARLY_FLAG_GPIO_HIGH 25
//#define BOOT_EARLY_FLAG_GPIO_LOW 25

/* The number of CSI-2 data lanes to use (if undefined, defaults to 2) */
// #define MIPICONFIG_NUM_OF_DATALANES	2
