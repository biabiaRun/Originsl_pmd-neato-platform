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

#include <imager/FlashDefinedImagerComponent.hpp>

namespace royale
{
    namespace imager
    {
        /**
        * Abstract base class for M2453 imagers
        */
        class ImagerM2453 : public FlashDefinedImagerComponent
        {
        public:
            /**
            * The lifecycle of the bridge and config object must be longer than the Imager.
            * This imager uses an external configuration, thus the externalConfig member of
            * the ImagerParameters parameter must point to a valid external configuration.
            *
            * \param   params   Set of parameters that have to be passed to a concrete imager object.
            */
            IMAGER_EXPORT explicit ImagerM2453 (const ImagerParameters &params);
            ~ImagerM2453() override = default;

        protected:
            void doFlashToImagerUseCaseTransfer (const IImagerExternalConfig::UseCaseData &useCaseData) override;

            /**
            * Initiates a SPI transfer from an attached flash memory into the imager configuration container.
            *
            * \param   flashAddress   A 24 bit address of the flash memory the read operation should start from
            * \param   cfgcntOffset   The register offset relative to CFGCNT the write operation should start
            * \param   payloadSize    The count of 16 bit words to transfer
            */
            void doFlashToImagerBlockTransfer (uint32_t flashAddress, uint16_t cfgcntOffset, size_t payloadSize);
            ExpoTimeRegInfo getExpoTimeRegInfo() override;
        };
    }
}
