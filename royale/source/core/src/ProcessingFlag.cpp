/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <royale/ProcessingFlag.hpp>

#include <string>
#include <map>

using namespace royale;

namespace
{
    const std::map<ProcessingFlag, const String> processingFlagNames =
    {
        {
            { ProcessingFlag::ConsistencyTolerance_Float, "ConsistencyTolerance_Float" },
            { ProcessingFlag::FlyingPixelsF0_Float, "FlyingPixelsF0_Float" },
            { ProcessingFlag::FlyingPixelsF1_Float, "FlyingPixelsF1_Float" },
            { ProcessingFlag::FlyingPixelsFarDist_Float, "FlyingPixelsFarDist_Float" },
            { ProcessingFlag::FlyingPixelsNearDist_Float, "FlyingPixelsNearDist_Float" },
            { ProcessingFlag::LowerSaturationThreshold_Int, "LowerSaturationThreshold_Int" },
            { ProcessingFlag::UpperSaturationThreshold_Int, "UpperSaturationThreshold_Int" },
            { ProcessingFlag::MPIAmpThreshold_Float, "MPIAmpThreshold_Float" },
            { ProcessingFlag::MPIDistThreshold_Float, "MPIDistThreshold_Float" },
            { ProcessingFlag::MPINoiseDistance_Float, "MPINoiseDistance_Float" },
            { ProcessingFlag::NoiseThreshold_Float, "NoiseThreshold_Float" },
            { ProcessingFlag::AdaptiveNoiseFilterType_Int, "AdaptiveNoiseFilterType_Int" },
            { ProcessingFlag::AutoExposureRefAmplitude_Float, "AutoExposureRefAmplitude_Float" },
            { ProcessingFlag::UseAdaptiveNoiseFilter_Bool, "UseAdaptiveNoiseFilter_Bool" },
            { ProcessingFlag::UseAutoExposure_Bool, "UseAutoExposure_Bool" },
            { ProcessingFlag::UseRemoveFlyingPixel_Bool, "UseRemoveFlyingPixel_Bool" },
            { ProcessingFlag::UseMPIFlagAverage_Bool, "UseMPIFlagAverage_Bool" },
            { ProcessingFlag::UseMPIFlag_Amp_Bool, "UseMPIFlag_Amp_Bool" },
            { ProcessingFlag::UseMPIFlag_Dist_Bool, "UseMPIFlag_Dist_Bool" },
            { ProcessingFlag::UseValidateImage_Bool, "UseValidateImage_Bool" },
            { ProcessingFlag::UseRemoveStrayLight_Bool, "UseRemoveStrayLight_Bool" },
            { ProcessingFlag::UseSparsePointCloud_Bool, "UseSparsePointCloud_Bool" },
            { ProcessingFlag::UseFilter2Freq_Bool, "UseFilter2Freq_Bool" },
            { ProcessingFlag::GlobalBinning_Int, "GlobalBinning_Int" },
            { ProcessingFlag::UseAdaptiveBinning_Bool, "UseAdaptiveBinning_Bool" },
            { ProcessingFlag::AutoExposureRefValue_Float, "AutoExposureRefValue_Float" },
            { ProcessingFlag::UseFlagSBI_Bool, "UseFlagSBI_Bool" },
            { ProcessingFlag::UseSmoothingFilter_Bool, "UseSmoothingFilter_Bool" },
            { ProcessingFlag::SmoothingAlpha_Float, "SmoothingAlpha_Float" },
            { ProcessingFlag::SmoothingFilterType_Int, "SmoothingFilterType_Int" },
            { ProcessingFlag::UseHoleFilling_Bool, "UseHoleFilling_Bool" },
            { ProcessingFlag::AutoExpoMin_Int, "AutoExpoMin_Int" },
            { ProcessingFlag::AutoExpoMax_Int, "AutoExpoMax_Int" },
        }
    };
}

namespace royale
{
    String getProcessingFlagName (ProcessingFlag mode)
    {
        auto processingFlagName = processingFlagNames.find (mode);
        if (processingFlagName == processingFlagNames.end())
        {
            return "";
        }

        return processingFlagName->second;
    }


    bool parseProcessingFlagName (const String &modeName, ProcessingFlag &processingFlag)
    {
        for (auto it : processingFlagNames)
        {
            if (it.second.compare (modeName) == 0)
            {
                processingFlag = it.first;
                return true;
            }
        }

        return false;
    }

    ProcessingParameterMap combineProcessingMaps (const ProcessingParameterMap &a,
            const ProcessingParameterMap  &b)
    {
        ProcessingParameterMap combination = a;
        for (auto param : b)
        {
            combination[param.first] = param.second;
        }

        return combination;
    }
}
