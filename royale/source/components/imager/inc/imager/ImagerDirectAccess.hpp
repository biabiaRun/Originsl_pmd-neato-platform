/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <imager/ImagerParameters.hpp>

namespace royale
{
    namespace imager
    {
        /**
        * ImagerDirectAccess
        *
        * Template class that exposes direct register access (via readRegisters/writeRegisters)
        * for a software imager implementation.
        * Please consider that any direct register access may make the internal state of the
        * underlying software imager inconsistent (and can mess up the hardware as well).
        * No checks are done here, no guarantees can be made.
        *
        * The BaseImager is expected to offer a ctor with an ImagerParameters argument
        * as well as writeRegistersInternal() and readRegisterInternal() functions.
        * ImagerBase has implementations for the latter two (which makes this class
        * usable for the software defined imager implementations).
        *
        */
        template<class BaseImager>
        class ImagerDirectAccess : public BaseImager
        {
        public:
            explicit ImagerDirectAccess (const ImagerParameters &params) :
                BaseImager (params) {}

            void writeRegisters (const std::vector<uint16_t> &registerAddresses,
                                 const std::vector<uint16_t> &registerValues) override
            {
                this->writeRegistersInternal (registerAddresses, registerValues);
            };

            void readRegisters (const std::vector<uint16_t> &registerAddresses,
                                std::vector<uint16_t> &registerValues) override
            {
                this->readRegistersInternal (registerAddresses, registerValues);
            };
        };
    }
}
