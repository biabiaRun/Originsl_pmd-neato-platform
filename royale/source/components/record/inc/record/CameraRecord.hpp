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

#include <mutex>
#include <atomic>

#include <config/ImagerType.hpp>
#include <record/FileWriter.hpp>
#include <royale/IRecordStopListener.hpp>
#include <royale/String.hpp>
#include <royale/IRecord.hpp>
#include <royale/ProcessingFlag.hpp>
#include <collector/IFrameCaptureReleaser.hpp>

namespace royale
{
    namespace record
    {
        class CameraRecord : public royale::IRecord
        {
        public:
            /**
            * Constructor for the CameraRecord class.
            * @param recordStopListener Listener that gets notified as soon as the recording stops. Can be NULL.
            * @param captureListener Listener that processes the raw data afterwards. Can be NULL, but then a CaptureReleaser has to be provided.
            * @param releaser Called to release the frames if no listener is provided. Can be NULL if a listener is provided.
            * @param cameraName A unique string which is used to identify the module in the recording format
            * @param imagerType The enumerated imager type (will be converted to a string for the .rrf format)
            */
            ROYALE_API CameraRecord (royale::IRecordStopListener *recordStopListener,
                                     royale::collector::IFrameCaptureListener *captureListener,
                                     royale::collector::IFrameCaptureReleaser *releaser,
                                     royale::String cameraName,
                                     royale::config::ImagerType imagerType);

            /**
            * Destructor for the CameraRecord class.
            */
            ROYALE_API virtual ~CameraRecord();

            ROYALE_API void captureCallback (std::vector<royale::common::ICapturedRawFrame *> &frames,
                                             const royale::usecase::UseCaseDefinition &definition,
                                             royale::StreamId streamId,
                                             std::unique_ptr<const royale::collector::CapturedUseCase> capturedCase) override;
            ROYALE_API void releaseAllFrames () override;

            // IRecord interface
            ROYALE_API bool isRecording() override;
            ROYALE_API void setProcessingParameters (const royale::ProcessingParameterVector &parameters, const royale::StreamId streamId) override;
            ROYALE_API void startRecord (const royale::String &filename, const std::vector<uint8_t> &calibrationData,
                                         const royale::String &imagerSerial,
                                         const uint32_t numFrames = 0, const uint32_t frameSkip = 0, const uint32_t msSkip = 0) override;
            ROYALE_API void resetParameters() override;
            ROYALE_API void stopRecord() override;
            ROYALE_API bool setFrameCaptureListener (royale::collector::IFrameCaptureListener *captureListener) override;

        private:

            void putFrame (const std::vector<royale::common::ICapturedRawFrame *> &frames,
                           const royale::usecase::UseCaseDefinition &definition,
                           const royale::StreamId streamId,
                           const royale::collector::CapturedUseCase &capturedCase,
                           const royale::ProcessingParameterMap &parameterMap);


            /**
            * Listener called by the callback after the frame is written.
            */
            royale::collector::IFrameCaptureListener *m_listener;

            /**
            * FileWriter object which handles the file.
            */
            FileWriter m_writer;

            /**
            * Current parameter settings.
            */
            std::map<royale::StreamId, royale::ProcessingParameterMap> m_parameterMap;

            /**
            * How many frames should be recorded.
            */
            uint32_t m_framesToRecord;

            /**
            * How many frames should be skipped between recorded frames.
            */
            uint32_t m_framesToSkip;
            uint32_t m_framesSkipped;

            /**
            * How many milliseconds should be skipped between recorded frames.
            */
            uint32_t m_msToSkip;
            std::chrono::milliseconds m_lastCapture;

            /**
            * How many frames are already recorded.
            */
            uint32_t m_currentFrame;

            /**
            * Is a recording running.
            */
            std::atomic<bool> m_isRecording;

            /**
            * Listener which will be called if the recording is stopped.
            */
            royale::IRecordStopListener *m_recordStopListener;

            /**
            * File mutex.
            */
            std::mutex m_mutex;

            /**
             * Camera name of the used camera
             */
            royale::String m_cameraName;

            /**
             * ImagerType which is only used for informational purposes
             */
            royale::String m_imagerType;

            /**
            * Pseudo data interpreter which has to be used for replay
            */
            royale::String m_pseudoDataInter;

            /**
            * Capture releaser;
            */
            royale::collector::IFrameCaptureReleaser *m_releaser;
        };
    }
}
