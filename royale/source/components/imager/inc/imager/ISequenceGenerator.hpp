/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <imager/MeasurementBlock.hpp>

#include <map>

namespace royale
{
    namespace imager
    {
        /**
        * An interface that defines the generic method signature for a strategy that converts a
        * list of measurement blocks into a list of register changes. Imagers may use it by implementing
        * the interface directly or by creating helper classes in their local/anonymous namespace.
        */
        class ISequenceGenerator
        {
        public:
            virtual ~ISequenceGenerator() = default;

            /**
            * This generates an address/value based register configuration out of the
            * configuration provided via the list of MeasurementBlock elements that has
            * been passed as parameter.
            *
            * \param   mbList       A list of MeasurementBlock elements, all elements containing
            *                       intermediate data describing the sequencer configuration but
            *                       not yet assigning this configuration to concrete registers.
            * \param   regChanges   The computed register changes as map of address/value pairs.
            */
            virtual void generateMeasurementBlockConfig (const std::vector<MeasurementBlock> &mbList,
                    std::map < uint16_t, uint16_t > &regChanges) = 0;
        };
    }
}
