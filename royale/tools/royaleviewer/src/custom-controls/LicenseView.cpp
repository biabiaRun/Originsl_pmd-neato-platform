/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "LicenseView.hpp"
#include "DisplaySupport.hpp"
#include "PMDButton.hpp"
#include <QFile>
#include <QLabel>
#include <QPainter>
#include <QScrollArea>
#include <QTabBar>
#include <qtextbrowser.h>
#include <QTextStream>

LicenseView::LicenseView (QWidget *parent) : PMDView (parent),
    m_count (0)
{
    ui.setupUi (this);
    ui.tabWidget->setStyleSheet ("background-color: rgb(52, 139, 187);");
    layout()->setSizeConstraint (QLayout::SetMaximumSize);

    int tabHeight = static_cast<int> (DisplaySupport::sharedInstance()->pointsToPixels (25));
    QString tabStyle = "QTabBar::tab {height: " + QString::number (tabHeight) + "px;}";
    ui.tabWidget->tabBar()->setStyleSheet (tabStyle);

    tryToAddLicense ("royale_license", "Royale");
    tryToAddLicense ("royaleviewer_license", "RoyaleViewer");
    tryToAddLicense ("used_licenses", "Used licenses");
}

void LicenseView::tryToAddLicense (const QString &name, const QString &displayName)
{
    QString filename = ":/" + name;

    QFile licenseFile (filename);
    if (licenseFile.open (QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream (&licenseFile);
        QString content = "";
        while (!stream.atEnd())
        {
            content += stream.readLine();
            content += "\n";
        }
        licenseFile.close();

        QTextBrowser *newLicenseLabel = new QTextBrowser();
        newLicenseLabel->setMaximumWidth (this->width());
        newLicenseLabel->setAlignment (Qt::AlignTop);
        newLicenseLabel->setStyleSheet ("color: white;");
        newLicenseLabel->setText (content);
        newLicenseLabel->setFocusPolicy (Qt::FocusPolicy::NoFocus);

        const QString name = "   " + displayName + "   ";
        ui.tabWidget->addTab (newLicenseLabel, name);
        newLicenseLabel->show();

        m_count++;
    }
}

void LicenseView::switchLicenseTab (int i)
{
    int curIdx = (ui.tabWidget->currentIndex() + m_count + i) % m_count;
    ui.tabWidget->setCurrentIndex (curIdx);
}
