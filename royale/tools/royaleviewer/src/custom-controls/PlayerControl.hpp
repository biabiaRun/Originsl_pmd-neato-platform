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

public slots:
    void playStopCallback();

signals:
    void rewind();
    void forward();
    void playStop (bool play);
    void seekTo (int frame);
    void setLabelText (QString text);

private slots:
    void rewindCallback();
    void forwardCallback();

private:
    Ui::PlayerControl ui;
    bool m_isPlaying;

    uint32_t m_frameCount;
    uint32_t m_currentFrame;
};
