/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale/ProcessingFlag.hpp>
#include <processing/ProcessingParameterId.hpp>

namespace royale
{
    // The settings in this file are used by getProcessingParameterMapFactoryRoyale()
    //
    // \todo ROYAL-3353 decide if these are the correct sets of parameters, and update the existing
    // module configs to link to them by id instead of having sets of parameters in each camera
    // family's configs.
    namespace moduleconfig
    {
        /**
         * This id and the associated map are a default set of settings for 9-phase use cases.
         */
        static const royale::processing::ProcessingParameterId CommonId2Frequencies
        {
            royale::processing::ProcessingParameterId::datatype{{'c', 'o', 'm', 'm', 'o', 'n', ' ', 'i', 'd', ' ', '2', ' ', 'f', 'r', 'e', 'q'}}
        };
        static const royale::ProcessingParameterMap CommonProcessingParams2Frequencies
        {
            royale::parameter::stdConsistencyTolerance,
            royale::parameter::stdFlyingPixelsF0,
            royale::parameter::stdFlyingPixelsF1,
            royale::parameter::stdFlyingPixelsFarDist,
            royale::parameter::stdFlyingPixelsNearDist,
            royale::parameter::stdLowerSaturationThreshold,
            royale::parameter::stdUpperSaturationThreshold,
            royale::parameter::stdMPIAmpThreshold,
            royale::parameter::stdMPIDistThreshold,
            royale::parameter::stdMPINoiseDistance,
            royale::parameter::stdNoiseThreshold,
            royale::parameter::stdAdaptiveNoiseFilterType,
            royale::parameter::stdAutoExposureRefValue,
            { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseAutoExposure_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            royale::parameter::stdUseFlagSBI,
            royale::parameter::stdUseSmoothingFilter,
            royale::parameter::stdSmoothingFilterAlpha,
            royale::parameter::stdSmoothingFilterType,
            royale::parameter::stdUseHoleFilling,
            { royale::ProcessingFlag::SpectreProcessingType_Int, royale::Variant (1, 1, 2) },
        };

        /**
         * This id and the associated map are a default set of settings for 5-phase use cases.
         */
        static const royale::processing::ProcessingParameterId CommonId1Frequency
        {
            royale::processing::ProcessingParameterId::datatype {{'c', 'o', 'm', 'm', 'o', 'n', ' ', 'i', 'd', ' ', '1', ' ', 'f', 'r', 'e', 'q'}}
        };
        static const royale::ProcessingParameterMap CommonProcessingParams1Frequency
        {
            royale::parameter::stdConsistencyTolerance,
            royale::parameter::stdFlyingPixelsF0,
            royale::parameter::stdFlyingPixelsF1,
            royale::parameter::stdFlyingPixelsFarDist,
            royale::parameter::stdFlyingPixelsNearDist,
            royale::parameter::stdLowerSaturationThreshold,
            royale::parameter::stdUpperSaturationThreshold,
            royale::parameter::stdMPIAmpThreshold,
            royale::parameter::stdMPIDistThreshold,
            royale::parameter::stdMPINoiseDistance,
            royale::parameter::stdNoiseThreshold,
            royale::parameter::stdAdaptiveNoiseFilterType,
            royale::parameter::stdAutoExposureRefValue,
            { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseAutoExposure_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            royale::parameter::stdUseFlagSBI,
            royale::parameter::stdUseSmoothingFilter,
            royale::parameter::stdSmoothingFilterAlpha,
            royale::parameter::stdSmoothingFilterType,
            royale::parameter::stdUseHoleFilling,
            { royale::ProcessingFlag::SpectreProcessingType_Int, royale::Variant (1, 1, 2) },
        };

        /**
         * This id is a default for mixed modes with the first stream being 9-phase and the second
         * stream being 5-phase. It should be used with a vector containing
         * {CommonProcessingParams2Frequencies, CommonProcessingParams1Frequency} in that order.
         */
        static const royale::processing::ProcessingParameterId CommonIdMixedEsHt
        {
            royale::processing::ProcessingParameterId::datatype {{'c', 'o', 'm', 'm', 'o', 'n', ' ', 'i', 'd', ' ', '2', ' ', '1', ' ', 'f', 'q'}}
        };

        /**
        * This id is a default for mixed modes with the first stream being 5-phase and the second
        * stream being 9-phase. It should be used with a vector containing
        * {CommonProcessingParams1Frequency, CommonProcessingParams2Frequencies} in that order.
        */
        static const royale::processing::ProcessingParameterId CommonIdMixedHtEs
        {
            royale::processing::ProcessingParameterId::datatype{ { 'c', 'o', 'm', 'm', 'o', 'n', ' ', 'i', 'd', ' ', '1', ' ', '2', ' ', 'f', 'q' } }
        };

        /**
        * This id and the associated map are a default set of settings for the "Low Noise Extended" use case.
        */
        static const royale::processing::ProcessingParameterId CommonIdLowNoiseExtended
        {
            royale::processing::ProcessingParameterId::datatype{ { 'L', 'o', 'w', 'N', 'o', 'i', 's', 'e', 'E', 'x', 't', 'e', 'n', 'd', 'e', 'd' } }
        };

        static const royale::ProcessingParameterMap CommonProcessingParamsLowNoiseExtended
        {
            { royale::ProcessingFlag::ConsistencyTolerance_Float, royale::Variant (3.0f, 0.2f, 3.0f) },
            { royale::ProcessingFlag::FlyingPixelsF0_Float, royale::Variant (0.01f, 0.01f, 0.04f) },
            { royale::ProcessingFlag::FlyingPixelsF1_Float, royale::Variant (0.2f, 0.01f, 0.5f) },
            { royale::ProcessingFlag::FlyingPixelsFarDist_Float, royale::Variant (4.5f, 0.01f, 1000.0f) },
            { royale::ProcessingFlag::FlyingPixelsNearDist_Float, royale::Variant (1.0f, 0.01f, 1000.0f) },
            { royale::ProcessingFlag::LowerSaturationThreshold_Int, royale::Variant (400, 0, 600) },
            { royale::ProcessingFlag::UpperSaturationThreshold_Int, royale::Variant (3700, 3500, 4095) },
            { royale::ProcessingFlag::MPIAmpThreshold_Float, royale::Variant (0.2f, 0.1f, 0.5f) },
            { royale::ProcessingFlag::MPIDistThreshold_Float, royale::Variant (0.05f, 0.05f, 0.2f) },
            { royale::ProcessingFlag::MPINoiseDistance_Float, royale::Variant (3.0f, 2.0f, 5.0f) },
            { royale::ProcessingFlag::NoiseThreshold_Float, royale::Variant (0.07f, 0.03f, 0.2f) },
            { royale::ProcessingFlag::AdaptiveNoiseFilterType_Int, royale::Variant (1, 1, 2) },
            { royale::ProcessingFlag::AutoExposureRefValue_Float, 1200.0f },
            { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseAutoExposure_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseSmoothingFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::SmoothingFilterType_Int, royale::Variant (1, 1, 1) },
            { royale::ProcessingFlag::SmoothingAlpha_Float, royale::Variant (0.1f, 0.0f, 1.0f) },
            { royale::ProcessingFlag::UseHoleFilling_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::SpectreProcessingType_Int, royale::Variant (1, 1, 2) },
        };

        /**
        * This id and the associated map are a default set of settings for the "Video Extended" use case.
        */
        static const royale::processing::ProcessingParameterId CommonIdVideoExtended
        {
            royale::processing::ProcessingParameterId::datatype{ { 'V', 'i', 'd', 'e', 'o', 'E', 'x', 't', 'e', 'n', 'd', 'e', 'd', ' ', ' ', ' ' } }
        };

        static const royale::ProcessingParameterMap CommonProcessingParamsVideoExtended
        {
            { royale::ProcessingFlag::ConsistencyTolerance_Float, royale::Variant (1.0f, 0.2f, 3.0f) },
            { royale::ProcessingFlag::FlyingPixelsF0_Float, royale::Variant (0.01f, 0.01f, 0.04f) },
            { royale::ProcessingFlag::FlyingPixelsF1_Float, royale::Variant (0.14f, 0.01f, 0.5f) },
            { royale::ProcessingFlag::FlyingPixelsFarDist_Float, royale::Variant (4.5f, 0.01f, 1000.0f) },
            { royale::ProcessingFlag::FlyingPixelsNearDist_Float, royale::Variant (1.0f, 0.01f, 1000.0f) },
            { royale::ProcessingFlag::LowerSaturationThreshold_Int, royale::Variant (400, 0, 600) },
            { royale::ProcessingFlag::UpperSaturationThreshold_Int, royale::Variant (3700, 3500, 4095) },
            { royale::ProcessingFlag::MPIAmpThreshold_Float, royale::Variant (0.2f, 0.1f, 0.5f) },
            { royale::ProcessingFlag::MPIDistThreshold_Float, royale::Variant (0.05f, 0.05f, 0.2f) },
            { royale::ProcessingFlag::MPINoiseDistance_Float, royale::Variant (3.0f, 2.0f, 5.0f) },
            { royale::ProcessingFlag::NoiseThreshold_Float, royale::Variant (0.07f, 0.03f, 0.2f) },
            { royale::ProcessingFlag::AdaptiveNoiseFilterType_Int, royale::Variant (1, 1, 2) },
            { royale::ProcessingFlag::AutoExposureRefValue_Float, 1200.0f },
            { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseAutoExposure_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseSmoothingFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::SmoothingFilterType_Int, royale::Variant (1, 1, 1) },
            { royale::ProcessingFlag::SmoothingAlpha_Float, royale::Variant (0.15f, 0.0f, 1.0f) },
            { royale::ProcessingFlag::UseHoleFilling_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::SpectreProcessingType_Int, royale::Variant (1, 1, 2) },
        };

        /**
        * This id and the associated map are a default set of settings for the "Video Half" use case.
        */
        static const royale::processing::ProcessingParameterId CommonIdVideoHalf
        {
            royale::processing::ProcessingParameterId::datatype{ { 'V', 'i', 'd', 'e', 'o', 'H', 'a', 'l', 'f', ' ', ' ', ' ', ' ', ' ', ' ', ' ' } }
        };

        static const royale::ProcessingParameterMap CommonProcessingParamsVideoHalf
        {
            { royale::ProcessingFlag::ConsistencyTolerance_Float, royale::Variant (1.0f, 0.2f, 3.0f) },
            { royale::ProcessingFlag::FlyingPixelsF0_Float, royale::Variant (0.01f, 0.01f, 0.04f) },
            { royale::ProcessingFlag::FlyingPixelsF1_Float, royale::Variant (0.2f, 0.01f, 0.5f) },
            { royale::ProcessingFlag::FlyingPixelsFarDist_Float, royale::Variant (4.5f, 0.01f, 1000.0f) },
            { royale::ProcessingFlag::FlyingPixelsNearDist_Float, royale::Variant (1.0f, 0.01f, 1000.0f) },
            { royale::ProcessingFlag::LowerSaturationThreshold_Int, royale::Variant (400, 0, 600) },
            { royale::ProcessingFlag::UpperSaturationThreshold_Int, royale::Variant (3700, 3500, 4095) },
            { royale::ProcessingFlag::MPIAmpThreshold_Float, royale::Variant (0.2f, 0.1f, 0.5f) },
            { royale::ProcessingFlag::MPIDistThreshold_Float, royale::Variant (0.05f, 0.05f, 0.2f) },
            { royale::ProcessingFlag::MPINoiseDistance_Float, royale::Variant (3.0f, 2.0f, 5.0f) },
            { royale::ProcessingFlag::NoiseThreshold_Float, royale::Variant (0.07f, 0.03f, 0.2f) },
            { royale::ProcessingFlag::AdaptiveNoiseFilterType_Int, royale::Variant (1, 1, 2) },
            { royale::ProcessingFlag::AutoExposureRefValue_Float, 1200.0f },
            { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseAutoExposure_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseSmoothingFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::SmoothingFilterType_Int, royale::Variant (1, 1, 1) },
            { royale::ProcessingFlag::SmoothingAlpha_Float, royale::Variant (0.2f, 0.0f, 1.0f) },
            { royale::ProcessingFlag::UseHoleFilling_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::SpectreProcessingType_Int, royale::Variant (1, 1, 2) },
        };

        /**
        * This id and the associated map are a default set of settings for the "Video" use case.
        */
        static const royale::processing::ProcessingParameterId CommonIdVideo
        {
            royale::processing::ProcessingParameterId::datatype{ { 'V', 'i', 'd', 'e', 'o', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' } }
        };

        static const royale::ProcessingParameterMap CommonProcessingParamsVideo
        {
            { royale::ProcessingFlag::ConsistencyTolerance_Float, royale::Variant (1.0f, 0.2f, 3.0f) },
            { royale::ProcessingFlag::FlyingPixelsF0_Float, royale::Variant (0.01f, 0.01f, 0.04f) },
            { royale::ProcessingFlag::FlyingPixelsF1_Float, royale::Variant (0.2f, 0.01f, 0.5f) },
            { royale::ProcessingFlag::FlyingPixelsFarDist_Float, royale::Variant (4.5f, 0.01f, 1000.0f) },
            { royale::ProcessingFlag::FlyingPixelsNearDist_Float, royale::Variant (1.0f, 0.01f, 1000.0f) },
            { royale::ProcessingFlag::LowerSaturationThreshold_Int, royale::Variant (400, 0, 600) },
            { royale::ProcessingFlag::UpperSaturationThreshold_Int, royale::Variant (3700, 3500, 4095) },
            { royale::ProcessingFlag::MPIAmpThreshold_Float, royale::Variant (0.2f, 0.1f, 0.5f) },
            { royale::ProcessingFlag::MPIDistThreshold_Float, royale::Variant (0.05f, 0.05f, 0.2f) },
            { royale::ProcessingFlag::MPINoiseDistance_Float, royale::Variant (3.0f, 2.0f, 5.0f) },
            { royale::ProcessingFlag::NoiseThreshold_Float, royale::Variant (0.07f, 0.03f, 0.2f) },
            { royale::ProcessingFlag::AdaptiveNoiseFilterType_Int, royale::Variant (1, 1, 2) },
            { royale::ProcessingFlag::AutoExposureRefValue_Float, 1200.0f },
            { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseAutoExposure_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseSmoothingFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::SmoothingFilterType_Int, royale::Variant (1, 1, 1) },
            { royale::ProcessingFlag::SmoothingAlpha_Float, royale::Variant (0.15f, 0.0f, 1.0f) },
            { royale::ProcessingFlag::UseHoleFilling_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::SpectreProcessingType_Int, royale::Variant (1, 1, 2) },
        };

        /**
        * This id and the associated map are a default set of settings for the "Fast Acquisition" use case.
        */
        static const royale::processing::ProcessingParameterId CommonIdFastAcquisition
        {
            royale::processing::ProcessingParameterId::datatype{ { 'F', 'a', 's', 't', 'A', 'c', 'q', 'u', 'i', 's', 'i', 't', 'i', 'o', 'n', ' ' } }
        };

        static const royale::ProcessingParameterMap CommonProcessingParamsFastAcquisition
        {
            { royale::ProcessingFlag::ConsistencyTolerance_Float, royale::Variant (1.0f, 0.2f, 3.0f) },
            { royale::ProcessingFlag::FlyingPixelsF0_Float, royale::Variant (0.01f, 0.01f, 0.04f) },
            { royale::ProcessingFlag::FlyingPixelsF1_Float, royale::Variant (0.2f, 0.01f, 0.5f) },
            { royale::ProcessingFlag::FlyingPixelsFarDist_Float, royale::Variant (4.5f, 0.01f, 1000.0f) },
            { royale::ProcessingFlag::FlyingPixelsNearDist_Float, royale::Variant (1.0f, 0.01f, 1000.0f) },
            { royale::ProcessingFlag::LowerSaturationThreshold_Int, royale::Variant (400, 0, 600) },
            { royale::ProcessingFlag::UpperSaturationThreshold_Int, royale::Variant (3700, 3500, 4095) },
            { royale::ProcessingFlag::MPIAmpThreshold_Float, royale::Variant (0.2f, 0.1f, 0.5f) },
            { royale::ProcessingFlag::MPIDistThreshold_Float, royale::Variant (0.05f, 0.05f, 0.2f) },
            { royale::ProcessingFlag::MPINoiseDistance_Float, royale::Variant (3.0f, 2.0f, 5.0f) },
            { royale::ProcessingFlag::NoiseThreshold_Float, royale::Variant (0.07f, 0.03f, 0.2f) },
            { royale::ProcessingFlag::AdaptiveNoiseFilterType_Int, royale::Variant (1, 1, 2) },
            { royale::ProcessingFlag::AutoExposureRefValue_Float, 1200.0f },
            { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseAutoExposure_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseSmoothingFilter_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::SmoothingFilterType_Int, royale::Variant (1, 1, 1) },
            { royale::ProcessingFlag::SmoothingAlpha_Float, royale::Variant (1.0f, 0.0f, 1.0f) },
            { royale::ProcessingFlag::UseHoleFilling_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::SpectreProcessingType_Int, royale::Variant (1, 1, 2) },
        };

        /**
        * This id and the associated map are a default set of settings for the "Very Fast Acquisition" use case.
        */
        static const royale::processing::ProcessingParameterId CommonIdVeryFastAcquisition
        {
            royale::processing::ProcessingParameterId::datatype{ { 'V', 'F', 'a', 's', 't', 'A', 'c', 'q', 'u', 'i', 's', 'i', 't', 'i', 'o', 'n' } }
        };

        static const royale::ProcessingParameterMap CommonProcessingParamsVeryFastAcquisition
        {
            { royale::ProcessingFlag::ConsistencyTolerance_Float, royale::Variant (1.0f, 0.2f, 3.0f) },
            { royale::ProcessingFlag::FlyingPixelsF0_Float, royale::Variant (0.01f, 0.01f, 0.04f) },
            { royale::ProcessingFlag::FlyingPixelsF1_Float, royale::Variant (0.2f, 0.01f, 0.5f) },
            { royale::ProcessingFlag::FlyingPixelsFarDist_Float, royale::Variant (4.5f, 0.01f, 1000.0f) },
            { royale::ProcessingFlag::FlyingPixelsNearDist_Float, royale::Variant (1.0f, 0.01f, 1000.0f) },
            { royale::ProcessingFlag::LowerSaturationThreshold_Int, royale::Variant (400, 0, 600) },
            { royale::ProcessingFlag::UpperSaturationThreshold_Int, royale::Variant (3700, 3500, 4095) },
            { royale::ProcessingFlag::MPIAmpThreshold_Float, royale::Variant (0.2f, 0.1f, 0.5f) },
            { royale::ProcessingFlag::MPIDistThreshold_Float, royale::Variant (0.05f, 0.05f, 0.2f) },
            { royale::ProcessingFlag::MPINoiseDistance_Float, royale::Variant (3.0f, 2.0f, 5.0f) },
            { royale::ProcessingFlag::NoiseThreshold_Float, royale::Variant (0.07f, 0.03f, 0.2f) },
            { royale::ProcessingFlag::AdaptiveNoiseFilterType_Int, royale::Variant (1, 1, 2) },
            { royale::ProcessingFlag::AutoExposureRefValue_Float, 1200.0f },
            { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseAutoExposure_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseSmoothingFilter_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::SmoothingFilterType_Int, royale::Variant (1, 1, 1) },
            { royale::ProcessingFlag::SmoothingAlpha_Float, royale::Variant (1.0f, 0.0f, 1.0f) },
            { royale::ProcessingFlag::UseHoleFilling_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::SpectreProcessingType_Int, royale::Variant (1, 1, 2) },
        };


        // For OZT167

        static const royale::processing::ProcessingParameterId CommonIdVideoExtendedCBF
        {
            royale::processing::ProcessingParameterId::datatype{ { 'V', 'i', 'd', 'e', 'o', 'E', 'x', 't', 'e', 'n', 'd', 'e', 'd', 'C', 'B', 'F' } }
        };

        static const royale::processing::ProcessingParameterId CommonIdVideoExtendedNG
        {
            royale::processing::ProcessingParameterId::datatype{ { 'V', 'i', 'd', 'e', 'o', 'E', 'x', 't', 'e', 'n', 'd', 'e', 'd', 'N', 'G', ' ' } }
        };

        static const royale::ProcessingParameterMap CommonProcessingParamsVideoExtendedNG
        {
            { royale::ProcessingFlag::LowerSaturationThreshold_Int, royale::Variant (400, 0, 600) },
            { royale::ProcessingFlag::UpperSaturationThreshold_Int, royale::Variant (3750, 3500, 4095) },
            { royale::ProcessingFlag::AutoExposureRefValue_Float, royale::Variant (1000.0f, 1.0f, 4095.0f) },
            { royale::ProcessingFlag::SmoothingAlpha_Float, royale::Variant (0.25f, 0.0f, 1.0f) },
            { royale::ProcessingFlag::AutoExpoMin_Int, royale::Variant (1, 1, 50000) },
            { royale::ProcessingFlag::AutoExpoMax_Int, royale::Variant (1200, 1, 50000) },
            { royale::ProcessingFlag::SpectreProcessingType_Int, royale::Variant (3, 1, 5) },
            { royale::ProcessingFlag::NoiseFilterSigmaD_Float, royale::Variant (2.0f, 1.0f, 5.0f) },
            { royale::ProcessingFlag::NoiseFilterIterations_Int, royale::Variant (3, 1, 10) },
            { royale::ProcessingFlag::FlyingPixelAngleLimit_Float, royale::Variant (0.17f, 0.006f, 0.5f) },
            { royale::ProcessingFlag::FlyingPixelAmpThreshold_Float, royale::Variant (1.0f, 0.0f, 64.0f) },
            { royale::ProcessingFlag::FlyingPixelMinNeighbors_Int, royale::Variant (3, 0, 8) },
            { royale::ProcessingFlag::FlyingPixelMaxNeighbors_Int, royale::Variant (6, 0, 8) },
            { royale::ProcessingFlag::FlyingPixelNoiseRatioThresh_Float, royale::Variant (10.5f, 0.0f, 128.0f) },
            { royale::ProcessingFlag::SmoothingFilterResetThreshold_Float, royale::Variant (8.0f, 0.0f, 64.0f) },
            { royale::ProcessingFlag::CCThresh_Int, royale::Variant (42, 0, 16384) },
            { royale::ProcessingFlag::PhaseNoiseThresh_Float, royale::Variant (0.0f, 0.0f, 10.0f) },
            { royale::ProcessingFlag::UseAutoExposure_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::StraylightThreshold_Float, royale::Variant (4.0f, 0.0f, 10.0f) },
        };

        static const royale::processing::ProcessingParameterId CommonIdVideoExtendedNGB
        {
            royale::processing::ProcessingParameterId::datatype{ { 'V', 'i', 'd', 'e', 'o', 'E', 'x', 't', 'e', 'n', 'd', 'e', 'd', 'N', 'G', 'B' } }
        };

        static const royale::processing::ProcessingParameterId CommonIdVideoHalfCBF
        {
            royale::processing::ProcessingParameterId::datatype{ { 'V', 'i', 'd', 'e', 'o', 'H', 'a', 'l', 'f', 'C', 'B', 'F', ' ', ' ', ' ', ' ' } }
        };

        static const royale::processing::ProcessingParameterId CommonIdVideoHalfNG
        {
            royale::processing::ProcessingParameterId::datatype{ { 'V', 'i', 'd', 'e', 'o', 'H', 'a', 'l', 'f', 'N', 'G', ' ', ' ', ' ', ' ', ' ' } }
        };

        static const royale::ProcessingParameterMap CommonProcessingParamsVideoHalfNG
        {
            { royale::ProcessingFlag::LowerSaturationThreshold_Int, royale::Variant (400, 0, 600) },
            { royale::ProcessingFlag::UpperSaturationThreshold_Int, royale::Variant (3750, 3500, 4095) },
            { royale::ProcessingFlag::AutoExposureRefValue_Float, royale::Variant (1000.0f, 1.0f, 4095.0f) },
            { royale::ProcessingFlag::SmoothingAlpha_Float, royale::Variant (0.25f, 0.0f, 1.0f) },
            { royale::ProcessingFlag::AutoExpoMin_Int, royale::Variant (1, 1, 50000) },
            { royale::ProcessingFlag::AutoExpoMax_Int, royale::Variant (1200, 1, 50000) },
            { royale::ProcessingFlag::SpectreProcessingType_Int, royale::Variant (3, 1, 5) },
            { royale::ProcessingFlag::NoiseFilterSigmaD_Float, royale::Variant (2.0f, 1.0f, 5.0f) },
            { royale::ProcessingFlag::NoiseFilterIterations_Int, royale::Variant (3, 1, 10) },
            { royale::ProcessingFlag::FlyingPixelAngleLimit_Float, royale::Variant (0.17f, 0.006f, 0.5f) },
            { royale::ProcessingFlag::FlyingPixelAmpThreshold_Float, royale::Variant (1.0f, 0.0f, 64.0f) },
            { royale::ProcessingFlag::FlyingPixelMinNeighbors_Int, royale::Variant (3, 0, 8) },
            { royale::ProcessingFlag::FlyingPixelMaxNeighbors_Int, royale::Variant (6, 0, 8) },
            { royale::ProcessingFlag::FlyingPixelNoiseRatioThresh_Float, royale::Variant (10.5f, 0.0f, 128.0f) },
            { royale::ProcessingFlag::SmoothingFilterResetThreshold_Float, royale::Variant (8.0f, 0.0f, 64.0f) },
            { royale::ProcessingFlag::CCThresh_Int, royale::Variant (42, 0, 16384) },
            { royale::ProcessingFlag::PhaseNoiseThresh_Float, royale::Variant (0.0f, 0.0f, 10.0f) },
        };

        static const royale::processing::ProcessingParameterId CommonIdVideoHalfNGB
        {
            royale::processing::ProcessingParameterId::datatype{ { 'V', 'i', 'd', 'e', 'o', 'H', 'a', 'l', 'f', 'N', 'G', 'B', ' ', ' ', ' ', ' ' } }
        };

        static const royale::processing::ProcessingParameterId CommonIdAF
        {
            royale::processing::ProcessingParameterId::datatype{ { 'A', 'F', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' } }
        };

        static const royale::processing::ProcessingParameterId CommonIdAR
        {
            royale::processing::ProcessingParameterId::datatype{ { 'A', 'R', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' } }
        };

        static const royale::processing::ProcessingParameterId CommonIdIR
        {
            royale::processing::ProcessingParameterId::datatype{ { 'I', 'R', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' } }
        };

        static const royale::ProcessingParameterMap CommonProcessingParamsIR
        {
            { royale::ProcessingFlag::SpectreProcessingType_Int, royale::Variant (6, 6, 6) },
            { royale::ProcessingFlag::UseGrayImageFallbackAmplitude_Bool, royale::Variant (false) },
        };

        static const royale::processing::ProcessingParameterId CommonIdFaceID
        {
            royale::processing::ProcessingParameterId::datatype{ { 'F', 'a', 'c', 'e', 'I', 'D', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' } }
        };
    }
}
