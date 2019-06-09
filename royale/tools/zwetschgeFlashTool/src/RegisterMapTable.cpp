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

RegisterMapTable::RegisterMapTable()
{
    setupUi (this);

    tblRegisterMap->horizontalHeader()->setSectionResizeMode (QHeaderView::Stretch);
    tblRegisterMap->setRowCount (0);
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
