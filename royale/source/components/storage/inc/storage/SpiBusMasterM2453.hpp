/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <storage/SpiBusMasterFlashBasedDevice.hpp>
#include <hal/IBridgeImager.hpp>

#include <cstdint>
#include <memory>

namespace royale
{
    namespace storage
    {
        /**
         * This class is the special bus master implementation for the M2453.
         */
        class SpiBusMasterM2453 : public SpiBusMasterFlashBasedDevice
        {
        public:
            ROYALE_API SpiBusMasterM2453 (std::shared_ptr<royale::hal::IBridgeImager> access);
            ~SpiBusMasterM2453 () override;

        private:
            void initializeOnce() override;
        };
    }
}
