/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <QWidget>
#include "ui_ListView.h"
#include "PMDView.hpp"

class ListView : public PMDView
{
    Q_OBJECT

public:
    explicit ListView (QWidget *parent = 0);
    void addItem (const QString &name);
    void setSelectedItem (int index);
    void setSelectedItem (const QString &itemName);
    void clearItems();

protected slots:
    void itemSelected (QListWidgetItem *item);

signals:
    void itemSelected (const QString &name);

protected:
    Ui::ListView ui;
};
