/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "RegisterView.hpp"
#include "DisplaySupport.hpp"
#include <QKeyEvent>
#include <QPainter>

RegisterView::RegisterView (QWidget *parent) : PMDView (parent)
{
    setupUi (this);

    QObject::connect (leRegisterRequest, SIGNAL (returnPressed (void)), this, SLOT (registerRequestSent (void)));

    leRegisterRequest->setToolTip (QString ("Read register : r registeraddress\n") +
                                   QString ("Write register : w registeraddress value"));
    leRegisterRequest->setFocusPolicy (Qt::StrongFocus);
}

void RegisterView::registerRequestSent()
{
    if (leRegisterRequest->text().length() < 1)
    {
        return;
    }

    auto request = leRegisterRequest->text();
    leRegisterRequest->clear();

    auto requestList = request.trimmed().toLower().split (' ');

    if (requestList.at (0) == "r")
    {
        if (requestList.size() < 2)
        {
            lbRegisterResult->setText ("Not enough arguments. Use 'r registeraddress'");
            return;
        }

        auto registerStr = requestList.at (1);

        emit readRegister (registerStr);
    }
    else if (requestList.at (0) == "w")
    {
        if (requestList.size() < 3)
        {
            lbRegisterResult->setText ("Not enough arguments. Use 'w registeraddress value'");
            return;
        }

        auto registerStr = requestList.at (1);

        bool ok;
        auto value = requestList.at (2).toUShort (&ok, 0);

        emit writeRegister (registerStr, value);
    }
    else
    {
        lbRegisterResult->setText ("Allowed are : 'r registeraddress' and 'w registeraddress value'");
        return;
    }
}

void RegisterView::registerReadReturn (QString registerStr, uint16_t result)
{
    QString resStr = registerStr + " : 0x" + QString::number (result, 16).toUpper();
    lbRegisterResult->setText (resStr);
}

void RegisterView::registerWriteReturn (bool result)
{
    if (result)
    {
        lbRegisterResult->setText ("OK");
    }
    else
    {
        lbRegisterResult->setText ("Error writing register");
    }
}
