/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <royale/IDepthDataListener.hpp>
#include <royale/IDepthImageListener.hpp>
#include <royale/ISparsePointCloudListener.hpp>
#include <royale/IIRImageListener.hpp>

#include <royale/FilterLevel.hpp>
#include <royale/IExtendedDataListener.hpp>
#include <royale/String.hpp>
#include <royale/LensParameters.hpp>
#include <royale/ProcessingFlag.hpp>
#include <royale/IExposureListener2.hpp>
#include <usecase/UseCaseDefinition.hpp>
#include <collector/IFrameCaptureListener.hpp>
#include <processing/IRefineExposureTime.hpp>
#include <royale/StreamId.hpp>
#include <royale/ExposureMode.hpp>

#include <vector>

namespace royale
{
    namespace processing
    {
        struct DataListeners
        {
            DataListeners() :
                depthDataListener (nullptr),
                depthImageListener (nullptr),
                sparsePointCloudListener (nullptr),
                irImageListener (nullptr),
                extendedListener (nullptr)
            {}

            ~DataListeners()
            {
                depthDataListener = nullptr;
                depthImageListener = nullptr;
                sparsePointCloudListener = nullptr;
                irImageListener = nullptr;
                extendedListener = nullptr;
            }

            royale::IDepthDataListener *depthDataListener;
            royale::IDepthImageListener *depthImageListener;
            royale::ISparsePointCloudListener *sparsePointCloudListener;
            royale::IIRImageListener *irImageListener;
            royale::IExtendedDataListener *extendedListener;
        };

        /**
        * This is the main interface for accessing the processing library. This interface should in
        * general used in all other components which are working with processing (e.g. CameraDevice)
        */
        class IProcessing : public royale::collector::IFrameCaptureListener,
            public royale::processing::IRefineExposureTime
        {
        public:

            virtual ~IProcessing() = default;

            /**
            * Used to register/unregister all the data listeners.
            * Listeners can be unregistered by setting them to nullptr.
            * @param listeners struct with all available listeners
            */
            virtual void registerDataListeners (const DataListeners &listeners) = 0;

            /**
            * Is called to set the current camera name used with the current processing.
            */
            virtual void setCameraName (const royale::String &cameraName) = 0;

            /**
            * Set calibration data to the processing module which will be used for subsequent calculations.
            * @param calibrationData Vector containing the calibration data
            */
            virtual void setCalibrationData (const std::vector<uint8_t> &calibrationData) = 0;

            /**
            * Returns a reference to the calibration data
            * @return vector containing the calibration data
            */
            virtual const std::vector<uint8_t> &getCalibrationData () const = 0;

            /**
            * Checks if calibration data is set.
            * @return True, if calibration data is set.
            */
            virtual bool hasCalibrationData() const = 0;

            /**
            * Checks if the calibration data provides lens center information.
            * @return True, if lens center information can be queried.
            */
            virtual bool hasLensCenterCalibration() const = 0;

            /**
            * Get the calibrated lens center for this camera module (given pixels).
            * @param[out] centerX X center
            * @param[out] centerY Y center
            */
            virtual void getLensCenterCalibration (uint16_t &centerX, uint16_t &centerY) = 0;

            /**
            * Get the calibrated lens parameters for this camera module.
            * @param[out] params lens parameters
            */
            virtual void getLensParameters (royale::LensParameters &params) = 0;

            /**
            * Adds a exposure listener which will get called when new exposure data is available.
            * Only one exposure listener is supported at a time, calling this will automatically
            * unregister any previously registered listener.
            *
            * @param exposureListener IExposureListener2 that will be added
            */
            virtual void registerExposureListener (royale::IExposureListener2 *exposureListener) = 0;

            /**
            * Unregisters the current exposure listener.
            */
            virtual void unregisterExposureListener () = 0;

            /**
            *  Set/alter processing parameters in order to control the data output. A list of processing flags
            *  is available as an enumeration. The `Variant` data type can take float, int, or bool. Please
            *  make sure to set the proper `Variant` type for the enum.
            */
            virtual void setProcessingParameters (const royale::ProcessingParameterMap &parameters,
                                                  const royale::StreamId streamId) = 0;

            /**
            *  Retrieve the available processing parameters which are used for the calculation.
            */
            virtual void getProcessingParameters (royale::ProcessingParameterMap &parameters,
                                                  const royale::StreamId streamId) = 0;

            /**
            * Is called internally to set the current use case after useCaseChanged has been called.
            * Checks if the given use case is supported by the current processing module and an valid
            * calibration is available. If a calibration mismatch is detected a consequent error is
            * thrown and a pseudo calibration is used alternatively.
            * @param useCase Current use case
            * @return Verification status.
            */
            virtual void setUseCase (const royale::usecase::UseCaseDefinition &useCase) = 0;

            /**
            * Checks if the given use case is supported by the current processing module.
            * @param useCase Use case that should be checked
            * @return Verification status.
            */
            virtual royale::usecase::VerificationStatus verifyUseCase (const royale::usecase::UseCaseDefinition &useCase) = 0;

            /**
            * Checks if the processing is ready to calculate data for the current use case.
            * Some IProcessing implementations might need information like calibration data
            * before they are ready for processing. As this calibration data might be
            * sufficient for one use case but not for another (e.g. using different modulation
            * frequencies) this function should be called after setting the desired use case.
            * @return True if the processing is ready.
            */
            virtual bool isReadyToProcessDepthData() = 0;

            // IRefineExposureTime
            virtual uint32_t getRefinedExposureTime (const uint32_t exposureTime,
                    const royale::Pair<uint32_t, uint32_t> &exposureLimits) override = 0;

            /**
            * Returns information about the current processing (e.g. name and version)
            * @return Vector of key/value pairs with information.
            */
            virtual royale::Vector<royale::Pair<royale::String, royale::String>> getProcessingInfo() = 0;

            /**
            * Checks if the processing needs calibration data to process data.
            * @return This function returns a compile-time property of the processing
            * implementation, not the current state of the implementation.
            * It will return false if this subclass of IProcessing never requires calibration data.
            * It will return true if the implementation uses calibration data, even if the calibration
            * data has been supplied - use isReadyToProcessDepthData() to check the run-time status.
            */
            virtual bool needsCalibrationData() = 0;

            /**
            * Change the exposure mode for the processing.
            *
            * In AUTOMATIC mode the optimum exposure settings will be determined by the processing
            * and new exposure suggestions are delivered via the ExposureListener callback.
            * The default value is MANUAL.
            *
            * @param exposureMode Turn automatic exposure on or off
            * @param streamId The ID of the current stream
            */
            virtual void setExposureMode (royale::ExposureMode exposureMode,
                                          const royale::StreamId streamId) = 0;

            /**
            * Retrieve the current state of the auto exposure of the processing.
            *
            * @param streamId The ID of the current stream
            * @return the current exposure mode
            */
            virtual royale::ExposureMode getExposureMode (const royale::StreamId streamId) = 0;

            /**
            *  Change the level of filtering that is used during the processing.
            *  Please have a look at the royale::FilterLevel enum to see which levels are available.
            */
            virtual void setFilterLevel (royale::FilterLevel level, royale::StreamId streamId) = 0;

            /**
            *  Retrieve the level of filtering that is used during the processing.
            *  Please have a look at the royale::FilterLevel enum to see which levels are available.
            */
            virtual royale::FilterLevel getFilterLevel (royale::StreamId streamId) const = 0;
        };
    }
}
