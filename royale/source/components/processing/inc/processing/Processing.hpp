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

#include <cstdint>
#include <memory>
#include <vector>
#include <mutex>

#include <collector/IFrameCaptureReleaser.hpp>
#include <processing/IProcessing.hpp>

#include <royale/IExposureListener2.hpp>
#include <processing/ExtendedData.hpp>
#include <royale/ProcessingFlag.hpp>
#include <royale/LensParameters.hpp>
#include <royale/String.hpp>

namespace royale
{
    namespace processing
    {
        /**
        * This contains the base class for the processing.
        */
        class Processing : public IProcessing
        {
        public:

            Processing (royale::collector::IFrameCaptureReleaser *releaser,
                        royale::processing::IRefineExposureTime *refineExposureTime = nullptr);
            virtual ~Processing();

            // IProcessing interface
            void registerDataListeners (const DataListeners &listeners) override;
            void registerExposureListener (royale::IExposureListener2 *exposureListener) override;
            void unregisterExposureListener () override;
            royale::usecase::VerificationStatus verifyUseCase (const royale::usecase::UseCaseDefinition &useCase) override;
            bool hasLensCenterCalibration() const override;
            void setProcessingParameters (const royale::ProcessingParameterMap &parameters,
                                          const royale::StreamId streamId) override;
            void getProcessingParameters (royale::ProcessingParameterMap &parameters,
                                          const royale::StreamId streamId) override;
            void setUseCase (const royale::usecase::UseCaseDefinition &useCase) override;
            void setCameraName (const royale::String &cameraName) override;

            /**
            * Capture callback called by the access module.
            * @param frames Captured raw frames
            * @param definition Use case definition
            * @param streamId Which sub use case has been captured
            * @param capturedCase Use case used for capturing the images
            */
            void captureCallback (std::vector<royale::common::ICapturedRawFrame *> &frames,
                                  const royale::usecase::UseCaseDefinition &definition,
                                  royale::StreamId streamId,
                                  std::unique_ptr<const royale::collector::CapturedUseCase> capturedCase) override;

            void releaseAllFrames () override;


            uint32_t getRefinedExposureTime (const uint32_t exposureTime,
                                             const royale::Pair<uint32_t, uint32_t> &exposureLimits) override;

            royale::Vector<royale::Pair<royale::String, royale::String>> getProcessingInfo() override;

        protected:

            /**
            * Struct to keep track which DepthData object is still is use
            */
            struct DepthDataItem
            {
                DepthDataItem ()
                {
                    depthData.reset (new royale::DepthData());
                    rawData.reset (new royale::RawData());
                    intermediateData.reset (new royale::IntermediateData());
                    depthImage.reset (new royale::DepthImage());
                    sparsePointCloud.reset (new royale::SparsePointCloud());
                    irImage.reset (new royale::IRImage());
                    extendedData.reset (new royale::processing::ExtendedData());
                }

                ~DepthDataItem()
                {
                }

                std::shared_ptr<royale::DepthData> depthData;
                std::shared_ptr<royale::RawData> rawData;
                std::shared_ptr<royale::IntermediateData> intermediateData;
                std::shared_ptr<royale::DepthImage> depthImage;
                std::shared_ptr<royale::SparsePointCloud> sparsePointCloud;
                std::shared_ptr<royale::IRImage> irImage;
                std::shared_ptr<royale::processing::ExtendedData> extendedData;
                royale::StreamId streamId;
            };

            /**
            * Result buffer
            */
            DepthDataItem m_depthDataBuffer;

            /**
            * Processing function which has to be reimplemented.
            * @param frames Captured raw frames
            * @param capturedCase Use case used for capturing the images
            * @param depthData Depth data struct which should be filled
            * @param capturedTimes Only the exposure times relevant for this stream
            * @param newExposureTimes New exposure times returned from the auto exposure
            */
            virtual void processFrame (std::vector<royale::common::ICapturedRawFrame *> &frames,
                                       std::unique_ptr<const royale::collector::CapturedUseCase> capturedCase,
                                       const DepthDataItem &depthData,
                                       const royale::Vector<uint32_t> &capturedTimes,
                                       std::vector<uint32_t> &newExposureTimes) = 0;

            /**
            * Listeners
            */
            DataListeners m_listeners;

            /**
            * ExposureListener queue.
            */
            royale::IExposureListener2 *m_exposureListener {nullptr};

            /**
            * Releaser used to release the source frames
            */
            royale::collector::IFrameCaptureReleaser *m_releaser;

            /**
            * The current processing parameters
            */
            std::map<royale::StreamId, royale::ProcessingParameterMap> m_parameters;

            /**
            * The current camera name
            */
            std::string m_cameraName;

            /*!
             * Protects the access to the data listeners during capturing
             */
            std::mutex m_listenerMutex;

            /*!
            * Index of the relevant exposure time (index of the first modulated sequence).
            * This depends on the phase order. The index can be applied to vectors ordered
            * according to the raw frame set order (e.g. m_newExposureTimesQueue).
            */
            std::map<royale::StreamId, uint32_t> m_exposureTimeIndex;

            std::mutex m_calcMutex;

            royale::processing::IRefineExposureTime *m_refineExposureTime;

            std::map<royale::StreamId, royale::Pair<uint32_t, uint32_t>> m_exposureLimits;

            /**
            * Returns the name of the current processing.
            * @return royale string with the name of the processing.
            */
            virtual royale::String getProcessingName() = 0;

            /**
            * Returns the version of the current processing.
            * @return royale string with the version of the processing.
            */
            virtual royale::String getProcessingVersion() = 0;
        };
    }
}
