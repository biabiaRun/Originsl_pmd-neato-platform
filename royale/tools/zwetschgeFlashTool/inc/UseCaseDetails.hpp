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

#include <QDialog>

#include <ui_UseCaseDetails.h>

#include <RegisterMapTable.hpp>


class UseCaseDetails :
    public QDialog,
    public Ui::UseCaseDetailsDlg
{
    Q_OBJECT

public:
    explicit UseCaseDetails();
    ~UseCaseDetails();

    RegisterMapTable *m_registerMapTable;
};
