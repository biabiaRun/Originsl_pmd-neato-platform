/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <raw2rrf.hpp>

#include <common/FileSystem.hpp>
#include <common/NtcTempAlgo.hpp>
#include <factory/ImagerFactory.hpp>

#include <imager/M2453/PseudoDataInterpreter_B11.hpp>

#include <modules/CommonProcessingParameters.hpp>
#include <record/CameraRecord.hpp>
#include <usecase/UseCaseArbitraryPhases.hpp>
#include <usecase/UseCaseEightPhase.hpp>
#include <usecase/UseCaseFourPhase.hpp>

#include <stdint.h>

using namespace royale;
using namespace royale::collector;
using namespace royale::common;
using namespace royale::config;
using namespace royale::imager;
using namespace royale::record;
using namespace royale::usecase;

namespace
{
    class DummyCapturedRawFrame : public ICapturedRawFrame
    {
    public:
        DummyCapturedRawFrame (uint16_t *data) :
            m_data (data),
            m_imageOffset (0)
        {

        }
        DummyCapturedRawFrame (uint16_t *data, std::size_t offset) :
            m_data (data),
            m_imageOffset (offset)
        {

        }
        uint16_t *getImageData()
        {
            return &m_data[m_imageOffset];
        }

        const uint16_t *getPseudoData() const
        {
            return m_data;
        }
    private:
        uint16_t *m_data;
        std::size_t m_imageOffset;
    };

    UseCaseDefinition createUC (uint16_t fps,
                                uint32_t modFreqGray,
                                uint32_t modFreq1,
                                uint32_t modFreq2,
                                uint16_t numPhases)
    {
        if (numPhases == 5)
        {
            return UseCaseFourPhase (
                       fps, modFreq1, royale::Pair<uint32_t, uint32_t> { 1u, 100000u }, 1u, 1u,
                       royale::usecase::ExposureGray::On,
                       royale::usecase::IntensityPhaseOrder::IntensityFirstPhase);
        }
        else
        {
            return UseCaseEightPhase (
                       fps, modFreq1, modFreq2, royale::Pair<uint32_t, uint32_t> { 1u, 100000u }, 1u, 1u, 1u,
                       royale::usecase::ExposureGray::On,
                       royale::usecase::IntensityPhaseOrder::IntensityFirstPhase);
        }
    }

}

Raw2RRF::Raw2RRF()
{
    setupUi (this);
}

Raw2RRF::~Raw2RRF()
{
}

void Raw2RRF::on_pbSelectRaw_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames (
                                this,
                                "Select raw files",
                                QDir::homePath(),
                                "Raw files (*.raw *.bin);;Any files (*.*)");
    if (fileNames.size())
    {
        lstSelectedFiles->clear();
        for (auto curFile : fileNames)
        {
            QListWidgetItem *lwi  = new QListWidgetItem (curFile);
            lwi->setTextAlignment (Qt::AlignRight);
            lstSelectedFiles->addItem (lwi);
        }
    }

    checkProcessButton();
}

void Raw2RRF::on_pbSelectCalib_clicked()
{
    QString fileName = QFileDialog::getOpenFileName (
                           this,
                           "Select calibration file",
                           QDir::homePath(),
                           "Calibration files (*.cal *.jgf);;Any files (*.*)");
    if (!fileName.isEmpty())
    {
        lblCalibFile->setText (fileName);
    }

    checkProcessButton();
}

void Raw2RRF::on_pbSelectOutput_clicked()
{
    QString fileName = QFileDialog::getSaveFileName (
                           this,
                           "Select output file",
                           QDir::homePath(),
                           "Royale recording files (*.rrf);;Any files (*.*)");
    if (!fileName.isEmpty())
    {
        lblOutputFile->setText (fileName);
    }

    checkProcessButton();
}

void Raw2RRF::checkProcessButton()
{
    // Enable the button if all input is available

    if (lstSelectedFiles->count() > 0 &&
            !lblCalibFile->text().isEmpty() &&
            !lblOutputFile->text().isEmpty())
    {
        pbProcess->setEnabled (true);
    }
    else
    {
        pbProcess->setEnabled (false);
    }
}

void Raw2RRF::on_pbProcess_clicked()
{
    QApplication::setOverrideCursor (Qt::WaitCursor);

    ImagerType imagerType;

    std::unique_ptr<royale::common::IPseudoDataInterpreter> pseudoDataInter;
    auto imagerStr = cbImager->currentText();
    if (imagerStr == "IRS2381")
    {
        imagerType = ImagerType::M2453_B11;
        pseudoDataInter.reset (new M2453_B11::PseudoDataInterpreterB11);
    }
    else
    {
        QMessageBox::warning (this, "Processing error", "Wrong imager");
        return;
    }

    std::vector<uint8_t> calibData;
    readFileToStdVector (lblCalibFile->text().toLatin1().data(), calibData);

    // Read in first raw file to retrieve resolution and number
    // of phases
    std::vector<uint16_t> rawData;
    auto item = lstSelectedFiles->item (0);
    rawData.clear();
    readFileToStdVector (item->text().toLatin1().data(), rawData);

    DummyCapturedRawFrame capturedData (&rawData[0]);

    auto rows = pseudoDataInter->getVerticalSize (capturedData);
    auto cols = pseudoDataInter->getHorizontalSize (capturedData);

    auto numPhases = rawData.size() / ( (rows + 1) * cols);

    auto fps = static_cast<uint16_t> (spbFPS->value());
    auto modFreqGray = static_cast<uint32_t> (spbFreqGray->value());
    auto modFreq1 = static_cast<uint32_t> (spbFreqFirst->value());
    auto modFreq2 = static_cast<uint32_t> (spbFreqSecond->value());

    float ntcR1 = static_cast<float> (spbNtcR1->value());
    float ntcRNTC0 = static_cast<float> (spbNtcRNTC0->value());
    float ntcRefTemp = static_cast<float> (spbNtcRefTemp->value());
    float ntcThermistor = static_cast<float> (spbNtcThermistor->value());

    NtcTempAlgo ntcTemp (ntcR1, ntcRNTC0, ntcRefTemp, ntcThermistor);

    UseCaseDefinition uc = createUC (fps, modFreqGray, modFreq1, modFreq2, static_cast<uint16_t> (numPhases));
    uc.setImage (cols, rows);

    royale::Vector<uint32_t> modFreqs;

    auto rfs = uc.getRawFrameSets();
    for (auto curRFS : rfs)
    {
        for (auto i = 0u; i < curRFS.countRawFrames(); ++i)
        {
            modFreqs.push_back (curRFS.modulationFrequency);
        }
    }

    royale::ProcessingParameterMap params;
    if (numPhases == 5)
    {
        params = moduleconfig::CommonProcessingParams1Frequency;
    }
    else if (numPhases == 9)
    {
        params = moduleconfig::CommonProcessingParams2Frequencies;
    }

    CameraRecord writer (0, 0, 0, "OEMCamera", imagerType);
    writer.startRecord (lblOutputFile->text().toLatin1().data(),
                        calibData,
                        "0000-0000-0000-0000");

    std::chrono::microseconds currentTimestamp = std::chrono::duration_cast<std::chrono::microseconds> (CapturedUseCase::CLOCK_TYPE::now().time_since_epoch());
    auto fpsDelay = 1000000 / fps;


    for (auto i = 0; i < lstSelectedFiles->count(); ++i)
    {
        auto item = lstSelectedFiles->item (i);
        rawData.clear();
        readFileToStdVector (item->text().toLatin1().data(), rawData);

        royale::Vector<uint32_t> exposureTimes;

        float temperature = 0.0f;
        // Temperature
        uint16_t NtcOffset = 0;

        std::vector<ICapturedRawFrame *> rawFrames;
        for (auto i = 0u; i < numPhases; ++i)
        {
            DummyCapturedRawFrame *currentFrame = new DummyCapturedRawFrame (&rawData[i * ( (rows + 1) * cols)], cols);

            auto expoTime = pseudoDataInter->getExposureTime (*currentFrame, modFreqs[i]);
            exposureTimes.push_back (expoTime);

            auto values = pseudoDataInter->getTemperatureRawValues (*currentFrame);
            uint16_t vRef1 = values[0];
            uint16_t vRef2 = values[1];
            uint16_t vNtc1 = values[2];
            uint16_t vNtc2 = values[3];
            if (vRef1 != 0 && vRef2 != 0 && vNtc1 != 0 && vNtc2 != 0)
            {
                float frameTemp = 0.0f;
                try
                {
                    frameTemp = ntcTemp.calcTemperature (vRef1, vNtc1, vRef2, vNtc2, NtcOffset);
                }
                catch (...)
                {
                    frameTemp = 0.0f;
                }

                if (frameTemp > 0.0f)
                {
                    temperature = frameTemp;
                }
            }

            rawFrames.push_back (currentFrame);
        }


        std::unique_ptr<CapturedUseCase> cuc{ new CapturedUseCase{ nullptr, temperature, currentTimestamp, exposureTimes } };
        writer.captureCallback (rawFrames, uc, uc.getStreamIds().at (0), std::move (cuc));

        for (auto &curFrame : rawFrames)
        {
            delete curFrame;
        }

        currentTimestamp += std::chrono::microseconds (fpsDelay);
    }

    writer.stopRecord();

    QApplication::restoreOverrideCursor();

    QMessageBox::information (this, "Conversion done", "Raw files have been converted");
}
