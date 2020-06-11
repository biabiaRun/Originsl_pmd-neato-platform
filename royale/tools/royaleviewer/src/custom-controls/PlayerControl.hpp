#pragma once

#include <cstdint>
#include <QWidget>
#include "ui_PlayerControl.h"

class PlayerControl : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerControl (QWidget *parent = 0);

    void reset();
    void updateSlider (unsigned currentFrame);
    void setFrameCount (unsigned frames);
    void setUpRange (int firstFrame, int lastFrame);

public slots:
    void playStopCallback();
    void showRangeBox (bool enabled);

signals:
    void rewind();
    void rewindWhenRange();
    void forward();
    void forwardWhenRange();
    void playStop (bool play);
    void seekTo (int frame);
    void seekToWhenRange (int frame);
    void setLabelText (QString text);
    void rangePlay (int firstFrame, int lastFrame);

private slots:
    void rewindCallback();
    void forwardCallback();
    void rangeStartCallback (int firstFrame);
    void rangeEndCallback (int lastFrame);
    void seekToCallback (int frame);

private:
    Ui::PlayerControl ui;
    bool m_isPlaying;

    uint32_t m_frameCount;
    uint32_t m_currentFrame;
    int m_rangeStart;
    int m_rangeEnd;
    bool m_rangeFlag;
};
