/****************************************************************************\
 * Copyright (C) 2016 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <rrftool.hpp>

#include <stdint.h>

RRFTool::RRFTool()
{
    setupUi (this);

    progressBar->setVisible (false);
}

RRFTool::~RRFTool()
{
}

void RRFTool::on_actionOpenFile_triggered()
{
    on_actionCloseFile_triggered();

    QString filename = QFileDialog::getOpenFileName (this, tr ("Open File"),
                       "",
                       tr ("Royale recordings (*.rrf)"));

    if (filename.isEmpty())
    {
        return;
    }

    if (royale_open_input_file (&m_fileReadHandle, filename.toLatin1().data()) != RRF_NO_ERROR)
    {
        QMessageBox::warning (this, this->windowTitle(), "Could not load the file!");
        m_fileReadHandle = 0;
        return;
    }

    fillInfoBox();
    actionSaveAs->setEnabled (true);
    actionCloseFile->setEnabled (true);
}

void RRFTool::on_actionSaveAs_triggered()
{
    QString filename = QFileDialog::getSaveFileName (this, tr ("Save File As"),
                       "",
                       tr ("Royale recordings (*.rrf)"));

    if (filename.isEmpty())
    {
        return;
    }

    QApplication::setOverrideCursor (Qt::WaitCursor);

    const struct royale_fileinformation_v3 *fileInfo = royale_get_fileinformation (m_fileReadHandle);

    if (!fileInfo)
    {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning (this, this->windowTitle(), "Couldn't retrieve file information!");
        return;
    }

    royale_rrf_handle outHandle;
    if (royale_open_output_file (&outHandle, filename.toLatin1().data(), fileInfo) != royale_rrf_api_error::RRF_NO_ERROR)
    {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning (this, this->windowTitle(), "Error opening output file!");
        return;
    }

    progressBar->setVisible (true);
    if (!saveFrames (outHandle))
    {
        QMessageBox::warning (this, this->windowTitle(), "Error outputting frames!");
    }
    progressBar->setVisible (false);

    if (royale_close_output_file (outHandle) != royale_rrf_api_error::RRF_NO_ERROR)
    {
        QMessageBox::warning (this, this->windowTitle(), "Error closing output file!");
    }

    QApplication::restoreOverrideCursor();
}

void RRFTool::on_actionExit_triggered()
{
    on_actionCloseFile_triggered();
    close();
}

void RRFTool::on_actionCloseFile_triggered()
{
    clearInfoBox();

    royale_close_input_file (m_fileReadHandle);
    m_fileReadHandle = 0;

    actionSaveAs->setEnabled (false);
    actionCloseFile->setEnabled (false);
}

void RRFTool::fillInfoBox()
{
    const struct royale_fileinformation_v3 *fileInfo = royale_get_fileinformation (m_fileReadHandle);

    if (!fileInfo)
    {
        return;
    }

    lblVersion->setText (QString ("RRFv") + QString::number (royale_get_version (m_fileReadHandle)));

    switch (fileInfo->platform)
    {
        case RRF_ROYALE_WINDOWS:
            lblPlatform->setText ("Windows");
            break;
        case RRF_ROYALE_LINUX:
            lblPlatform->setText ("Linux");
            break;
        case RRF_ROYALE_MAC:
            lblPlatform->setText ("Mac");
            break;
        case RRF_ROYALE_ANDROID:
            lblPlatform->setText ("Android");
            break;
        case RRF_ROYALE_UNDEFINED:
        default:
            lblPlatform->setText ("Undefined");
    }

    lblNumFrames->setText (QString::number (royale_get_num_frames (m_fileReadHandle)));
    QString royaleVersion = QString::number (fileInfo->royaleMajor) + "." +
                            QString::number (fileInfo->royaleMinor) + "." +
                            QString::number (fileInfo->royalePatch) + "." +
                            QString::number (fileInfo->royaleBuild);
    lblRoyaleVersion->setText (royaleVersion);

    lblCameraName->setText (QString::fromLatin1 (fileInfo->cameraName));
    lblImagerSerial->setText (QString::fromLatin1 (fileInfo->imagerSerial));
    lblImagerType->setText (QString::fromLatin1 (fileInfo->imagerType));
    lblPseudoDataType->setText (QString::fromLatin1 (fileInfo->pseudoDataInter));

    lblCalibSize->setText (QString::number (fileInfo->calibrationDataSize));

    fileInfoBox->setEnabled (true);
}

void RRFTool::clearInfoBox()
{
    lblVersion->clear();
    lblPlatform->clear();
    lblNumFrames->clear();
    lblRoyaleVersion->clear();
    lblCameraName->clear();
    lblImagerSerial->clear();
    lblImagerType->clear();
    lblPseudoDataType->clear();
    lblCalibSize->clear();

    fileInfoBox->setEnabled (false);
}

bool RRFTool::saveFrames (const royale_rrf_handle &outHandle)
{
    uint32_t numFrames = royale_get_num_frames (m_fileReadHandle);

    if (!numFrames)
    {
        return true;
    }

    progressBar->setMaximum (numFrames);

    for (uint32_t i = 0u; i < numFrames; ++i)
    {
        progressBar->setValue (i);
        if (royale_seek (m_fileReadHandle, i) != royale_rrf_api_error::RRF_NO_ERROR)
        {
            QApplication::restoreOverrideCursor();
            QMessageBox::warning (this, this->windowTitle(), "Error seeking to frame " + QString::number (i) + "!");
            return false;
        }

        struct royale_frame_v3 *curFrame;
        if (royale_get_frame (m_fileReadHandle, &curFrame) != royale_rrf_api_error::RRF_NO_ERROR)
        {
            QApplication::restoreOverrideCursor();
            QMessageBox::warning (this, this->windowTitle(), "Error reading frame " + QString::number (i) + "!");
            return false;
        }

        if (royale_output_data (outHandle, curFrame) != royale_rrf_api_error::RRF_NO_ERROR)
        {
            QApplication::restoreOverrideCursor();
            QMessageBox::warning (this, this->windowTitle(), "Error writing frame " + QString::number (i) + "!");
            return false;
        }

        if (royale_free_frame (&curFrame) != royale_rrf_api_error::RRF_NO_ERROR)
        {
            QApplication::restoreOverrideCursor();
            QMessageBox::warning (this, this->windowTitle(), "Error freeing frame " + QString::number (i) + "!");
            return false;
        }
    }

    return true;
}