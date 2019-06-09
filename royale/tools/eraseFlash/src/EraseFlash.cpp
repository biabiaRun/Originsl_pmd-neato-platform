/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <EraseFlash.hpp>
#if defined(ROYALE_BRIDGE_ENCLUSTRA)
#include <EraseEnclustra.hpp>
#endif
#if defined(ROYALE_BRIDGE_UVC)
#include <EraseUvc.hpp>
#endif

#include <common/MakeUnique.hpp>

#include <G8PIDs.hpp>

#include <memory>
#include <stdint.h>
#include <vector>

using namespace royale::common;
using namespace royale::config;
using namespace royale::factory;
using namespace eraseflash;

EraseFlash::EraseFlash()
{
    setupUi (this);
}

EraseFlash::~EraseFlash()
{
}

void EraseFlash::on_pbOpenCamera_clicked()
{
    if (pbOpenCamera->text() == "Open Camera")
    {
        royale::factory::BridgeController bridgeController (getUsbProbeDataPlusG8 (true, true),
                getProcessingParameterMapFactoryRoyale());

        try
        {
            auto devices = bridgeController.probeDevices();

            if (devices.empty())
            {
                QMessageBox::warning (this, this->windowTitle(), "No suitable devices found!");
                return;
            }

            if (devices.size() > 1)
            {
                QMessageBox::warning (this, this->windowTitle(), "Please make sure that only one module is connected!");
                return;
            }

            auto &builder = *devices.front();

            IBridgeFactory &tmpFactory = builder.getBridgeFactory();

            std::vector <std::unique_ptr<IEraseToolFactory> > factories;
#if defined(ROYALE_BRIDGE_ENCLUSTRA)
            factories.push_back (makeUnique<EraseToolEnclustraFactory> ());
#endif
#if defined(ROYALE_BRIDGE_UVC)
            factories.push_back (makeUnique<EraseToolArcticFactory> ());
#endif

            for (auto &factory : factories)
            {
                try
                {
                    m_eraseTool = factory->createTool (tmpFactory);
                    break;
                }
                catch (...)
                {
                }
            }

            if (!m_eraseTool)
            {
                QString supported;
                for (auto &factory : factories)
                {
                    supported += factory->getName() + ", ";
                }

                QMessageBox::warning (this, this->windowTitle(), "This currently only supports the following types of modules: " + supported);
                return;
            }
        }
        catch (...)
        {
            QMessageBox::warning (this, this->windowTitle(), "Cannot create CameraModule!");
            return;
        }

        flashGroup->setEnabled (true);
        pbOpenCamera->setText ("Close Camera");
    }
    else
    {
        flashGroup->setEnabled (false);
        pbOpenCamera->setText ("Open Camera");

        m_eraseTool = nullptr;
    }
}

void EraseFlash::on_pbEraseFirmware_clicked()
{
    bool ret = true;
    QApplication::setOverrideCursor (Qt::WaitCursor);
    if (m_eraseTool)
    {
        ret = m_eraseTool->eraseFlash();
    }

    if (!ret)
    {
        QMessageBox::warning (this, this->windowTitle(), "Problem erasing firmware!");
    }
    else
    {
        QMessageBox::information (this, this->windowTitle(), "Successfully erased firmware!");
    }

    QApplication::restoreOverrideCursor();
}
