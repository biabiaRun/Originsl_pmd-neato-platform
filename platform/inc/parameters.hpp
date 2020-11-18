#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <royale.hpp>

namespace ToFparams
{
  // Parameter settings
  static const royale::ProcessingParameterPair NOISE_THRESHOLD ({ royale::ProcessingFlag::NoiseThreshold_Float, 0.07f });
  static const royale::ProcessingParameterPair AUTO_EXPOSURE_REF_VALUE ({ royale::ProcessingFlag::AutoExposureRefValue_Float, 1000.0f });
  static const royale::ProcessingParameterPair ADAPTIVE_NOISE_FILTER_TYPE ({ royale::ProcessingFlag::AdaptiveNoiseFilterType_Int, 2 });
  static const royale::ProcessingParameterPair GLOBAL_BINNING ({ royale::ProcessingFlag::GlobalBinning_Int, 1 });
  static const royale::ProcessingParameterPair USE_ADAPTIVE_NOISE_FILTER ({ royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, true });
  static const royale::ProcessingParameterPair USE_FLYING_PIXEL ({ royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, false });
  static const royale::ProcessingParameterPair USE_MPI_AVERAGE ({ royale::ProcessingFlag::UseMPIFlagAverage_Bool, true });
  static const royale::ProcessingParameterPair USE_MPI_AMP ({ royale::ProcessingFlag::UseMPIFlag_Amp_Bool, true });
  static const royale::ProcessingParameterPair USE_MPI_DIST ({ royale::ProcessingFlag::UseMPIFlag_Dist_Bool, true });
  static const royale::ProcessingParameterPair USE_VALIDATE_IMAGE ({ royale::ProcessingFlag::UseValidateImage_Bool, false });
  static const royale::ProcessingParameterPair USE_STRAY_LIGHT ({ royale::ProcessingFlag::UseRemoveStrayLight_Bool, true });
  static const royale::ProcessingParameterPair USE_FILTER_2_FREQ ({ royale::ProcessingFlag::UseFilter2Freq_Bool, true });
  static const royale::ProcessingParameterPair USE_SBI_FLAG ({ royale::ProcessingFlag::UseFlagSBI_Bool, false });
  static const royale::ProcessingParameterPair USE_SMOOTHING_FILTER ({ royale::ProcessingFlag::UseSmoothingFilter_Bool, false });
  static const royale::ProcessingParameterPair USE_HOLE_FILLING ({ royale::ProcessingFlag::UseHoleFilling_Bool, false });
}
#endif