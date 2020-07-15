/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

using System;

namespace RoyaleDotNet
{
    /*!
    *  This is a list of flags which can be set/altered in the camera expert mode in order
    *  to control the processing pipeline. The suffix type indicates the expected Variant type.
    *  Keep in mind, that if this list is changed, the map with names has to be adapted!
    */
    public enum ProcessingFlag
    {
        ///Consistency limit for asymmetry validation
        ConsistencyTolerance_Float,
        ///Scaling factor for lower depth value normalization
        FlyingPixelsF0_Float,
        ///Scaling factor for upper depth value normalization
        FlyingPixelsF1_Float,
        ///Upper normalized threshold value for flying pixel detection
        FlyingPixelsFarDist_Float,
        ///Lower normalized threshold value for flying pixel detection
        FlyingPixelsNearDist_Float,
        ///Lower limit for valid raw data values
        LowerSaturationThreshold_Int,
        ///Upper limit for valid raw data values
        UpperSaturationThreshold_Int,
        ///Threshold for MPI flags triggered by amplitude discrepancy
        MPIAmpThreshold_Float,
        ///Threshold for MPI flags triggered by distance discrepancy
        MPIDistThreshold_Float,
        ///Threshold for MPI flags triggered by noise
        MPINoiseDistance_Float,
        ///Upper threshold for final distance noise
        NoiseThreshold_Float,
        ///Kernel type of the adaptive noise filter
        AdaptiveNoiseFilterType_Int,
        ///DEPRECATED : The reference amplitude for the new exposure estimate
        AutoExposureRefAmplitude_Float,
        ///Activate spatial filter reducing the distance noise
        UseAdaptiveNoiseFilter_Bool,
        ///DEPRECATED : Activate dynamic control of the exposure time
        UseAutoExposure_Bool,
        ///Activate FlyingPixel flag
        UseRemoveFlyingPixel_Bool,
        ///Activate spatial averaging MPI value before thresholding
        UseMPIFlagAverage_Bool,
        ///Activates MPI-amplitude flag
        UseMPIFlag_Amp_Bool,
        ///Activates MPI-distance flag
        UseMPIFlag_Dist_Bool,
        ///Activates output image validation
        UseValidateImage_Bool,
        ///Activates the removal of stray light
        UseRemoveStrayLight_Bool,
        ///DEPRECATED : Creates a sparse-point cloud in Spectre
        UseSparsePointCloud_Bool,
        ///Activates 2 frequency filtering
        UseFilter2Freq_Bool,
        ///Size of the global binning kernel
        GlobalBinning_Int,
        ///DEPRECATED : Activate adaptive binning
        UseAdaptiveBinning_Bool,
        ///The reference value for the new exposure estimate
        AutoExposureRefValue_Float,
        ///Enable/Disable the smoothing filter
        UseSmoothingFilter_Bool,
        ///The alpha value used for the smoothing filter
        SmoothingAlpha_Float,
        ///Determines the type of smoothing that is used
        SmoothingFilterType_Int,
        ///Enable/Disable the flagging of pixels where the SBI was active
        UseFlagSBI_Bool,
        ///Enable/Disable the hole filling algorithm
        UseHoleFilling_Bool,

        Reserved1,
        Reserved2,
        Reserved3,
        Reserved4,
        Reserved5,
        Reserved6,
        Reserved7,
        Reserved8,

        ///The minimum value for the auto exposure algorithm
        AutoExpoMin_Int,
        ///The maximum value for the auto exposure algorithm
        AutoExpoMax_Int,
        ///The type of processing used by Spectre
        SpectreProcessingType_Int,
        ///Uses the fallback image in the gray image pipeline as amplitude image
        UseGrayImageFallbackAmplitude_Bool,
        ///Value where the mean of the gray image is mapped to
        GrayImageMeanMap_Int,
        ///SigmaD
        NoiseFilterSigmaD_Float,
        ///Iterations of the noise filter
        NoiseFilterIterations_Int,
        ///Angle limit of the flying pixel algorithm
        FlyingPixelAngleLimit_Float,
        ///Amplitude threshold of the flying pixel algorithm
        FlyingPixelAmpThreshold_Float,
        ///Minimum neighbors for the flying pixel algorithm
        FlyingPixelMinNeighbors_Int,
        ///Maximum neighbors for the flying pixel algorithm
        FlyingPixelMaxNeighbors_Int,
        ///Noiseratio threshold
        FlyingPixelNoiseRatioThresh_Float,
        ///Reset value for the smoothing
        SmoothingFilterResetThreshold_Float,
        ///Connected components threshold
        CCThresh_Int,
        ///PhaseNoise threshold
        PhaseNoiseThresh_Float,
        ///Straylight threshold
        StraylightThreshold_Float,
    };
}
