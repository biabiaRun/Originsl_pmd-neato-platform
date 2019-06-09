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
#include <map>
#include <mutex>
#include <memory>

#include <processing/Processing.hpp>

namespace royale
{
    namespace processing
    {
        class ICalibrationProvider;
        class IParameterProvider;

        /**
         * Class which provides dummy processing, it will calculate a depth image, but using a very
         * simple algorithm. This can, for example, be used to port Royale to different platforms
         * without having to also port Spectre.
         *
         * It will only use the first set of modulated frames, so all the extra data that is in a
         * 9-frame use case (instead of a 5-phase use case) will be ignored by the algorithm.  Any
         * grayscale frames will also be ignored.
         *
         * All calibration data will also be ignored.
         */
        class ProcessingSimple : public Processing
        {
        public:

            /**
             * Ctor
             *
             * @param releaser  capture release
             */
            ROYALE_API explicit ProcessingSimple (royale::collector::IFrameCaptureReleaser *releaser);

            ROYALE_API virtual ~ProcessingSimple();

            ROYALE_API void setUseCase (const royale::usecase::UseCaseDefinition &useCase) override;

            ROYALE_API royale::usecase::VerificationStatus verifyUseCase (const royale::usecase::UseCaseDefinition &useCase) override;

            ROYALE_API bool isReadyToProcessDepthData() override;

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

            virtual void getLensCenterCalibration (uint16_t &centerX, uint16_t &centerY) override;

            virtual void getLensParameters (royale::LensParameters &params) override;

            royale::String getProcessingName() override;
            royale::String getProcessingVersion() override;

        private:

            void calcViewingVectors (const royale::usecase::UseCaseDefinition &useCase);

            std::mutex m_lock;

            std::vector<uint8_t> m_calibrationData;

            /** Image dimension, set from the UseCaseDefinition in setUseCase */
            uint16_t m_currentWidth = 0;
            /** Image dimension, set from the UseCaseDefinition in setUseCase */
            uint16_t m_currentHeight = 0;

            /**
             * For each stream in the use case, the first frame of the first modulation
             * sequence.  The depth will be calculated from this one and the next three frames.
             *
             * This is the index to the vector of frames as received in captureCallback.
             */
            std::map<royale::StreamId, std::size_t> m_firstRawFrame;

            /**
             * For each stream in the use case, a distance corresponding to the modulation
             * frequency of the raw frame set.
             */
            std::map<royale::StreamId, float> m_range;

            /**
             * Conversion of pixels to 3D points.  This assumes a 60 degree viewing angle, and is
             * only dependent on the image size, its valid for every stream in the use case.
             */
            std::vector<float> viewingVectors[3];
        };
    }
}
