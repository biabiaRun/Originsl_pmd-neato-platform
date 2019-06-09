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

#include <ui_FlashDatav100.h>

/*
 * This class is used for pico flexx based storage
 */

class FlashDataV100 :
    public FlashDataBase,
    public Ui::FlashDataV100
{
    Q_OBJECT

public:

    FlashDataV100 (std::unique_ptr<royale::ICameraDevice> cameraDevice, QWidget *parent = 0);
    ~FlashDataV100();

    void writeFlashData() override;
    void readFlashData() override;

};
