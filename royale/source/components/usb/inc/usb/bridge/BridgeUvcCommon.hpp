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

#include <royale/Definitions.hpp>

#include <buffer/BridgeCopyAndNormalize.hpp>
#include <usb/pal/UsbSpeed.hpp>

#include <common/EventForwarder.hpp>

#include <atomic>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            /**
             * Common code for USB Video Class bridges using Windows' DirectShow or Media Foundation
             * frameworks, or a similar UVC library.
             */
            class BridgeUvcCommon : public royale::buffer::BridgeCopyAndNormalize
            {
            public:
                ROYALE_API explicit BridgeUvcCommon();
                ROYALE_API ~BridgeUvcCommon() override;

                /**
                 * @inheritdoc
                 *
                 * The data from setUvcBridgeInfo is added to the data returned by
                 * BridgeCopyAndNormalize::getBridgeInfo().
                 */
                royale::Vector<royale::Pair<royale::String, royale::String>> getBridgeInfo() override;

                /**
                 * The data passed to this is returned by getBridgeInfo.
                 *
                 * At the time of writing, it's implementation defined whether calling it a second
                 * time merges the two sets of data or simply overwrites all data from previous
                 * calls.
                 */
                void setUvcBridgeInfo (royale::Vector<royale::Pair<royale::String, royale::String>> &&info);

            private:
                royale::Vector<royale::Pair<royale::String, royale::String>> m_uvcBridgeInfo;
            };
        }
    }
}
