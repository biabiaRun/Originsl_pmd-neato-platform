Copyright (C) 2018 Infineon Technologies

All of the prebuilt .img files contain code Copyright (C) Cypress
Semiconductor, and code Copyright (C) Infineon Technologies.

The ThreadX operating system from Express Logic is used as the
embedded RTOS in these devices, Copyright (C) Express Logic Inc.

These .img files may only be used as agreed in the software
license agreement under which you received them.

THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Prebuilt images
===============

The following table shows the prebuilt images which are supplied.

To find out which features (GPIOs, etc) each image supports, refer to the device family. For
example, `cx3_amundsen_u7.img` uses `cx3_device_family/device_family_skycha_two_lane.h`.  If the
family column contains '-', that image is built without including any of the family files, therefore
without any of the features that can be enabled in those files.

| Filename (followed by .img)    | USB ID    | Device family      | Manufac. | Name in USB descriptor               | Description                                   |
|--------------------------------|-----------|--------------------|----------|--------------------------------------|-----------------------------------------------|
| `cx3_amundsen_animator`        | 058b 00a1 | `animator`         | Infineon | Amundsen CX3 Animator                | non-UVC image for Animator boards             |
| `cx3_uvc_animator`             | 058b 00a6 | `animator`         | Infineon | Infineon IRS16x5C Evaluation Kit CX3 | generic Infineon / PMD bring-up boards        |
| `cx3_uvc_picomonstar2`         | 1c28 c019 | `monstar`          | PMD      | pico monstar 2                       | pico monstar with configuration 2             |
| `cx3_amundsen_picomonstar`     | 1c28 c025 | `monstar`          | PMD      | pico monstar                         | pico monstar                                  |
| `cx3_amundsen_salome`          | 1c28 c028 | `skycha_two_lane`  | PMD      | Salome                               | PMD's Salome family                           |
| `cx3_amundsen_x1`              | 1c28 c02d | `skycha_two_lane`  | PMD      | X1                                   | PMD's X1 family                               |
| `cx3_amundsen_u7`              | 1c28 c030 | `skycha_two_lane`  | PMD      | U7                                   | PMD's U7 family                               |
| `cx3_amundsen_selene`          | 1c28 c031 | `skycha_two_lane`  | PMD      | Selene                               | PMD's Selene family                           |
| `cx3_amundsen_orpheus`         | 1c28 c032 | `skycha_two_lane`  | PMD      | Orpheus                              | PMD's Orpheus family                          |
| `cx3_amundsen_pmdmodule`       | 1c28 c033 | `skycha_two_lane`  | PMD      | pmdModule                            | PMD module                                    |
| `cx3_amundsen_pmdmodule277x`   | 1c28 c039 | `skycha_two_lane`  | PMD      | pmdModule277x                        | PMD module 277x                               |

The difference between UVC and Amundsen images
----------------------------------------------

UVC (USB Video Class) is a USB standard for cameras, so devices running UVC firmware will be
typically be recognised as cameras by the host's operating system.

Devices running Amundsen firmware identify themselves as using a vendor-specific protocol, and don't
identify themselves as UVC, so that the device isn't claimed by any OS-provided camera server.  On
Windows they require a driver to be installed (the CyApi driver which is also used by Enclustra).

The UVC protocol uses a composite device, with separate interfaces for control and video streaming;
when running the Amundsen firmware a single interface is used for both functions.

Some devices have more UVC images than Amundsen images, which is related to the history of Royale's
support for multi-stage device probing.  When the UVC images for the pico maxx and pico monstar were
originally added, the variants needed different VID:PID pairs for each variant; when the Amundsen
images were added one image could be used for all variants.  For the maxx and monstar, note that
Royale may require L3 access to open Amundsen devices, even though the device opens with L1 when
using UVC firmware.  This is explained below in the "Royale, eye-safety and product identifiers"
section.

The `cx3_amundsen_animator.img` and `cx3_uvc_animator.img`
----------------------------------------------------------

The Animator images are for using the Animator board for developing new cameras.  Royale recognises
the Animator firmware, and searches for an identifier stored in the device which tells it which
configuration to load.  There is more information in Royale's ModuleConfigFactoryAnimatorBoard.hpp.

skycha_two_lane Device Family
-----------------------------

Originally introduced for the Skylla and LiteMore (Charybdis) images, that were intended for
developing new cameras based on the Skylla board; they support different GPIOs to the Animator
image.  Please refer to the device family file for the list of GPIOs.

The Skylla used only 1 CSI-2 data lane, the other devices all use 2 lanes and therefore use
`device_family_skycha_two_lane.h`.

The Pico Monstar images
-----------------------

These are for production modules.  The pico monstar's images set a GPIO during boot.

Flashing instructions
=====================

Install the FX3 dev kit from Cypress (FX3 also covers CX3).
http://www.cypress.com/documentation/software-and-drivers/ez-usb-fx3-software-development-kit

From the dev kit, open "EZ-USB FX3 SDK/GettingStartedWithFX3SDK.pdf". Instructions for flashing are in section 3 of this PDF.

Set up the device
-----------------

Skip most of section 3.1 (building an image), and just set up the device (this corresponds to section 3.1.3 of the PDF).

1. Unplug
2. Put the device in to boot-from-USB mode, using the DIP switch settings below.
3. When it's plugged back in, Windows should detect a "WestBridge" device.

If the memory selected does not contain a firmware image, the CX3 will automatically fall back to
USB boot mode.

### Devices that are already running Royale-compatible firmware

If the device doesn't have accessible switches to select the boot mode, but is already usable by
Royale, Royale's eraseFlash tool can be used to erase the existing firmware and trigger the CX3's
fallback to USB boot mode.  Power-cycle the device after erasing the firmware to reboot it.

If you are switching a device between Enclustra and Arctic firmware, note that Royale's support for
calibration storage requires a different configuration, and a different SPI or EEPROM layout, for
the two types of firmware.  Royale's calibrationFlashTool can be used to copy the calibration before
switching the firmware type, and then to restore it with the new firmware type.

### Which board do I have?

The boards can be recognised either by their text label, or more quickly by their size.

| Board    | Size (cm) | Text label                              |
|----------|-----------|-----------------------------------------|
| Animator | 8 x 13    | CX3 BrUp Platform Interface Boardb V2.0 |
| mini     | 3 x 4     | C2 CX-3 BB v1.0                         |
| Skylla   | 1.5 x 4.5 | Skylla CX3 V1.0                         |

### The Animator board

| P2 (SW2-3) | P1 (SW2-2) | P0 (SW2-1) | Mode |
|------------|------------|------------|------|
| OFF        | ON         | ON         | USB  |
| OFF        | ON         | OFF        | I2C  |
| ON         | OFF        | ON         | SPI  |

Note: check which version of the board you have (printed on the bottom side).  The difference is
that the switch was mounted in reverse on version 1, so the switch numbering does not correspond to
the pin numbering. In both cases this table covers the options, just make sure you pay attention to
the Px label on the board, instead of switch numbers, if you use version 1.

### The mini CX3 board

With the mini CX3 board, the boot mode is controlled by the 2-switch DIP.

|  1  |  2  | Mode |
|-----|-----|------|
| OFF | ON  | SPI  |
| ON  | OFF | USB  |

The usual setting is boot-from-SPI, moving both switches to the opposite position selects the
boot-from-USB mode.  In both modes, one switch is ON and the other is OFF.

Flashing the firmware
---------------------

Follow sections 3.2 and 3.3 of the PDF, programming to RAM.

Using the device
----------------

The board should automatically reset itself after programming, and show up as an updated device for
Royale.

Every time the USB device is physically disconnected, it will reset back to bootloader mode. To
permanently update it, flash to SPI and then move the DIP switches back to the SPI position.

For Amundsen devices on Windows, you must close the Cypress Control Center before Royale will be
able to open the device.

Royale, eye safety, and product identifiers
-------------------------------------------

Flashing an Amundsen image to an existing UVC device is likely to need L3 access to complete the
change.  In Royale, Amundsen images will typically open only with L3 access, until a product
identifier has been written to them; this will also be the case for any UVC devices which weren't
supported in older versions of Royale.

Devices which are both supported by Royale v3.8.0 and certified as Laser Class 1 still have to be
eye-safe when used with the older Royale. This limitation on devices means that new variants of the
maxx and monstar that use the existing UVC VID:PID pair must be eye-safe with a version of Royale
that doesn't read the product identifier.

For this reason, the behavior of Royale v3.9.0 for devices without these identifiers depends on
whether Royale v3.8.0 supported a given VID:PID.

For images with VID:PID pairs that are newly introduced in Royale v3.9.0, the default behavior of
Royale is to refuse to open the device without L3 access, and with L3 access to open the device in a
calibration-only mode.
