/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <EraseFlash.hpp>

namespace eraseflash
{
    class EraseToolEnclustraFactory : public IEraseToolFactory
    {
    public:
        ~EraseToolEnclustraFactory() override = default;
        QString getName() override;
        std::unique_ptr<EraseTool> createTool (royale::factory::IBridgeFactory &bridgeFactory) override;
    };
}
