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

#include <royale/ICameraDevice.hpp>
#include <factory/ICameraCoreBuilder.hpp>
#include <memory>

namespace royale
{
    class DeviceFactory
    {
    public:
        /**
        * Creates an ICameraDevice from pre-built components.
        */
        static std::unique_ptr<royale::ICameraDevice> createCameraDevice (
            royale::CameraAccessLevel accessLevel,
            royale::factory::ICameraCoreBuilder &coreBuilder);

    };
}
