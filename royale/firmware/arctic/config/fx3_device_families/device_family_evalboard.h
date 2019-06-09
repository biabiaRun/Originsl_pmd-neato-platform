/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

/* Settings for the FX3 based Evaluation Board from the IRS10x0C Evaluation Kit.  It's referred to
 * here as the generic bring-up board, to match the naming convention of the CX3 bring-up board.
 */

#pragma once

/* The pins that are connected to the imager's reset and standby pins.  These must be defined for
 * FX3 images (this is different to the CX3, which has dedicated pins on the MIPI block)
 */
#define IMAGER_RESET_GPIO       22
#define IMAGER_STANDBY_GPIO     21

/* If defined, the reset pin is not driven high.  This needs an external pull-up resistor (there is
 * one on the EvalKit board), but lets external devices put the imager in reset state. */
#define OPEN_COLLECTOR_RESET

/* Some GPIOs require CyU3PDeviceGpioOverride to be called.  This evaluates to true for GPIOs which
 * should be overridden if they are used.  The FX3 API does this as a sanity check to prevent
 * hardware damage, please be careful of adding GPIOs to it.
 */
#define GPIOS_TO_OVERRIDE(gpio) (gpio == 21 || gpio == 22)

/* If a second SPI device is attached, the GPIO used as its chip select.
 * This line will be initialized to high state, if defined.
 */
#define SPI_DEVICE_1_GPIO 57

/* GPIOs accessible via the GPIO_LOGICAL vendor command (Royale's IGpioAccess interface), other than
 * the RESET and STANDBY GPIOs, are selected by defining PIN_name_GPIO.
 *
 * These will be left in the highZ state, until a GPIO_LOGICAL command is received for this GPIO.
 */
//#define PIN_IO_0_GPIO 17
//#define PIN_IO_1_GPIO 18
//#define PIN_IO_2_GPIO 19
//#define PIN_IO_3_GPIO 44
//#define PIN_IO_4_GPIO 45

//#define PIN_EN_3V3_GPIO 26
//#define PIN_EN_1V5_GPIO 25
//#define PIN_EN_VIL_GPIO 24
//#define PIN_EN_1V8_GPIO 23

/* This GPIO, which should be one of the ones accessible via the GPIO_LOGICAL command, will be set
 * to HIGH or LOW (respectively) instead of HighZ state when the firmware is ready.
 *
 * The interaction of this with VENDOR_CMD_DEVICE_RESET is mostly implementation-defined. A hard
 * reset will always finish by setting this GPIO to the flag state, but any other option for that
 * command may leave the GPIO in either this state or HighZ state.
 */
// #define BOOT_COMPLETE_FLAG_GPIO_HIGH PIN_IO_2_GPIO
// #define BOOT_COMPLETE_FLAG_GPIO_LOW PIN_IO_4_GPIO
