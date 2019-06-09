/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#pragma once

#include <factory/ModuleConfigFactoryByStorageId.hpp>

namespace royale
{
    namespace factory
    {
        /**
         * A factory that returns the ModuleConfigs for devices using the CX3-based Animator
         * bring-up board.  This is a bring-up board, expected to be used by hardware developers to
         * test new modules, so the configuration needs to be changed to match the hardware on the
         * board.
         *
         * This factory reads an identifier from a V7 header (as normally used by the calibration
         * data) stored in the same SPI flash as the CX3 firmware. If the identifier can't be read,
         * or if an unknown identifier is found, it will return the AnimatorDefault reference config
         * for the Animator board.
         *
         * By default it will return the config for the C2Uvc camera, note that this device normally
         * stores its calibration data in the EEPROM instead of the SPI flash.
         */
        class ModuleConfigFactoryAnimatorBoard : public ModuleConfigFactoryByStorageId
        {
        public:
            explicit ModuleConfigFactoryAnimatorBoard();
        };
    }
}
