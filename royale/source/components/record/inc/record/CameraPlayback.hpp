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

#include <thread>
#include <mutex>

#include <device/CameraDeviceBase.hpp>
#include <record/FileReaderDispatcher.hpp>
#include <royale/IReplay.hpp>
#include <royale/IRecordStopListener.hpp>
#include <royale/IPlaybackStopListener.hpp>
#include <collector/IFrameCaptureListener.hpp>
#include <collector/IFrameCaptureReleaser.hpp>
#include <processing/Processing.hpp>
#include <record/RecordedRawFrame.hpp>
#include <processing/ExtendedData.hpp>
#include <common/EventForwarder.hpp>

namespace royale
{
    namespace record
    {
        /**
        * This contains the CameraPlayback class.
        */
        class CameraPlayback :
            public royale::device::CameraDeviceBase,
            public royale::IReplay,
            public royale::collector::IFrameCaptureReleaser
        {
        public:
            /**
            * Constructor for the CameraPlayback class.
            * After construction the CameraPlayback class will be set to looping mode and will
            * play the file according to the recorded timestamps. This behavior can be changed
            * with the IReplay interface.
            * @param level Camera access level
            * @param filename Filename for playback
            */
            ROYALE_API CameraPlayback (royale::CameraAccessLevel level, const std::string &filename);

            /**
            * Destructor for the CameraPlayback class.
            */
            ROYALE_API virtual ~CameraPlayback();

            /************************************************************************
             *  ICameraDevice functions
             ************************************************************************/

            // Level 1
            ROYALE_API royale::CameraStatus initialize() override;
            ROYALE_API royale::CameraStatus setUseCase (const royale::String &name) override;
            ROYALE_API royale::CameraStatus startCapture() override;
            ROYALE_API royale::CameraStatus stopCapture() override;
            ROYALE_API royale::CameraStatus getMaxSensorWidth (uint16_t &maxSensorWidth) const override;
            ROYALE_API royale::CameraStatus getMaxSensorHeight (uint16_t &maxSensorHeight) const override;
            ROYALE_API royale::CameraStatus isConnected (bool &connected) const override;
            ROYALE_API royale::CameraStatus isCapturing (bool &capturing) const override;

            ROYALE_API royale::CameraStatus registerEventListener (royale::IEventListener *listener) override;
            ROYALE_API royale::CameraStatus unregisterEventListener() override;

            // Level 2
            ROYALE_API royale::CameraStatus setCallbackData (uint16_t cbData) override;
            ROYALE_API royale::CameraStatus setProcessingParameters (const royale::ProcessingParameterVector &parameters, royale::StreamId streamId) override;
            ROYALE_API royale::CameraStatus getProcessingParameters (royale::ProcessingParameterVector &parameters, royale::StreamId streamId) override;

            // Level 3 (unimplemented)
            ROYALE_API royale::CameraStatus getLensCenter (uint16_t &x, uint16_t &y) override;


            // Level 1 unimplemented (not useful for a recording)
            ROYALE_API royale::CameraStatus setExposureTime (uint32_t exposureTime, royale::StreamId streamId) override;
            ROYALE_API royale::CameraStatus setExposureMode (royale::ExposureMode exposureMode, royale::StreamId streamId) override;
            ROYALE_API royale::CameraStatus getExposureMode (royale::ExposureMode &exposureMode, royale::StreamId streamId) const override;

            ROYALE_API royale::CameraStatus setFrameRate (uint16_t framerate) override;

            ROYALE_API royale::CameraStatus getFrameRate (uint16_t &frameRate) const override;
            ROYALE_API royale::CameraStatus getMaxFrameRate (uint16_t &maxFrameRate) const override;

            ROYALE_API royale::CameraStatus startRecording (const royale::String &fileName,
                    uint32_t numberOfFrames,
                    uint32_t frameSkip = 0,
                    uint32_t msSkip = 0) override;
            ROYALE_API royale::CameraStatus stopRecording() override;
            ROYALE_API royale::CameraStatus getCameraInfo (royale::Vector<royale::Pair<royale::String, royale::String>> &camInfo) const override;
            ROYALE_API royale::CameraStatus getStreams (royale::Vector<uint16_t> &streams) const override;
            ROYALE_API royale::CameraStatus getNumberOfStreams (const royale::String &name, uint32_t &nrStreams) const override;

            ROYALE_API royale::CameraStatus setExternalTrigger (bool useExternalTrigger) override;

            // Level 2 unimplemented (not useful for a recording)
            ROYALE_API royale::CameraStatus setExposureTimes (const Vector<uint32_t> &exposureTimes, royale::StreamId streamId = 0) override;
            ROYALE_API royale::CameraStatus setExposureForGroups (const royale::Vector<uint32_t> &exposureTimes) override;
            ROYALE_API royale::CameraStatus writeCalibrationToFlash() override;
            ROYALE_API royale::CameraStatus getExposureLimits (royale::Pair<uint32_t, uint32_t> &exposureLimits, royale::StreamId streamId) const override;

            ROYALE_API royale::CameraStatus getExposureGroups (royale::Vector< royale::String > &exposureGroups) const override;
            ROYALE_API royale::CameraStatus getExposureLimits (const String &exposureGroup, royale::Pair<uint32_t, uint32_t> &exposureLimits) const override;
            ROYALE_API royale::CameraStatus setExposureTime (const String &exposureGroup, uint32_t exposureTime) override;

            // Level 3 unimplemented (not useful for a recording)
            ROYALE_API royale::CameraStatus writeDataToFlash (const royale::Vector<uint8_t> &data) override;
            ROYALE_API royale::CameraStatus writeDataToFlash (const royale::String &filename) override;
            ROYALE_API royale::CameraStatus setDutyCycle (double dutyCycle, uint16_t index) override;
            ROYALE_API royale::CameraStatus writeRegisters (const royale::Vector<royale::Pair<royale::String, uint64_t>> &registers) override;
            ROYALE_API royale::CameraStatus readRegisters (royale::Vector<royale::Pair<royale::String, uint64_t>> &registers) override;

            ROYALE_API royale::CameraStatus shiftLensCenter (int16_t tx, int16_t ty) override;

            // Level 4 unimplemented (not useful for a recording)
            ROYALE_API royale::CameraStatus initialize (const royale::String &initUseCase) override;

            /************************************************************************
             *  CaptureReleaser functions
             ************************************************************************/

            ROYALE_API void releaseCapturedFrames (std::vector<royale::common::ICapturedRawFrame *> frame) override;

            /************************************************************************
             *  IReplay functions
             ************************************************************************/

            ROYALE_API royale::CameraStatus seek (const uint32_t frameNumber) override;
            ROYALE_API void loop (const bool restart) override;
            ROYALE_API void useTimestamps (const bool timestampsUsed) override;
            ROYALE_API uint32_t frameCount() override;
            ROYALE_API uint32_t currentFrame() override;
            ROYALE_API void pause() override;
            ROYALE_API void resume() override;

            ROYALE_API void registerStopListener (royale::IPlaybackStopListener *listener) override;
            ROYALE_API void unregisterStopListener() override;

            ROYALE_API uint16_t getFileVersion() override;

        private:

            void aquisitionFunction();

            void internalCallback (std::vector<royale::common::ICapturedRawFrame *> &frames,
                                   const royale::usecase::UseCaseDefinition &definition,
                                   std::unique_ptr<const royale::collector::CapturedUseCase> capturedCase,
                                   royale::StreamId streamId);

            void readFrame (std::vector<std::vector<uint16_t>> &frames,
                            std::unique_ptr<royale::usecase::UseCaseDefinition> &definition,
                            std::chrono::milliseconds &timestamp,
                            float &illuminationTemperature,
                            std::vector<uint32_t> &capturedExposureTimes,
                            royale::ProcessingParameterMap &parameterMap,
                            std::vector<std::pair<std::string, std::vector<uint8_t>>> &additionalData,
                            royale::StreamId &streamId);

            void saveProcessingParameters (royale::StreamId streamId) override;

            royale::CameraStatus setupListeners() override;

        private:
            FileReaderDispatcher m_reader;
            royale::collector::IFrameCaptureListener *m_captureListener;
            bool m_isCapturing;
            royale::String m_filename;
            bool m_timestampsUsed;
            bool m_loop;
            bool m_oneShot;
            std::thread aquisitionThread;
            std::unique_ptr<royale::common::IPseudoDataInterpreter> m_pseudoDataInterpreter;
            royale::IPlaybackStopListener *m_stopListener;
            bool m_seeked;
            std::vector<uint32_t> m_capturedExposureTimes;
            royale::Vector<uint32_t> m_modulationFrequencies;
            royale::Vector<royale::StreamId> m_streamIds;

            // Parameters set by setProcessingParameters function. These override
            // the ones saved in the recording. By default this map is empty and the
            // recorded parameters will be used
            std::map <royale::StreamId, royale::ProcessingParameterMap> m_userParameters;

            // Events

            /** Event source */
            royale::EventForwarder m_eventForwarder;


            // Mutex that is used in the playback to synchronize the use case change
            std::mutex m_playbackMutex;
        };
    }
}
