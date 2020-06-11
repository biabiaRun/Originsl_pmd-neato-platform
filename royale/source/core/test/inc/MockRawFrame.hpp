/****************************************************************************\
* Copyright (C) 2020 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <common/ICapturedRawFrame.hpp>
#include <gmock/gmock.h>

namespace royale
{
    namespace sensors
    {
        namespace test
        {
        class MockRawFrame : public common::ICapturedRawFrame
        {
        public:
            MockRawFrame() = default;
            virtual ~MockRawFrame() = default;
            MOCK_METHOD( uint16_t *, getImageData, (), (override));
            MOCK_METHOD (const uint16_t *, getPseudoData, (), (const, override));
        };
        }
    }
}
