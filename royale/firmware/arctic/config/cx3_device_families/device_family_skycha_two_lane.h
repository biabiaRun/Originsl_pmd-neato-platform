/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
/* Common settings for PMD's Skylla/Charybdis boards with two CSI-2 lanes, i.e. Charybdis */

#pragma once

/* Some GPIOs require CyU3PDeviceGpioOverride to be called.  This evaluates to true for GPIOs which
 * should be overridden if they are used.  The FX3 API does this as a sanity check to prevent
 * hardware damage, please be careful of adding GPIOs to it.
 */
#define GPIOS_TO_OVERRIDE(gpio) (gpio == 18 || gpio == 19)

/* If a second SPI device is attached, the GPIO used as its chip select.
 * This line will be initialized to high state, if defined.
 */
#define SPI_DEVICE_1_GPIO 57

/* GPIOs accessible via the GPIO_LOGICAL vendor command (Royale's IGpioAccess interface), other than
 * the RESET and STANDBY GPIOs, are selected by defining PIN_name_GPIO.
 *
 * These will be left in the highZ state, until a GPIO_LOGICAL command is received for this GPIO.
 */
#define PIN_IO_1_GPIO 18
#define PIN_IO_2_GPIO 19
#define PIN_EN_3V3_GPIO 26
#define IMAGER_START_GPIO 23

/* This GPIO, which should be one of the ones accessible via the GPIO_LOGICAL command, will be set
 * to HIGH instead of HighZ state when the firmware is ready.
 *
 * The interaction of this with VENDOR_CMD_DEVICE_RESET is mostly implementation-defined. A hard
 * reset will always finish by setting the GPIO to HIGH, but any other option for that command
 * may leave the GPIO in either HIGH or HighZ state.
 */
#define BOOT_COMPLETE_FLAG_GPIO_HIGH PIN_IO_2_GPIO

/* The number of CSI-2 data lanes to use (if undefined, defaults to 2) */
#define MIPICONFIG_NUM_OF_DATALANES	2
