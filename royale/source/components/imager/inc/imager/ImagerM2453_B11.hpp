/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
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
        /**
        * This class is prepared for the M2453 imager and is compatible to the design step B11.
        */
        class ImagerM2453_B11 : public ImagerM2453
        {
        public:
            /**
            * The lifecycle of the bridge and config object must be longer than the Imager.
            * This imager uses an external configuration, thus the externalConfig member of
            * the ImagerParameters parameter must point to a valid external configuration.
            *
            * \param   params   Set of parameters that have to be passed to a concrete imager object.
            */
            IMAGER_EXPORT explicit ImagerM2453_B11 (const ImagerParameters &params);
            ~ImagerM2453_B11() override = default;

            std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter() override;
            void reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex) override;

            void initialize() override;

        protected:
            std::vector < uint16_t > getSerialRegisters() override;
        };
    }
}
