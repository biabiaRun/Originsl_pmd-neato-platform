/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "DisplaySupport.hpp"
#include "OpenFileView.hpp"
#include <FileHelper.hpp>
#include <QDir>
#include <QStringList>
#include <QScroller>

OpenFileView::OpenFileView (QWidget *parent) : PMDView (parent)
{
    ui.setupUi (this);
    ui.loadButton->resize (1.0f, 0.7f);
    layout()->setSizeConstraint (QLayout::SetMaximumSize);
    ui.fileList->setStyleSheet ("background-color:transparent;");
    ui.fileList->setProperty ("separator", true);
    QObject::connect (ui.loadButton, SIGNAL (clicked()), this, SLOT (okClicked()));

    ui.fileList->setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);
    QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea *> (ui.fileList);
    QScroller::grabGesture (area->viewport(), QScroller::TouchGesture);
}

void OpenFileView::showEvent (QShowEvent *)
{
    ui.fileList->clear();
    std::string inputPath = royaleviewer::getOutputPath();
    if (inputPath.empty())
    {
        return;
    }
    QStringList nameFilter ("*.rrf");
    QDir directory (QString::fromStdString (inputPath));
    QStringList recordings = directory.entryList (nameFilter);
    if (!recordings.isEmpty ())
    {
        ui.fileList->addItems (recordings);
    }
    for (int i = 0; i < ui.fileList->count(); ++i)
    {
        QListWidgetItem *item = ui.fileList->item (i);
        item->setTextColor (QColor (0, 0, 0));
        item->setSizeHint (QSize (item->sizeHint().width(), static_cast<int> (DisplaySupport::sharedInstance()->pointsToPixels (44))));
    }
}

void OpenFileView::okClicked()
{
    if (!ui.fileList)
    {
        return;
    }
    if (ui.fileList->selectedItems().isEmpty())
    {
        return;
    }
    QString file = ui.fileList->currentItem()->text();
    std::string pathToFile = royaleviewer::getOutputPath() + "/" + file.toStdString();
    emit fileSelected (pathToFile);
}
