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
    tryToAddLicense ("cypress_license", "Cypress");
    tryToAddLicense ("gradle-wrapper_license", "Gradle-wrapper");
    tryToAddLicense ("jquery_license", "JQuery");
    tryToAddLicense ("kissfft_license", "KissFFT");
    tryToAddLicense ("lgpl-2.1", "LibUSB");
    tryToAddLicense ("packedarray_license", "Packed Array");
    tryToAddLicense ("sizzle_license", "Sizzle");
    tryToAddLicense ("gpl-3.0", "GPL 3.0");
    tryToAddLicense ("lgpl-3.0", "LGPL 3.0");
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

        QLabel *newLicenseLabel = new QLabel();
        //newLicenseLabel->setTextInteractionFlags (Qt::TextSelectableByMouse);
        newLicenseLabel->setMaximumWidth (this->width());
        newLicenseLabel->setAlignment (Qt::AlignTop);
        newLicenseLabel->setStyleSheet ("color: white;");
        newLicenseLabel->setText (content);
        newLicenseLabel->setWordWrap (true);
        newLicenseLabel->setFocusPolicy (Qt::FocusPolicy::NoFocus);

        QScrollArea *scrollArea = new QScrollArea();
        scrollArea->setAlignment (Qt::AlignHCenter);
        scrollArea->setWidget (newLicenseLabel);

        const QString name = "   " + displayName + "   ";
        ui.tabWidget->addTab (scrollArea, name);
        newLicenseLabel->show();

        m_count++;
    }
}

void LicenseView::switchLicenseTab (int i)
{
    int curIdx = (ui.tabWidget->currentIndex() + m_count + i) % m_count;
    ui.tabWidget->setCurrentIndex (curIdx);
}
