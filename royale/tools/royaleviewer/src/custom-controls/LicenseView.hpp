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
#include "ui_LicenseView.h"
#include "PMDView.hpp"

class LicenseView : public PMDView
{
    Q_OBJECT

public:
    explicit LicenseView (QWidget *parent = 0);
    void switchLicenseTab (int i);

protected:
    Ui::LicenseView ui;

private:
    void tryToAddLicense (const QString &name, const QString &displayName);
    int m_count;
};
