/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <device/CameraDeviceBase.hpp>
#include <processing/ExtendedData.hpp>
#include <royale/String.hpp>
#include <device/CameraCore.hpp>
#include <config/ICoreConfig.hpp>
#include <collector/IFrameCaptureListener.hpp>
#include <royale/ExposureMode.hpp>
#include <royale/IEventListener.hpp>
#include <royale/IRecord.hpp>

namespace royale
{
    namespace device
    {
        /**
         * This is the implementation of the ICameraDevice v2
         */
        class CameraDevice :
            public royale::device::CameraDeviceBase,
            public royale::IRecordStopListener,
            protected royale::IExposureListener2
        {
        public:
            ROYALE_API CameraDevice (
                royale::CameraAccessLevel level,
                const std::string &id,
                std::unique_ptr<CameraCore> module,
                std::shared_ptr<const royale::config::ICoreConfig> config,
                std::shared_ptr<royale::processing::IProcessing> processing,
                royale::CallbackData cbData = royale::CallbackData::Depth);

            ROYALE_API CameraDevice (const CameraDevice &) = delete; // to ensure the class cannot be copied (due to mutex thread locking mechanism)
            ROYALE_API ~CameraDevice ();

            /************************************************************************
            *  ICameraDevice interface
            ************************************************************************/

            // Level 1
            ROYALE_API royale::CameraStatus initialize () override;

            ROYALE_API royale::CameraStatus getCameraInfo (royale::Vector<royale::Pair<royale::String, royale::String>> &camInfo) const override;
            ROYALE_API royale::CameraStatus setUseCase (const royale::String &name) override;
            ROYALE_API royale::CameraStatus setUseCase (size_t idx);
            ROYALE_API royale::CameraStatus setExposureTime (uint32_t exposureTime, royale::StreamId streamId) override;
            ROYALE_API royale::CameraStatus setExposureMode (royale::ExposureMode exposureMode, royale::StreamId streamId) override;
            ROYALE_API royale::CameraStatus getExposureMode (royale::ExposureMode &exposureMode, royale::StreamId streamId) const override;
            ROYALE_API royale::CameraStatus getExposureLimits (royale::Pair<uint32_t, uint32_t> &exposureLimits, royale::StreamId streamId) const override;
            ROYALE_API royale::CameraStatus startCapture () override;
            ROYALE_API royale::CameraStatus stopCapture () override;
            ROYALE_API royale::CameraStatus getMaxSensorWidth (uint16_t &maxSensorWidth) const override;
            ROYALE_API royale::CameraStatus getMaxSensorHeight (uint16_t &maxSensorHeight) const override;
            ROYALE_API royale::CameraStatus isConnected (bool &connected) const override;
            ROYALE_API royale::CameraStatus isCapturing (bool &capturing) const override;
            ROYALE_API royale::CameraStatus startRecording (const royale::String &fileName,
                    uint32_t numberOfFrames,
                    uint32_t frameSkip = 0,
                    uint32_t msSkip = 0) override;
            ROYALE_API royale::CameraStatus stopRecording () override;
            ROYALE_API royale::CameraStatus registerEventListener (royale::IEventListener *listener) override;
            ROYALE_API royale::CameraStatus unregisterEventListener() override;
            ROYALE_API royale::CameraStatus setFrameRate (uint16_t framerate) override;
            ROYALE_API royale::CameraStatus getFrameRate (uint16_t &frameRate) const override;
            ROYALE_API royale::CameraStatus getMaxFrameRate (uint16_t &maxFrameRate) const override;
            ROYALE_API royale::CameraStatus getStreams (royale::Vector<royale::StreamId> &streams) const override;
            ROYALE_API royale::CameraStatus getNumberOfStreams (const royale::String &name, uint32_t &nrStreams) const override;
            ROYALE_API royale::CameraStatus setExternalTrigger (bool useExternalTrigger) override;

            // Level 2
            ROYALE_API royale::CameraStatus setExposureTimes (const Vector<uint32_t> &exposureTimes, royale::StreamId streamId = 0) override;
            ROYALE_API royale::CameraStatus setExposureForGroups (const royale::Vector<uint32_t> &exposureTimes) override;
            ROYALE_API royale::CameraStatus setExposureTime (const String &exposureGroup, uint32_t exposureTime) override;
            ROYALE_API royale::CameraStatus getExposureGroups (royale::Vector< royale::String > &exposureGroups) const override;
            ROYALE_API royale::CameraStatus getExposureLimits (const String &exposureGroup, royale::Pair<uint32_t, uint32_t> &exposureLimits) const override;
            ROYALE_API royale::CameraStatus setCallbackData (uint16_t cbData) override;
            ROYALE_API royale::CameraStatus writeCalibrationToFlash () override;

            // Level 3
            ROYALE_API royale::CameraStatus writeDataToFlash (const royale::Vector<uint8_t> &data) override;
            ROYALE_API royale::CameraStatus writeDataToFlash (const royale::String &filename) override;
            ROYALE_API royale::CameraStatus setDutyCycle (double dutyCycle, uint16_t index) override;
            ROYALE_API royale::CameraStatus writeRegisters (const royale::Vector<royale::Pair<royale::String, uint64_t>> &registers) override;
            ROYALE_API royale::CameraStatus readRegisters (royale::Vector<royale::Pair<royale::String, uint64_t>> &registers) override;

            ROYALE_API royale::CameraStatus shiftLensCenter (int16_t tx, int16_t ty) override;
            ROYALE_API royale::CameraStatus getLensCenter (uint16_t &x, uint16_t &y) override;

            // Level 4
            ROYALE_API royale::CameraStatus initialize (const royale::String &initUseCase) override;


            /************************************************************************
            * IRecordStopListener interface
            * (will be called automatically once recording is stopped)
            ************************************************************************/

            void onRecordingStopped (const uint32_t numFrames) override;

            /************************************************************************
            * IExposureListener2 interface
            ************************************************************************/

            void onNewExposure (const uint32_t exposureTime, const royale::StreamId streamId) override;

            /**
             * If recording should be enabled, a recording engine must be set
             */
            ROYALE_API void setRecordingEngine (std::unique_ptr<royale::IRecord> recording);

        private:
            royale::CameraStatus activateUseCase ();
            royale::CameraStatus activateLastUseCase (bool wasActive);

            void saveProcessingParameters (royale::StreamId streamId) override;

            royale::CameraStatus setupListeners() override;
            royale::CameraStatus updateRecordingEngineListener();
            royale::CameraStatus updateExposureTimes (const royale::Vector<uint32_t> &exposureTimes);

            void scaleExposuresForUseCase();
            void updateSupportedUseCases();
            bool isCapturing() const;
            void initLensOffset();

            /**
            * Update the exposure time.
            * This one is used internally as setExposureTime should not be used
            * when auto exposure is enabled.
            *
            *  \param exposureTime exposure time in microseconds
            *  \param streamId which stream to change exposure for
            */
            royale::CameraStatus updateExposureTime (uint32_t exposureTime, royale::StreamId streamId);

            /**
            * Checks if autoexposure is enabled for the given streamId.
            * Returns true if it is enabled. If there is a problem during the check
            * this method can throw!
            *
            *  \param streamId which stream to check
            */
            bool isAutoexposureEnabled (const royale::StreamId streamId);

            std::unique_ptr<royale::device::CameraCore> m_cameraCore;
            std::shared_ptr<const royale::config::ICoreConfig> m_config;
            royale::usecase::UseCaseList m_availableUseCases;
            royale::Vector<royale::ProcessingParameterMap> m_currentParameters;
            std::mutex m_recordingMutex;
            std::pair<uint16_t, uint16_t> m_lensCenter;

            royale::String m_imagerSerial;
            std::map<royale::StreamId, royale::Vector<uint32_t>> m_modulationFrequencies;

            bool m_useExternalTrigger;

            std::map<royale::StreamId, royale::ExposureMode> m_exposureModes;
        };
    }
}
