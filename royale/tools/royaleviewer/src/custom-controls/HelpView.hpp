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
#include "ui_HelpView.h"
#include "PMDButton.hpp"
#include "PMDView.hpp"

class HelpView : public PMDView
{
    Q_OBJECT

public:
    explicit HelpView (QWidget *parent = 0);

    void addInfoMessage (const QString &message);
    void addHelpMessage (const QString &message);
    void addAboutMessage (const QString &message);
    void clearInfoMessage();
    void setLogTabChecked();
    int getTabWidgetCurrentIndex();
    void switchHelpTab (int i);

signals:
    void licensePanelToggled();
    void newLogMessage();
    void tabBarClicked (int index);
    void refreshButtonClicked();

public slots:
    void addLogMessage (const QString &message);
    void clearButtonClicked();

protected:
    Ui::HelpView ui;

private slots:
    void openUrl (QString url);

private:
    void setLogTabSpecial();
    void setInfoTabSpecial();
    void setHelpTabSpecial();
    void setAboutTabSpecial();

    QLabel  *helpLabel;
    QLabel  *createFileLabel (QString fileName, QString shownText);
    QString m_aboutText;
};
