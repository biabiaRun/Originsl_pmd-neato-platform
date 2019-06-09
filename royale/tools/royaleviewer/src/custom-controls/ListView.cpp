/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "ListView.hpp"
#include "DisplaySupport.hpp"
#include <QScroller>

ListView::ListView (QWidget *parent) : PMDView (parent)
{
    ui.setupUi (this);
    layout()->setSizeConstraint (QLayout::SetMaximumSize);
    ui.listWidget->setStyleSheet ("background-color:transparent");
    ui.listWidget->setProperty ("separator", true);
    QObject::connect (ui.listWidget, SIGNAL (itemClicked (QListWidgetItem *)), this, SLOT (itemSelected (QListWidgetItem *)));

    ui.listWidget->setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);
    QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea *> (ui.listWidget);
    QScroller::grabGesture (area->viewport(), QScroller::TouchGesture);
}

void ListView::addItem (const QString &name)
{
    QListWidgetItem *item = new QListWidgetItem (name);
    ui.listWidget->addItem (item);
    item->setTextColor (QColor (0, 0, 0));
    item->setSizeHint (QSize (item->sizeHint().width(), static_cast<int> (DisplaySupport::sharedInstance()->pointsToPixels (44))));
}

void ListView::itemSelected (QListWidgetItem *item)
{
    emit itemSelected (item->text());
}

void ListView::clearItems()
{
    ui.listWidget->clear();
}

void ListView::setSelectedItem (int index)
{
    if (index >= ui.listWidget->count())
    {
        return;
    }
    ui.listWidget->blockSignals (true);
    ui.listWidget->item (index)->setSelected (true);
    ui.listWidget->blockSignals (false);
}

void ListView::setSelectedItem (const QString &itemName)
{
    ui.listWidget->blockSignals (true);
    for (int i = 0; i < ui.listWidget->count(); i++)
    {
        if (ui.listWidget->item (i)->text() == itemName)
        {
            ui.listWidget->item (i)->setSelected (true);
            break;
        }
    }
    ui.listWidget->blockSignals (false);
}
