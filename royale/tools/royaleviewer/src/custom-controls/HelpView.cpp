/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "HelpView.hpp"
#include "DisplaySupport.hpp"
#include <QDir>
#include <QSpacerItem>
#include <QDesktopServices>
#include <QUrl>

HelpView::HelpView (QWidget *parent) : PMDView (parent),
    m_aboutText ("\n")
{
    ui.setupUi (this);
    layout()->setSizeConstraint (QLayout::SetMaximumSize);

    setLogTabSpecial();
    setInfoTabSpecial();
    setHelpTabSpecial();
    setAboutTabSpecial();
    ui.tabWidget->setCurrentIndex (0);
    connect (ui.tabWidget, SIGNAL (tabBarClicked (int)), this, SIGNAL (tabBarClicked (int)));
}

void HelpView::setLogTabSpecial()
{
    ui.clearButton->resize (1.0f, 0.7f);
    QObject::connect (ui.clearButton, SIGNAL (clicked()), this, SLOT (clearButtonClicked()));
}

void HelpView::setInfoTabSpecial()
{
    ui.refreshButton->resize (1.0f, 0.7f);
    QObject::connect (ui.refreshButton, SIGNAL (clicked()), this, SIGNAL (refreshButtonClicked()));
}

void HelpView::setHelpTabSpecial()
{
#if defined(Q_OS_ANDROID)
    helpLabel = new QLabel;
    helpLabel->setAlignment (Qt::AlignVCenter);
    helpLabel->setStyleSheet ("color: white;");
    helpLabel->setWordWrap (true);
    ui.helpLayout->addWidget (helpLabel);
#else
    QDir path = QCoreApplication::applicationDirPath();

    // Change from the bin folder to the root
    path.cdUp();
#if defined (Q_OS_MAC)
    // On MacOS the executable is in bin\royaleviewer.app\Contents\MacOS
    for (int j = 0; j < 3; ++j)
    {
        path.cdUp();
    }
#endif

    QString installationFolder = path.absolutePath();

    // Use QUrl to create an encoded version of the path as some characters (e.g. spaces) are not valid for links
    QUrl decodedPath (installationFolder);

    QLabel *guideLabel = createFileLabel (decodedPath.toEncoded() + "/RoyaleViewer.pdf style=color:white", "Open User's Guide");
    QLabel *folderLabel = createFileLabel (decodedPath.toEncoded() + " style=color:white", "Open Installation Folder");

    QSpacerItem *verticalSpacerTop;
    QSpacerItem *verticalSpacerBottom;
    verticalSpacerTop = new QSpacerItem (20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalSpacerBottom = new QSpacerItem (20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    ui.helpLayout->addItem (verticalSpacerTop);
    ui.helpLayout->addWidget (guideLabel);
    ui.helpLayout->addWidget (folderLabel);
    ui.helpLayout->addItem (verticalSpacerBottom);
#endif
}

void HelpView::setAboutTabSpecial()
{
    int width = static_cast<int> (DisplaySupport::sharedInstance()->pointsToPixels (65));
    QString logoText = "<img src=:/logo width=" + QString::number (width) + ">";
    ui.logoLabel->setText (logoText);

    QString text = "<a href=file:/License style=color:white>Show Licenses of Royale</a>";
    ui.licenseLabel->setText (tr (text.toLatin1()));
    connect (ui.licenseLabel, SIGNAL (linkActivated (QString)), this, SLOT (openUrl (QString)));

    QString pmdText;
    QString ifxText;
#if defined(Q_OS_ANDROID)
    width = static_cast<int> (DisplaySupport::sharedInstance()->pointsToPixels (100));
    pmdText = "<a href = http://www.pmdtec.com><img src=:/pmdLogo.png width=" + QString::number (width) + "></a>";
    ifxText = "<a href = http://www.infineon.com><img src=:/ifxLogo.png width=" + QString::number (width) + "></a>";
#else
    pmdText = "<a href=http://www.pmdtec.com><img src=:/pmdLogo.png></a>";
    ifxText = "<a href=http://www.infineon.com><img src=:/ifxLogo.png></a>";
#endif

    ui.pmdLabel->setText (tr (pmdText.toLatin1()));
    connect (ui.pmdLabel, SIGNAL (linkActivated (QString)), this, SLOT (openUrl (QString)));

    ui.ifxLabel->setText (tr (ifxText.toLatin1()));
    connect (ui.ifxLabel, SIGNAL (linkActivated (QString)), this, SLOT (openUrl (QString)));
}

void HelpView::addLogMessage (const QString &message)
{
    ui.logTextEdit->append (message);
    ui.logTextEdit->moveCursor (QTextCursor::End, QTextCursor::MoveAnchor);
    emit newLogMessage();
}

void HelpView::addInfoMessage (const QString &message)
{
    ui.infoTextEdit->append (message);
    ui.infoTextEdit->moveCursor (QTextCursor::Start, QTextCursor::MoveAnchor);
}

void HelpView::addHelpMessage (const QString &message)
{
    helpLabel->setText (message);
}

void HelpView::addAboutMessage (const QString &message)
{
    m_aboutText += message + "\n";
    ui.aboutLabel->setText (m_aboutText);
}

void HelpView::clearButtonClicked()
{
    ui.logTextEdit->setText ("");
}

void HelpView::clearInfoMessage()
{
    ui.infoTextEdit->setText ("");
}

void HelpView::setLogTabChecked()
{
    ui.tabWidget->setCurrentIndex (0);
}

int HelpView::getTabWidgetCurrentIndex()
{
    return ui.tabWidget->currentIndex();
}

QLabel *HelpView::createFileLabel (QString fileName, QString shownText)
{
    QString text = "<a href=file:" + fileName + ">" + shownText + "</a>";

    QLabel *label = new QLabel;
    label->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Fixed);
    label->setAlignment (Qt::AlignCenter);
    label->setText (tr (text.toLatin1()));
    connect (label, SIGNAL (linkActivated (QString)), this, SLOT (openUrl (QString)));
    return label;
}

void HelpView::openUrl (QString url)
{
    if (url.right (7) == "License")
    {
        emit licensePanelToggled();
    }
    else
    {
        QDesktopServices::openUrl (QUrl (url));
    }
}

void HelpView::switchHelpTab (int i)
{
    int curIdx = (ui.tabWidget->currentIndex() + 4 + i) % 4;
    ui.tabWidget->setCurrentIndex (curIdx);

    emit tabBarClicked (curIdx);
}