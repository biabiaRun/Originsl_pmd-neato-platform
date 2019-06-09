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

#include <common/ISensorRoutingConfig.hpp>

#include <royale/Definitions.hpp>
#include <royale/String.hpp>

#include <cstdint>

namespace royale
{
    namespace config
    {
        /**
         * This ISensorRoutingConfig indicates that a file on the filesystem provides the data for
         * an IStorageReadRandom or INonVolatileStorage.
         *
         * This is currently intended only for use during bring-up, so how the filename maps to the
         * filesystem (particularly which directory it's in, but also what the directory separators
         * are) is implementation defined.
         */
        class SensorRoutingFilename : public royale::common::ISensorRoutingConfig
        {
        public:
            explicit SensorRoutingFilename (const royale::String &filename) :
                m_filename (filename)
            {
            }

            const royale::String &getFilename() const
            {
                return m_filename;
            }

        private:
            const royale::String m_filename;
        };
    }
}


