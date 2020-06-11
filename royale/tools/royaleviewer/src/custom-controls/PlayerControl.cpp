#include "PlayerControl.hpp"
#include "DisplaySupport.hpp"

PlayerControl::PlayerControl (QWidget *parent)
    : QWidget (parent),
      m_isPlaying (true),
      m_frameCount (0),
      m_currentFrame (0),
      m_rangeStart (0),
      m_rangeEnd (0),
      m_rangeFlag (false)
{
    ui.setupUi (this);
    ui.rewindButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-rewind-default"));
    ui.rewindButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-rewind-pressed"));
    ui.playStopButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-stop-default"));
    ui.playStopButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-stop-pressed"));
    ui.forwardButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-forward-default"));
    ui.forwardButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-forward-pressed"));
    QObject::connect (ui.rewindButton,   SIGNAL (clicked()), this, SLOT (rewindCallback()));
    QObject::connect (ui.forwardButton,  SIGNAL (clicked()), this, SLOT (forwardCallback()));
    QObject::connect (ui.playStopButton, SIGNAL (released()), this, SLOT (playStopCallback()));
    QObject::connect (ui.seekSlider, SIGNAL (valueChanged (int)), this, SLOT (seekToCallback (int)));
    QObject::connect (ui.rangeStart, SIGNAL (valueChanged (int)), this, SLOT (rangeStartCallback (int)));
    QObject::connect (ui.rangeEnd, SIGNAL (valueChanged (int)), this, SLOT (rangeEndCallback (int)));

    QObject::connect (this, SIGNAL (setLabelText (QString)), ui.frameCounter, SLOT (setText (QString)));
}

void PlayerControl::reset()
{
    m_isPlaying = true;
    ui.playStopButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-stop-default"));
    ui.playStopButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-stop-pressed"));
    ui.seekSlider->blockSignals (true);
    ui.seekSlider->setMinimum (0);
    ui.seekSlider->setMaximum (1);
    ui.seekSlider->setValue (0);
    ui.seekSlider->blockSignals (false);
}

void PlayerControl::seekToCallback (int frame)
{
    if (m_rangeFlag)
    {
        emit seekToWhenRange (frame);
    }
    else
    {
        emit seekTo (frame);
    }
}

void PlayerControl::rewindCallback()
{
    if (m_rangeFlag)
    {
        emit rewindWhenRange();
    }
    else
    {
        emit rewind();
    }
}

void PlayerControl::showRangeBox (bool enabled)
{
    ui.rangeStart->setVisible (enabled);
    ui.rangeEnd->setVisible (enabled);
    m_rangeFlag = enabled;
}

void PlayerControl::rangeStartCallback (int firstFrame)
{
    m_rangeStart = firstFrame - 1;
    emit rangePlay (m_rangeStart, m_rangeEnd);
}

void PlayerControl::rangeEndCallback (int lastFrame)
{
    m_rangeEnd = lastFrame;
    emit rangePlay (m_rangeStart, m_rangeEnd);
}

void PlayerControl::setUpRange (int firstFrame, int lastFrame)
{
    m_rangeStart = firstFrame;
    m_rangeEnd = lastFrame;

    ui.rangeStart->setRange (firstFrame, lastFrame);
    ui.rangeStart->setValue (firstFrame);

    ui.rangeEnd->setRange (firstFrame, lastFrame);
    ui.rangeEnd->setValue (lastFrame);

}

void PlayerControl::forwardCallback()
{
    if (m_rangeFlag)
    {
        emit forwardWhenRange();
    }
    else
    {
        emit forward();
    }
}

void PlayerControl::playStopCallback()
{
    m_isPlaying = !m_isPlaying;
    if (m_isPlaying)
    {
        ui.playStopButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-stop-default"));
        ui.playStopButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-stop-pressed"));
    }
    else
    {
        ui.playStopButton->setDefaultImage (DisplaySupport::sharedInstance()->asset ("/button-play-default"));
        ui.playStopButton->setPressedImage (DisplaySupport::sharedInstance()->asset ("/button-play-pressed"));
    }
    ui.playStopButton->repaint();
    emit playStop (m_isPlaying);
}

void PlayerControl::setFrameCount (unsigned frames)
{
    ui.seekSlider->blockSignals (true);
    ui.seekSlider->setMinimum (0);
    ui.seekSlider->setMaximum (frames - 1);
    m_frameCount = frames;
    ui.seekSlider->blockSignals (false);
}

void PlayerControl::updateSlider (unsigned currentFrame)
{
    ui.seekSlider->blockSignals (true);
    ui.seekSlider->setValue (currentFrame);
    m_currentFrame = currentFrame + 1;
    QString text = QString::number (m_currentFrame) + "/" + QString::number (m_frameCount);
    emit setLabelText (text);
    ui.seekSlider->blockSignals (false);
}
