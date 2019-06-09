/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <stdint.h>
#include <QWidget>
#include "ui_RegisterView.h"
#include "PMDView.hpp"

class RegisterView : public PMDView,
    public Ui::RegisterView
{
    Q_OBJECT
public:
    explicit RegisterView (QWidget *parent = 0);

public slots:
    void registerRequestSent();
    void registerReadReturn (QString registerStr, uint16_t result);
    void registerWriteReturn (bool result);

signals:
    void readRegister (QString registerStr);
    void writeRegister (QString registerStr, uint16_t value);
};
