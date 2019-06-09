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

#include <QApplication>
#include <QMessageBox>
#include <QWidget>
#include <royale.hpp>

class FlashDataBase :
    public QWidget
{
    Q_OBJECT

public:
    explicit FlashDataBase (std::unique_ptr<royale::ICameraDevice> cameraDevice, QWidget *parent = 0) :
        QWidget (parent),
        m_cameraDevice (std::move (cameraDevice))
    {
        m_statusBox.hide();
        m_statusBox.setStandardButtons (0);
        m_statusBox.setText ("Flash access in progress ...");
        m_statusBox.setInformativeText ("Please do not disconnect the camera!");
        m_statusBox.setModal (true);
    }

    virtual ~FlashDataBase() = default;

    royale::Vector<uint8_t> &getCalibrationData()
    {
        return m_calibrationData;
    }

    void setCalibrationData (const royale::Vector<uint8_t> &data)
    {
        m_calibrationData = data;
    }

    void showStatusBox()
    {
        // We currently only show an informative text. In the future
        // we could also display a progress bar (no requirement for this today)

        QApplication::setOverrideCursor (Qt::WaitCursor);

        m_statusBox.show();
        QCoreApplication::processEvents();
    }

    void hideStatusBox()
    {
        m_statusBox.hide();
        QApplication::restoreOverrideCursor();
    }

    void writeData()
    {
        showStatusBox();
        writeFlashData();
        hideStatusBox();
    }

    void readData()
    {
        showStatusBox();
        readFlashData();
        hideStatusBox();
    }

protected:

    virtual void writeFlashData() = 0;
    virtual void readFlashData() = 0;

    QMessageBox m_statusBox;
    std::unique_ptr<royale::ICameraDevice> m_cameraDevice;
    royale::Vector<uint8_t> m_calibrationData;
};
