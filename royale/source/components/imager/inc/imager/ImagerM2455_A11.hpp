/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <imager/ImagerM2453.hpp>

namespace royale
{
    namespace imager
    {
        using ImagerM2455Serial = ImagerSimpleHexSerialNumber;

        /**
        * Support for the M2455, currently only the A11 design step.
        *
        * The imager is similar to the M2453 with a higher image resolution. It's handled here as if
        * it was another design step of the M2453.
        */
        class ImagerM2455_A11 : public ImagerM2453
        {
        public:
            /**
            * This imager uses an external configuration, thus the externalConfig member of
            * the ImagerParameters parameter must point to a valid external configuration.
            *
            * \param   params   Set of parameters that have to be passed to a concrete imager object.
            */
            IMAGER_EXPORT explicit ImagerM2455_A11 (const ImagerParameters &params);
            ~ImagerM2455_A11() override = default;

            std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter() override;
            void reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex) override;

            void initialize() override;

        protected:
            std::vector < uint16_t > getSerialRegisters() override;
        };
    }
}
