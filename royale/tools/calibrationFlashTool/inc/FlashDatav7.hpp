/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <FlashDataBase.hpp>

#include <ui_FlashDatav7.h>

/*
 * This class is used for polar based storages with header version 7.
 * It doesn't work for modules that use the SPI flash firmware!
 */

class FlashDataV7 :
    public FlashDataBase,
    public Ui::FlashDataV7
{
    Q_OBJECT

public:

    FlashDataV7 (std::unique_ptr<royale::ICameraDevice> cameraDevice, QWidget *parent = 0);
    ~FlashDataV7();

    void writeFlashData() override;
    void readFlashData() override;

private:

    void writeHeaderAndData();

};
