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

#include <QtWidgets>
#include <device/CameraCore.hpp>
#include <factory/BridgeController.hpp>
#include <factory/ICameraCoreBuilder.hpp>

#include <royale/Pair.hpp>
#include <royale/Vector.hpp>
#include <imager/Imager.hpp>

#include <collector/IFrameCaptureListener.hpp>
#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/Exception.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <storage/IBridgeWithPagedFlash.hpp>
#include <usb/factory/IBridgeFactory.hpp>

#include <ui_mainwindow.h>
namespace eraseflash
{
    /**
     * These handle the module-specific details of how to erase.
     *
     * They're expected to be created using RAII, with the constructor throwing if it can't support the
     * module.
     */
    class EraseTool
    {
    public:
        virtual ~EraseTool() = default;
        /** Returns true if it successfully erases the data. */
        virtual bool eraseFlash() = 0;
    };

    class IEraseToolFactory
    {
    public:
        virtual ~IEraseToolFactory() = default;
        /** What type of modules this will support */
        virtual QString getName() = 0;
        /** Creates an EraseTool, or throws an exception */
        virtual std::unique_ptr<EraseTool> createTool (royale::factory::IBridgeFactory &bridgeFactory) = 0;
    };
}

class EraseFlash :
    public QMainWindow,
    public Ui::EraseFlashToolWindow
{
    Q_OBJECT

public:
    EraseFlash();
    ~EraseFlash();

protected slots :

    void on_pbOpenCamera_clicked();
    void on_pbEraseFirmware_clicked();

private:
    std::unique_ptr<eraseflash::EraseTool> m_eraseTool;
};
