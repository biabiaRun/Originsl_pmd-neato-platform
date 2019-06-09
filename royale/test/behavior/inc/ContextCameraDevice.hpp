#pragma once

#include <royale/CameraManager.hpp>
#include <royale/ICameraDevice.hpp>
#include <MockCaptureListener.hpp>
#include <royale/Status.hpp>
#include <RoyaleProfiler.hpp>

#include <chrono>
#include <memory>
#include <vector>

namespace royale
{
    namespace bdd
    {
        class ContextCameraDevice
        {
        public:
            enum CaptureMode
            {
                RawMode = 0,
                DepthMode
            };

            enum MeasureMode
            {
                Default = 0,
                Equivalent
            };

            ContextCameraDevice();
            ~ContextCameraDevice();

            royale::CameraStatus connect (String activationString = String());
            royale::CameraStatus registerDataListener();
            royale::CameraStatus registerDataListenerExtended();
            royale::CameraStatus setUseCase (const std::string &modeName);
            royale::CameraStatus setExposureMode (ExposureMode exposureMode);
            royale::CameraStatus setPlaybackFile (const std::string &playbackFile);
            royale::CameraStatus startCapture();
            royale::CameraStatus stopCapture();
            royale::CameraStatus setExposureTime (uint32_t time);
            royale::CameraStatus singleShot();
            royale::CameraStatus checkDepthData();

            royale::CameraStatus setUseCaseDefault();
            royale::CameraStatus setUseCaseFastest();
            royale::CameraStatus getUseCaseFastest (royale::String &);
            std::string getLastUseCaseName();

            ICameraDevice *getCameraDevice()
            {
                return m_cameraDevice.get();
            }
            MockDepthCaptureListener *getDepthListener()
            {
                return m_listenerDepth.get();
            }

            CameraStatus startRecording (const std::string &fileName = "royale_bdd_record.rrf");
            CameraStatus stopRecording();

            CameraStatus destroyCamera();

            void setRecordFileName (const std::string &recordFilename);
            std::string getRecordFileName();

            royale::CameraStatus sweepAllUseCasesAndCalcFPS (MeasureMode measureMode = MeasureMode::Equivalent, unsigned int time = 0);
            royale::CameraStatus checkCorrectFPSForAllUseCases (unsigned int range);

            bool isCapturing() const;
            bool isMixedMode (const std::string &modeName) const;

            void diffBetweenUseCaseSwitch (int32_t timeDiff) const;
            void diffBetweenExposureModeSwitch (int32_t timeDiffNanoSeconds) const;
            void diffBetweenExposureTimeSwitch (int32_t timeDiffNanoSeconds) const;
            void measureAverageFrameRate (unsigned int seconds);
            void getExposureLimits (unsigned int &maxValue, unsigned int &minValue);

            //!< get difference to current timestamp in milliseconds
            int64_t startUpPeriod();
            int32_t getDiffToLastCallback();
            int32_t getCallbackCounts() const;
            float getAverageFps() const;
            long getElapsedTimeMS() const;
            uint32_t getLastExposureTime() const;

            royale::CameraStatus startTimer();
            royale::CameraStatus stopTimer();
            royale::CameraStatus getCurrentTimeInMS();

            int64_t getStartCaptureDuration();
            int64_t getStopCaptureDuration();

            bool fileExists (const std::string &fileName);

        private:
            std::unique_ptr<royale::ICameraDevice> m_cameraDevice;
            std::shared_ptr<MockDepthCaptureListener> m_listenerDepth;
            std::shared_ptr<MockExtendedCaptureListener> m_listenerRaw;

            std::string m_serialNumber;
            std::string m_playbackFile;
            std::string m_recordFileName;
            std::string m_lastUseCaseName;

            int64_t m_elapsedTimeMS;
            std::chrono::milliseconds m_currentTimeMS;
            float m_averageFPSRaw;
            float m_averageFPSDepth;
            uint32_t m_lastExposureTime;
            std::map<std::string, float> m_measuredFPSDepth;
            std::map<std::string, float> m_measuredFPSRaw;

            int64_t m_startCaptureDuration;
            int64_t m_stopCaptureDuration;

            CaptureMode m_capMode;
            ExposureMode m_lastExposureMode;

            // Time marks
            royale::common::RoyaleProfiler m_startCaptureMark;
            royale::common::RoyaleProfiler m_stopCaptureMark;
            royale::common::RoyaleProfiler m_timerStart;

            std::vector<int64_t> m_operationModeChangeDurations;
            std::vector<int64_t> m_exposureTimeChangeDurations;
            std::vector<int64_t> m_exposureModeChangeDurations;
        };

    }
}
