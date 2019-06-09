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

#include <QWidget>
#include <stdint.h>

#include <config/IImagerExternalConfig.hpp>

#include <ui_RegisterMapTable.h>


class RegisterMapTable :
    public QWidget,
    public Ui::RegisterMapTable
{
    Q_OBJECT

public:
    explicit RegisterMapTable();
    ~RegisterMapTable();

    void addEntry (const royale::imager::TimedRegisterListEntry &entry);
    void addRegisterMapList (const royale::imager::TimedRegisterList &reglist);

};
