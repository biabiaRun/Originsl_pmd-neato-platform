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

#include <royale/Definitions.hpp>
#include <royale/Vector.hpp>
#include <common/IPseudoDataInterpreter.hpp>

#include <chrono>
#include <cstdint>
#include <memory>

namespace royale
{
    namespace collector
    {
        /**
         * This contains the data from capturing a complete UseCaseDefinition, excluding the parts
         * that are in the CapturedRawFrames.
         *
         * The expected lifecycle is that this will be destroyed before the Bridge, so it can hold
         * pointers to the Bridge's members without needing shared_ptrs.  See the documentation for
         * CaptureListener::captureCallback(), this class should not be accessed (except by the
         * destructor) after the final call to CaptureReleaser::releaseCapturedFrames().
         */
        class CapturedUseCase
        {
        public:
            /**
             * Which of C++'s three standard clocks is used for getTimestamp().
             */
            typedef std::chrono::system_clock CLOCK_TYPE;

            ROYALE_API CapturedUseCase (const common::IPseudoDataInterpreter *pdi, float temperature, std::chrono::microseconds timestamp, const royale::Vector<uint32_t> &exposures);
            ROYALE_API ~CapturedUseCase();

            /**
             * Access method for the metadata for CapturedRawFrames.  This should only be used with
             * the CapturedRawFrames that were provided in the same captureCallback() as this
             * CapturedUseCase.
             */
            ROYALE_API const common::IPseudoDataInterpreter &getInterpreter() const;

            /**
             * When the sequence was captured.  The timing offset (or choice of when during the
             * sequence capture the timestamp is generated) is implementation-defined, but will be
             * consistent for a given module/Bridge. The return value is the time in microseconds
             * from time since epoch (1970).
             */
            ROYALE_API const std::chrono::microseconds getTimestamp() const;

            /**
             * Temperature measurement in celcius.
             */
            ROYALE_API const float getIlluminationTemperature() const;

            /**
             * The exposure times that were used for this capture, in microseconds.
             *
             * After calling CameraModule::setExposureTime, some frames may be received using the
             * old settings, before the new settings are used.  This function returns the values
             * that the imager used for the frames in the captureCallback.
             *
             * In other words, this returns what UseCaseDefinition.getExposureTimes() would have
             * returned when the configuration was sent to the imager, even if the application or
             * autoexposure has changed the exposure in the meantime.
             *
             * In mixed mode (and also in non-mixed mode), this vector includes all ExposureGroups
             * in the UseCaseDefinition, regardless of which stream's frames are contained in the
             * current captureCallback.  Therefore any mapping from RawFrameSets to the return value
             * of UseCaseDefinition.getExposureTimes can (and should) also be used for this
             * function's return value.
             *
             * \return a vector where vector.at(i) corresponds to UseCaseDefinition::getExposureTimes().at(i)
             */
            ROYALE_API const royale::Vector<uint32_t> &getExposureTimes() const;

        private:
            const common::IPseudoDataInterpreter *m_pseudoDataInterpreter;
            std::chrono::microseconds m_timestamp;
            const float m_illuminationTemperature;
            const royale::Vector<uint32_t> m_exposureTimes;
        };
    }
}
