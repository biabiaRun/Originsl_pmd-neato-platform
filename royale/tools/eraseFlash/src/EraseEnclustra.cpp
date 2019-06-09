/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <EraseEnclustra.hpp>
#include <storage/IBridgeWithPagedFlash.hpp>
#include <usb/factory/IBridgeFactory.hpp>

#include <common/MakeUnique.hpp>

using namespace royale::common;
using namespace royale::factory;
using namespace royale::storage;
using namespace eraseflash;

namespace
{
    class EnclustraTool : public EraseTool
    {
    public:
        explicit EnclustraTool (IBridgeFactory &factory)
        {
            if (factory.supports<IBridgeWithPagedFlash>())
            {
                m_bridgeWithPagedFlash = factory.create<IBridgeWithPagedFlash>();
                return;
            }
            throw CouldNotOpen();
        }

        ~EnclustraTool() = default;

        bool eraseFlash() override
        {
            try
            {
                // We only have to erase the first sector to invalidate the firmware
                // and get to the Bootloader
                m_bridgeWithPagedFlash->eraseSectors (0, 1);
            }
            catch (...)
            {
                return false;
            }
            return true;
        }

    private:
        std::shared_ptr<royale::storage::IBridgeWithPagedFlash> m_bridgeWithPagedFlash;
    };
}

QString EraseToolEnclustraFactory::getName()
{
    return "Enclustra";
}

std::unique_ptr<EraseTool> EraseToolEnclustraFactory::createTool (IBridgeFactory &bridgeFactory)
{
    return makeUnique<EnclustraTool> (bridgeFactory);
}
