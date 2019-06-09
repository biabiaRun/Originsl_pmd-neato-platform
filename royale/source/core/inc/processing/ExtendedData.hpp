/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>
#include <royale/IExtendedData.hpp>
#include <royale/RawData.hpp>
#include <royale/DepthData.hpp>
#include <royale/IntermediateData.hpp>
#include <royale/Definitions.hpp>

#include <memory>
#include <cstdint>
#include <vector>
#include <chrono>

namespace royale
{
    namespace processing
    {
        class ExtendedData : public royale::IExtendedData
        {
        public:
            ROYALE_API ExtendedData();
            ROYALE_API ~ExtendedData();

            ROYALE_API void setDepthData (const royale::DepthData *data);
            ROYALE_API void setRawData (const royale::RawData *data);
            ROYALE_API void setIntermediateData (const royale::IntermediateData *data);

            ROYALE_API bool hasDepthData() const override;
            ROYALE_API bool hasRawData() const override;
            ROYALE_API bool hasIntermediateData() const override;

            ROYALE_API const royale::RawData *getRawData() const override;
            ROYALE_API const royale::DepthData *getDepthData() const override;
            ROYALE_API const royale::IntermediateData *getIntermediateData() const override;

        protected:
            const royale::DepthData *m_depthData;
            const royale::RawData *m_rawData;
            const royale::IntermediateData *m_intermediateData;
        };
    }
}
