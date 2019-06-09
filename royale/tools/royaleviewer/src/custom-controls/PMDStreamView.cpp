#include <QLabel>
#include <QHBoxLayout>
#include "PMDStreamView.hpp"

PMDStreamView::PMDStreamView (QWidget *parent, int rows, int columns)
    : PMDView (parent, rows, columns),
      m_streamId (0)
{
    QLabel *streamLabel = new QLabel ("StreamID : ");
    QHBoxLayout *l = new QHBoxLayout();
    m_streamsBox = new QComboBox();
    m_streamsBox->setStyleSheet ("background-color: rgb(52, 139, 187);");

    m_streamIdWidget = new QWidget();
    m_streamIdWidget->setLayout (l);

    l->addWidget (streamLabel);
    l->addWidget (m_streamsBox);

    layout()->addWidget (m_streamIdWidget);

    QObject::connect (m_streamsBox, SIGNAL (currentIndexChanged (int)), this, SLOT (comboIndexChanged (int)));
}

PMDStreamView::PMDStreamView (QWidget *parent)
    : PMDView (parent),
      m_streamId (0),
      m_streamsBox (nullptr),
      m_streamIdWidget (nullptr)
{
}

royale::StreamId PMDStreamView::getCurrentStreamId()
{
    return m_streamId;
}

void PMDStreamView::setCurrentStreamId (royale::StreamId streamId)
{
    m_streamId = streamId;
}

void PMDStreamView::updateStreams (royale::Vector<royale::StreamId> streams)
{
    m_streamsBox->clear();
    for (auto curStream : streams)
    {
        m_streamsBox->addItem (QString::number (curStream), QVariant (curStream));
    }

    if (streams.size() <= 1)
    {
        m_streamIdWidget->hide();
    }
    else
    {
        m_streamIdWidget->show();
    }
}

void PMDStreamView::comboIndexChanged (int index)
{
    m_streamId = static_cast<royale::StreamId> (m_streamsBox->itemData (m_streamsBox->currentIndex()).toUInt());
    emit streamIdChanged (m_streamId);
}
