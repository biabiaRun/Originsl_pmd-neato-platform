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

#include <usb/factory/BridgeFactoryArcticCommon.hpp>

#include <string>

namespace royale
{
    namespace factory
    {
        class BridgeFactoryUvcV4l : public BridgeFactoryArcticCommon
        {
        public:
            /**
             * Constructor - the filename is expected to be a /dev/video node.
             */
            BridgeFactoryUvcV4l (const std::string &filename);

            BridgeFactoryUvcV4l (const BridgeFactoryUvcV4l &) = delete;
            const BridgeFactoryUvcV4l &operator= (const BridgeFactoryUvcV4l &) = delete;
            ~BridgeFactoryUvcV4l() override;

            void initialize () override;

        private:
            std::string m_filename;
        };
    }
}
