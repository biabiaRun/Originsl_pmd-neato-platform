/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
/* Settings for the most generic FX3-based devices */

#pragma once

/* The pins that are connected to the imager's reset and standby pins.  These must be defined for
 * FX3 images (this is different to the CX3, which has dedicated pins on the MIPI block)
 */
#define IMAGER_RESET_GPIO       22
#define IMAGER_STANDBY_GPIO     21

/* Some GPIOs require CyU3PDeviceGpioOverride to be called.  This evaluates to true for GPIOs which
 * should be overridden if they are used.  The FX3 API does this as a sanity check to prevent
 * hardware damage, please be careful of adding GPIOs to it.
 */
#define GPIOS_TO_OVERRIDE(gpio) (gpio == 21 || gpio == 22)
