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
#include <map>
#include <royale.hpp>
#include "ui_ParameterView.h"
#include "PMDStreamView.hpp"

class ParameterView : public PMDStreamView
{
    Q_OBJECT

public:
    explicit ParameterView (QWidget *parent = 0);

    void reset();
    void setCurrentParameters (royale::ProcessingParameterVector parameters);

    void setValue (royale::Pair<royale::ProcessingFlag, royale::Variant> parameter);

public slots:

    void parameterChanged();

signals:

    void changePipelineParameter (royale::Pair<royale::ProcessingFlag, royale::Variant> parameter);

protected:
    Ui::ParameterView ui;

    std::map<QWidget *, royale::Pair<royale::ProcessingFlag, royale::Variant>> m_flagMapping;
};
