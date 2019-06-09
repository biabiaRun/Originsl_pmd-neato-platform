/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <royalev4l/bridge/BridgeV4l.hpp>

#include <common/exceptions/Disconnected.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/MakeUnique.hpp>

#include <gtest/gtest.h>

using namespace royale::v4l::bridge;
using namespace royale::common;

TEST (TestBridgeV4l, OpenDevNull)
{
    BridgeV4l bridge {"/dev/null"};

    ASSERT_FALSE (bridge.isConnected());
    ASSERT_THROW (bridge.getDeviceHandle(), Disconnected);
    ASSERT_ANY_THROW (bridge.startCapture());

    ASSERT_ANY_THROW (bridge.openConnection());

    ASSERT_FALSE (bridge.isConnected());
    ASSERT_THROW (bridge.getDeviceHandle(), Disconnected);
    ASSERT_ANY_THROW (bridge.startCapture());

    ASSERT_NO_THROW (bridge.closeConnection());

    ASSERT_FALSE (bridge.isConnected());
    ASSERT_THROW (bridge.getDeviceHandle(), Disconnected);
    ASSERT_ANY_THROW (bridge.startCapture());

    auto infoVector = bridge.getBridgeInfo();
    auto info = decltype (infoVector) ::toStdMap (infoVector);
    ASSERT_EQ (1u, info.count ("BRIDGE_TYPE"));
    ASSERT_EQ (royale::String {"V4L"}, info.at ("BRIDGE_TYPE"));
}
