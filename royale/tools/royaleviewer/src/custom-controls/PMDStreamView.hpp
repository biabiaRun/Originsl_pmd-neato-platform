/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <QComboBox>
#include <royale/StreamId.hpp>
#include <royale/Vector.hpp>

#include "PMDView.hpp"

class PMDStreamView : public PMDView
{
    Q_OBJECT

public:
    explicit PMDStreamView (QWidget *parent = 0);
    explicit PMDStreamView (QWidget *parent, int rows, int columns);

    royale::StreamId getCurrentStreamId();

    void updateStreams (royale::Vector<royale::StreamId> streams);

public slots:
    void setCurrentStreamId (royale::StreamId streamId);

signals:

    void streamIdChanged (royale::StreamId streamId);

private slots :

    void comboIndexChanged (int index);

private:

    royale::StreamId m_streamId;
    QComboBox *m_streamsBox;
    QWidget *m_streamIdWidget;

};
