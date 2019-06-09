/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#ifndef __INPUT_HPP__
#define __INPUT_HPP__

#include "common/Field.hpp"
#include "common/Utils.hpp"
#include "spectre/Parameter.hpp"

#include <cstdint>
#include <type_traits>

namespace spectre
{
    /// Maximum number of modulated measurements supported by Spectre
#define SPECTRE_MAX_4_PHASE_RAW_FRAME_SETS 2

    /// Number of modulated frames per measurements expected by Spectre
#define SPECTRE_FRAMES_PER_MOD_MEASUREMENTS 4


    /**
     * @brief Container class holding all data from a raw super frame.
     *
     * Spectre needs several raw frames to calculate a single depth image. These raw frames,
     * and associated meta-data (e.g., exposure times) are stored by the Input class.
     * Depending on the use case the input class might hold one 4-phase raw frame set
     * (consisting of 4 raw frames) or two 4-phase raw frame set (consisting of 2*4 raw frames).
     * Additionally, Spectre always requires an intensity frame.
     *
     * The class can be instantiated with an ArrayReference<uint16_t>, or an ArrayHolder<uint16_t>
     * as template parameter. If ArrayReference<uint16_t> is used, the caller is responsible to ensure
     * that the referenced memory locations are valid while the Input instance is used. The Input class
     * will not copy any raw data by itself (but in case of ArrayHolder<uint16_t> the holder will take
     * care of this).
     */
    template<template<typename> class T, int MAX_MOD_GROUPS = SPECTRE_MAX_4_PHASE_RAW_FRAME_SETS>
    class Input
    {
    public:
        /**
         * @brief Creates a new Input instance.
         */
        Input ()
        {
            for (int i = 0; i < MAX_MOD_GROUPS; i++)
            {
                m_modulatedGroups[i].exposureTime = 0;
            }
        }

        /**
         * @brief Sets the temperature
         *
         * @param temp temperature
         *
         * @return current instance of Input
         */
        inline Input &setTemperature (const float temp)
        {
            m_temperature = temp;
            return *this;
        }

        /**
         * @brief Sets the intensity frame and its exposure time
         *
         *
         * @param intensityFrame raw data of intensity frame
         * @param exposureTime exposure time
         *
         * @return current instance of Input
         */
        inline Input &setIntensityFrame (const T<uint16_t> &intensityFrame, unsigned exposureTime)
        {
            m_intensity = intensityFrame;
            m_intensityExposure = exposureTime;
            return *this;
        }

        /**
         * @brief Sets the data for the modulated frames acquired for one modulation frequency (4-phase raw frame set).
         *
         * The frames should be passed in the order of the expected frame order, which is 0, 90, 180, 270 degrees
         * by default in Spectre.
         *
         * Note: Only ParameterKey::FREQUENCY_i (i = 1,...,SPECTRE_MAX_4_PHASE_RAW_FRAME_SETS) is a valid
         * value for the template parameter P. It is used to match the 4-phase raw frame set to the
         * frequency used in the configuration of ISpectre.
         *
         * @param P frequency identifier
         * @param frame1 first modulated frame
         * @param frame2 second modulated frame
         * @param frame3 third modulated frame
         * @param frame4 fourth modulated frame
         * @param exposureTime exposure time used to capture the frames
         *
         * @return current instance of Input
         */
        template<common::ParameterKey P>
        inline IsFrequencyParameter<P, Input &> setModulatedFrames (const T<uint16_t> &frame1,
                const T<uint16_t> &frame2,
                const T<uint16_t> &frame3,
                const T<uint16_t> &frame4,
                unsigned exposureTime)
        {
            auto modFreqIdx = keyToIdx (P);
            m_modulatedGroups[modFreqIdx].fields[0] = frame1;
            m_modulatedGroups[modFreqIdx].fields[1] = frame2;
            m_modulatedGroups[modFreqIdx].fields[2] = frame3;
            m_modulatedGroups[modFreqIdx].fields[3] = frame4;
            m_modulatedGroups[modFreqIdx].exposureTime = exposureTime;
            return *this;
        }

        /**
        * @brief Gets the temperature
        *
        *
        * @return temperature
        */
        inline float getTemperature() const
        {
            return m_temperature;
        }

        /**
         * @brief Gets the exposure time set for intensity frame.
         *
         *
         * @return exposure time of intensity frame
         */
        inline unsigned getIntensityExposure() const
        {
            return m_intensityExposure;
        }


        /**
         * @brief Gets a reference to the raw data of the intensity frame
         *
         *
         * @return raw data of intensity frame
         */
        inline const common::AbstractField<uint16_t> &getIntensityFrame() const
        {
            return m_intensity;
        }

        /**
         * @brief Gets the exposure time of the frames acquired for the modulation frequency designated by P
         *
         * Note: Only ParameterKey::FREQUENCY_i (i = 1,...,SPECTRE_MAX_4_PHASE_RAW_FRAME_SETS) is a valid
         * value for the template parameter P
         *
         * @param P frequency identifier
         *
         * @return exposure time
         */
        template<common::ParameterKey P>
        inline IsFrequencyParameter<P, unsigned> getModulatedExposure () const
        {
            auto modFreqIdx = keyToIdx (P);
            return m_modulatedGroups[modFreqIdx].exposureTime;
        }

        /**
         * @brief Gets a raw frame from a 4-phase raw frame set
         *
         * Note: Only ParameterKey::FREQUENCY_i (i = 1,...,SPECTRE_MAX_4_PHASE_RAW_FRAME_SETS) is a valid
         * value for the template parameter P
         *
         * @param P frequency identifier
         * @param frameIdx frame index
         *
         * @return reference to raw data
         */
        template<common::ParameterKey P>
        inline IsFrequencyParameter<P, const common::AbstractField<uint16_t> &>getModulatedFrame (unsigned frameIdx) const
        {
            SPECTRE_ASSERT (frameIdx < SPECTRE_FRAMES_PER_MOD_MEASUREMENTS, "Invalid frame index. Frame index must be less "
                            " than SPECTRE_FRAMES_PER_MOD_MEASUREMENTS.");
            auto modFreqIdx = keyToIdx (P);
            return m_modulatedGroups[modFreqIdx].fields[frameIdx];
        }

        // Not supported operations
        Input &operator= (const Input &) = delete;
        Input (const Input &) = delete;
        Input (Input &&) = delete;

    private:
        inline size_t keyToIdx (common::ParameterKey p) const
        {
            return p == common::ParameterKey::FREQUENCY_1 ? 0u : 1u;
        }

    private:
        float m_temperature;
        unsigned m_intensityExposure;
        T<uint16_t> m_intensity;

        struct
        {
            T<uint16_t> fields[SPECTRE_FRAMES_PER_MOD_MEASUREMENTS];
            unsigned exposureTime;
        } m_modulatedGroups[MAX_MOD_GROUPS];

        static_assert (std::is_base_of<common::AbstractField<uint16_t>, T<uint16_t> >::value, "Template parameter must be derivied "
                       "from AbstractField<uint16_t>.");
    };
}

#endif /*__INPUT_HPP__*/
