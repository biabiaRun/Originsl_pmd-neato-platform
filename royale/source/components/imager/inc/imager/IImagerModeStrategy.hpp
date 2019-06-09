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

#include <imager/ISequenceGenerator.hpp>

namespace royale
{
    namespace imager
    {
        /**
        * The imager can run in both normal mode and in mixed mode.
        * This class abstracts away the differences between these two modes for measurement block based imagers.
        */
        class IImagerModeStrategy : public ISequenceGenerator
        {
        public:
            ~IImagerModeStrategy() override = 0;

            /**
            * For some imagers the registers for a reconfiguration (reconfiguration means the imager
            * changes parameters while being in the state ImagerState::Capturing)
            * must be translated to different address/value pairs. The reconfigure method uses this
            * translation to convert the register changes from shadow-config area to the safe-reconfig
            * area before they are written.
            *
            * A runtime exception must be thrown if any element of regChanges cannot be translated.
            * The reconfigure method of the imager in turn shall write the translated configuration,
            * trigger the firmware of the imager to accept this configuration and update the tracked
            * register map m_regDownloaded after reading the confirmation about the successful register
            * transfer from the firmware.
            * Please note that it is not mandatory that the size of the result map equals the size of
            * the input map.
            *
            * \param  mbCount      The number of measurement blocks
            * \param  regChanges   The desired register changes as map of address/value pairs.
            * \return              The translated address/value pairs
            */
            virtual std::map < uint16_t, uint16_t > reconfigTranslation (const std::size_t mbCount, const std::map < uint16_t, uint16_t > &regChanges) const = 0;

            /**
            * This function is common for all imagers with the AllInOne firmware using mixed mode.
            * It takes a list of changes registers and translates the changes to the special
            * safe-reconfig area (see firmware documentation for further information).
            *
            * \param   mbCount        The number of measurement blocks (MBs) used.
            * \param   mbStart        The address of the first entry of the first MB in the DMEM shadow area
            * \param   mbOffset       The address offset between two MBs
            * \param   seqIdxOffset   The number of registers one sequence entry consist of
            * \param   flags          The address of the so-called flags parameter (see firmware documentation)
            * \param   maxParams      The maximum number of parameter changable by a single reconfiguration
            * \param   srParamStart   The address of the first parameter in the CFGCNT safe reconfig area
            * \param   srDecodeStart  The address of the first decode in the CFGCNT safe reconfig area
            * \param   regChanges     The register configuration to be translated to a mixed mode safe reconfiguration
            * \return A set of registers prepared to be written to the imager
            */
            static const std::map < uint16_t, uint16_t > reconfigTranslationMixedMode (
                const std::size_t mbCount,
                const uint16_t mbStart,
                const uint16_t mbOffset,
                const uint16_t seqIdxOffset,
                const uint16_t flags,
                const uint16_t maxParams,
                const uint16_t srParamStart,
                const uint16_t srDecodeStart,
                const std::map < uint16_t, uint16_t > &regChanges);
        };
    }

}
