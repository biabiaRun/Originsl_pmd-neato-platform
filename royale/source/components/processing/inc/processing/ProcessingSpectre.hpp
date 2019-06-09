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

#include <exception>
#include <mutex>
#include <memory>
#include <array>

#include <processing/Processing.hpp>
#include <usecase/HardcodedMaxStreams.hpp>

#include <spectre/ISpectre.hpp>


namespace royale
{
    namespace processing
    {
        class ICalibrationProvider;

        /**
        * Class which provides processing with the pmd processing chain.
        */
        class ProcessingSpectre : public Processing
        {
        public:

            /**
             * Ctor
             *
             * @param releaser  capture release
             */
            ROYALE_API explicit ProcessingSpectre (royale::collector::IFrameCaptureReleaser *releaser);

            virtual ~ProcessingSpectre();

            ROYALE_API void setUseCase (const royale::usecase::UseCaseDefinition &useCase) override;

            ROYALE_API royale::usecase::VerificationStatus verifyUseCase (const royale::usecase::UseCaseDefinition &useCase) override;

            ROYALE_API bool isReadyToProcessDepthData() override;
            ROYALE_API void getProcessingParameters (royale::ProcessingParameterMap &parameters,
                    const royale::StreamId streamId) override;

            ROYALE_API void setProcessingParameters (const ProcessingParameterMap &parameters,
                    const royale::StreamId streamId) override;

            ROYALE_API bool needsCalibrationData() override;

            ROYALE_API void setExposureMode (royale::ExposureMode exposureMode,
                                             const royale::StreamId streamId) override;
            ROYALE_API royale::ExposureMode getExposureMode (const royale::StreamId streamId) override;

            ROYALE_API void setFilterLevel (royale::FilterLevel level, royale::StreamId streamId) override;
            ROYALE_API royale::FilterLevel getFilterLevel (royale::StreamId streamId) const override;

        protected:

            virtual void setCalibrationData (const std::vector<uint8_t> &calibrationData) override;

            virtual const std::vector<uint8_t> &getCalibrationData() const override;

            virtual bool hasCalibrationData() const override;

            void processFrame (std::vector<royale::common::ICapturedRawFrame *> &frames,
                               std::unique_ptr<const royale::collector::CapturedUseCase> capturedCase,
                               const DepthDataItem &depthData,
                               const royale::Vector<uint32_t> &capturedTimes,
                               std::vector<uint32_t> &newExposureTimes) override;

            virtual bool hasLensCenterCalibration() const override;

            virtual void getLensCenterCalibration (uint16_t &centerX, uint16_t &centerY) override;

            virtual void getLensParameters (royale::LensParameters &params) override;

            /**
            * Fills the output struct.
            * @param target Depth data output
            * @param streamId ID of the current stream
            */
            void prepareDepthDataOutput (royale::DepthData *target, const royale::StreamId streamId);

            void doPrepareDepthImage (const spectre::common::ArrayReference<float> &outCoord3d,
                                      const spectre::common::ArrayReference<float> &outNoise,
                                      const float noiseThreshold,
                                      const spectre::common::ArrayReference<uint32_t> &outFlags,
                                      royale::DepthImage *target, const royale::StreamId streamId);

            void prepareDepthImage (royale::DepthImage *target, const royale::StreamId streamId);
            void prepareSparsePointCloud (royale::SparsePointCloud *target, const royale::StreamId streamId);
            void doPrepareIRImage (const spectre::common::ArrayReference<float> &outAmplitude, royale::IRImage *target);
            void prepareIRImage (royale::IRImage *target, const royale::StreamId streamId);

            /**
            * Fills the intermediate output struct.
            * @param intermediateData Intermediate data output
            * @param streamId ID of the current stream
            */
            void prepareIntermediateOutput (royale::IntermediateData *intermediateData, const royale::StreamId streamId);

            royale::String getProcessingName() override;
            royale::String getProcessingVersion() override;

        private:
            /**
             * @brief Combines a Spectre instance with meta information
             */
            struct SpectreInstanceInfo
            {
                SpectreInstanceInfo()
                    : used (false) {}
                /// Spectre instance
                std::unique_ptr<spectre::ISpectre> spectre;
                /// StreamId for which the instance was configured
                StreamId streamId;
                /// Indicates if the instance is used for the current use case
                bool used;
                /**
                 * @brief Indices of ICapturedRawFrame in processFrame
                 *
                 * frameIndices[0] -> intensity image to use for processing
                 * frameIndices[1] -> start of first 4 phase raw frame set (if any)
                 * frameIndices[2] -> start of second 4 phase raw frame set (if any)
                 */
                std::vector<size_t> frameIndices;

                /**
                 * @brief Indices of exposure times in processFrame
                 *
                 * @see SpectreInstanceInfo::frameIndices
                 */
                std::vector<size_t> exposureIndices;
            };

            void prepareSpectre (const royale::usecase::UseCaseDefinition &useCase,
                                 const royale::StreamId streamId, SpectreInstanceInfo &info);

            bool setClearOutput();

            royale::usecase::VerificationStatus verifyRawFrameSet (const royale::usecase::RawFrameSet &rawFrameSet);

            void precalcLogLUT();

            void createDefaultParameterSets();

            inline uint8_t amplitudeToIRValue (float a);

            inline bool useValidateImageActivated (const royale::StreamId streamId);

            /**
             * @brief Gets the SpectreInstanceInfo associated to the given StreamId.
             *
             * If no Spectre is associated to the stream an exception will be raised.
             *
             * @param streamId stream id
             *
             * @return spectre instance
             */
            SpectreInstanceInfo &getSpectreForStream (const royale::StreamId &streamId);

            /**
             * Create spectre instances for this use case.
             * Must be called with m_lock held!
             */
            void activateUseCase (const royale::usecase::UseCaseDefinition &useCase);

        private:
            std::chrono::microseconds m_timeStamp;
            std::vector<uint8_t> m_calibrationData;

            // Lock for the spectre instances
            std::recursive_mutex m_lock;

            std::array<SpectreInstanceInfo, ROYALE_USECASE_MAX_STREAMS> m_spectres;

            std::shared_ptr<royale::usecase::UseCaseDefinition> m_currentUseCase;

            std::vector<uint8_t> m_logLUT;
            // Maximum amplitude that should be used for the LUT
            const float m_ampMax = 2400.0f;
            // Size of the look up table for the log representation
            // of the IR image
            const uint32_t m_lutSize = 4096;
            float m_ampMult;
            float m_scalingFactor;
            bool m_isReady;
        };
    }
}
