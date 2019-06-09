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

#include <factory/ICameraCoreBuilder.hpp>
#include <factory/CameraCoreBuilderImpl.hpp>
#include <memory>

namespace royale
{
    namespace factory
    {
        // Factory for Mira-based cameras
        class CameraCoreBuilderMira : public CameraCoreBuilderImpl
        {
        public:
            CameraCoreBuilderMira();
            std::unique_ptr<royale::device::CameraCore> createCameraCore() override;
        };
    }
}
