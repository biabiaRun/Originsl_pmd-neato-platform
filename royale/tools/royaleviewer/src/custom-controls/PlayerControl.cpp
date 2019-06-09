#include "PlayerControl.hpp"
#include "DisplaySupport.hpp"

PlayerControl::PlayerControl (QWidget *parent)
    : QWidget (parent),
      m_isPlaying (true),
      m_frameCount (0),
      m_currentFrame (0)
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
    QObject::connect (ui.seekSlider, SIGNAL (valueChanged (int)), this, SIGNAL (seekTo (int)));

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

void PlayerControl::rewindCallback()
{
    emit rewind();
}

void PlayerControl::forwardCallback()
{
    emit forward();
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
