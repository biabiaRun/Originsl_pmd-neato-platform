/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale/ICameraDevice.hpp>
#include <processing/IProcessing.hpp>
#include <processing/ExtendedData.hpp>
#include <royale/String.hpp>
#include <device/CameraCore.hpp>
#include <config/ICoreConfig.hpp>
#include <collector/IFrameCaptureListener.hpp>
#include <royale/ExposureMode.hpp>
#include <royale/IEventListener.hpp>
#include <royale/IRecord.hpp>

#include <mutex>

#define ROYALE_ACCESS_LEVEL_CHECK(required) \
    { \
    if(m_accessLevel < required) { \
        return royale::CameraStatus::INSUFFICIENT_PRIVILEGES; } \
    }

namespace royale
{
    namespace device
    {
        /**
        * This is the implementation of the ICameraDevice
        */
        class CameraDeviceBase :
            virtual public royale::ICameraDevice
        {
        public:
            CameraDeviceBase (CameraAccessLevel level, const std::string &id, const std::string &cameraName,
                              std::shared_ptr<royale::processing::IProcessing> processing,
                              royale::CallbackData cbData = royale::CallbackData::Depth);

            /************************************************************************
            *  ICameraDevice interface
            ************************************************************************/
            // Level 1
            ROYALE_API royale::CameraStatus getId (royale::String &id) const override;
            ROYALE_API royale::CameraStatus getCameraName (royale::String &cameraName) const override;
            ROYALE_API royale::CameraStatus getUseCases (royale::Vector<royale::String> &useCases) const override;
            ROYALE_API royale::CameraStatus getCurrentUseCase (royale::String &useCase) const override;
            ROYALE_API royale::CameraStatus getExposureMode (royale::ExposureMode &exposureMode, royale::StreamId streamId) const override = 0;
            ROYALE_API royale::CameraStatus getAccessLevel (royale::CameraAccessLevel &accessLevel) const override;
            ROYALE_API royale::CameraStatus isCalibrated (bool &calibrated) const override;
            ROYALE_API royale::CameraStatus getLensParameters (royale::LensParameters &param) const override;

            ROYALE_API royale::CameraStatus registerDataListener (royale::IDepthDataListener *listener) override;
            ROYALE_API royale::CameraStatus unregisterDataListener() override;
            ROYALE_API royale::CameraStatus registerRecordListener (royale::IRecordStopListener *listener) override;
            ROYALE_API royale::CameraStatus unregisterRecordListener() override;
            ROYALE_API royale::CameraStatus registerDepthImageListener (royale::IDepthImageListener *listener) override;
            ROYALE_API royale::CameraStatus unregisterDepthImageListener() override;
            ROYALE_API royale::CameraStatus registerSparsePointCloudListener (royale::ISparsePointCloudListener *listener) override;
            ROYALE_API royale::CameraStatus unregisterSparsePointCloudListener() override;
            ROYALE_API royale::CameraStatus registerIRImageListener (royale::IIRImageListener *listener) override;
            ROYALE_API royale::CameraStatus unregisterIRImageListener() override;

            ROYALE_API royale::CameraStatus registerExposureListener (royale::IExposureListener *listener) override;
            ROYALE_API royale::CameraStatus registerExposureListener (royale::IExposureListener2 *listener) override;
            ROYALE_API royale::CameraStatus unregisterExposureListener() override;

            ROYALE_API royale::CameraStatus registerEventListener (royale::IEventListener *listener) override;
            ROYALE_API royale::CameraStatus unregisterEventListener() override;

            ROYALE_API royale::CameraStatus registerDataListenerExtended (royale::IExtendedDataListener *listener) override;
            ROYALE_API royale::CameraStatus unregisterDataListenerExtended() override;

            ROYALE_API royale::CameraStatus setCalibrationData (const royale::String &filename) override;
            ROYALE_API royale::CameraStatus setCalibrationData (const royale::Vector<uint8_t> &data) override;
            ROYALE_API royale::CameraStatus getCalibrationData (royale::Vector<uint8_t> &data) override;

            ROYALE_API royale::CameraStatus setFilterLevel (const royale::FilterLevel level, royale::StreamId streamId = 0) override;
            ROYALE_API royale::CameraStatus getFilterLevel (royale::FilterLevel &level, royale::StreamId streamId = 0) const override;

            // Level 2
            ROYALE_API royale::CameraStatus setCallbackData (uint16_t cbData) override = 0;
            ROYALE_API royale::CameraStatus setCallbackData (royale::CallbackData cbData) override;

            ROYALE_API royale::CameraStatus setProcessingParameters (const royale::ProcessingParameterVector &parameters, royale::StreamId streamId) override;
            ROYALE_API royale::CameraStatus getProcessingParameters (royale::ProcessingParameterVector &parameters, royale::StreamId streamId) override;

        protected:

            virtual void saveProcessingParameters (royale::StreamId streamId) = 0;

            virtual royale::CameraStatus setupListeners() = 0;

            //template functions must be implemented in every cpp file, so better put it here
            template<typename T>
            royale::CameraStatus setSingleListener (T *newListener, T **destination)
            {
                auto oldListener = *destination;
                *destination = newListener;
                auto ret = setupListeners();
                if (ret != CameraStatus::SUCCESS)
                {
                    *destination = oldListener;
                }
                return ret;
            }

            // Retrieves the current exposure times from the given UseCaseDefinition and calls
            // the appropriate exposure listeners
            void callExposureListeners (const royale::usecase::UseCaseDefinition &definition);

        protected:

            /**
            * Ensure streamId is valid.
            * Checks whether streamId is a valid stream in the current usecase.
            * For non mixed mode use cases, streamId 0 (which is otherwise not a valid StreamId)
            * is replaced with the streamId of the first (and only) stream in the usecase.
            * If the stream doesn't exist in the usecase (or 0 is passed for mixed mode use cases)
            * this function throws an exception containing an appropriate CameraStatus.
            *
            * \param[in,out] streamId the StreamId to be checked/normalized
            */
            void normalizeStreamId (royale::StreamId &streamId) const;

            royale::String m_id;
            royale::String m_cameraName;
            royale::String m_currentUseCaseName;

            /**
            * m_callbackData is used to tell Royale which callbacks should be used (raw, depth,
            * extended). The different callback types have different requirements concerning
            * processing and calibration.
            * Usually Royale will start with depth callback which requires a valid calibration.
            * But there are instances (e.g. calibration) where we need to retrieve (raw) data
            * from the camera without having calibration data. In this case m_callbackData has
            * to be set to raw before initializing the camera. This will cause the processing
            * to use a fake calibration to still be able to provide the auto exposure feature.
            *
            * Switching back to the depth or extended callback will update the calibration
            * to the one loaded from the camera or from file.
            */
            uint16_t m_callbackData;

            royale::Vector<String> m_supportedUseCaseNames;
            royale::CameraAccessLevel m_accessLevel;
            std::unique_ptr<royale::IRecord> m_recording;
            royale::IEventListener *m_eventListener;
            royale::processing::DataListeners m_listeners;
            royale::IRecordStopListener *m_recordStopListener;
            std::shared_ptr<royale::processing::IProcessing> m_processing;
            royale::Vector<uint8_t> m_calibrationData;

            royale::Pair<uint32_t, uint32_t> m_exposureLimits;
            royale::CameraStatus setSingleListener (void *newListener, void **destination);

            royale::IExposureListener *m_exposureListener1;
            royale::IExposureListener2 *m_exposureListener2;

            std::recursive_mutex m_exposureListenerMutex;

            royale::usecase::UseCaseDefinition *m_currentUseCaseDefinition;

            /** Set when initialize() is called, even if initialization fails.  */
            bool m_isPartiallyInitialized;
            /** Set when initialize() succeeds.  */
            bool m_isInitialized;

            // Mutex to synchronize the access to the current use case definition
            // from different threads
            mutable std::recursive_mutex m_currentUseCaseMutex;

            royale::Vector<uint16_t> m_streams;
        };
    }
}
