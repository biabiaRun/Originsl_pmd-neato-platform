/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <RegisterMapTable.hpp>

#include <qclipboard.h>
#include <QMenu>

RegisterMapTable::RegisterMapTable()
{
    setupUi (this);

    tblRegisterMap->horizontalHeader()->setSectionResizeMode (QHeaderView::Stretch);
    tblRegisterMap->setRowCount (0);

    this->setContextMenuPolicy (Qt::CustomContextMenu);
    connect (this, SIGNAL (customContextMenuRequested (const QPoint &)),
             this, SLOT (showContextMenu (const QPoint &)));
}

RegisterMapTable::~RegisterMapTable()
{
}

void RegisterMapTable::addEntry (const royale::imager::TimedRegisterListEntry &entry)
{
    auto rowCount = tblRegisterMap->rowCount();
    tblRegisterMap->insertRow (rowCount);
    tblRegisterMap->setItem (rowCount, 0,
                             new QTableWidgetItem (QString ("0x%1").arg (entry.address, 4, 16, QChar ('0'))));
    tblRegisterMap->setItem (rowCount, 1,
                             new QTableWidgetItem (QString ("0x%1").arg (entry.value, 4, 16, QChar ('0'))));
    tblRegisterMap->setItem (rowCount, 2,
                             new QTableWidgetItem (QString::number (entry.sleepTime)));

}

void RegisterMapTable::addRegisterMapList (const royale::imager::TimedRegisterList &reglist)
{
    tblRegisterMap->clearContents();
    tblRegisterMap->setRowCount (0);
    for (auto curEntry : reglist)
    {
        addEntry (curEntry);
    }
    tblRegisterMap->resizeRowsToContents();
}

void RegisterMapTable::showContextMenu (const QPoint &pos)
{
    QMenu contextMenu (tr ("Context menu"), this);

    QAction copyAction ("Copy to clipboard", this);
    connect (&copyAction, SIGNAL (triggered()), this, SLOT (copyToClipboard()));
    contextMenu.addAction (&copyAction);

    contextMenu.exec (mapToGlobal (pos));
}

void RegisterMapTable::copyToClipboard()
{
    QClipboard *clip = QApplication::clipboard();
    QString tableContent = "";

    auto rowCount = tblRegisterMap->rowCount();
    for (auto i = 0; i < rowCount; ++i)
    {
        auto addressItem = tblRegisterMap->item (i, 0);
        auto valueItem = tblRegisterMap->item (i, 1);
        auto sleepItem = tblRegisterMap->item (i, 2);

        tableContent += addressItem->text() + ";" +
                        valueItem->text() + ";" +
                        sleepItem->text() + "\n";
    }

    clip->setText (tableContent);
}
