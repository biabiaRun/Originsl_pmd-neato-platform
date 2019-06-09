/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <processing/ParameterMapping.hpp>
#include <map>
#include <array>

#include <common/exceptions/CalibrationDataNotFound.hpp>
#include <common/NarrowCast.hpp>
#include <common/exceptions/RuntimeError.hpp>


using namespace spectre;
using namespace spectre::common;
using namespace royale;

namespace
{
    struct SpectreRoyaleMapping
    {
        std::map<ParameterKey, ProcessingFlag> spectre2Royale;
        std::map<ProcessingFlag, ParameterKey> royale2Spectre;
        void add (ParameterKey key, ProcessingFlag flag)
        {
            spectre2Royale[key] = flag;
            royale2Spectre[flag] = key;
        }
    };

    SpectreRoyaleMapping initMapping()
    {
        SpectreRoyaleMapping mapping;
        mapping.add (ParameterKey::CONSISTENCY_TOLERANCE, ProcessingFlag::ConsistencyTolerance_Float);
        mapping.add (ParameterKey::AUTO_EXPOSURE_SET_VALUE, ProcessingFlag::AutoExposureRefValue_Float);
        mapping.add (ParameterKey::FLYING_PIXEL_F0, ProcessingFlag::FlyingPixelsF0_Float);
        mapping.add (ParameterKey::FLYING_PIXEL_F1, ProcessingFlag::FlyingPixelsF1_Float);
        mapping.add (ParameterKey::FLYING_PIXEL_FAR_DIST, ProcessingFlag::FlyingPixelsFarDist_Float);
        mapping.add (ParameterKey::FLYING_PIXEL_NEAR_DIST, ProcessingFlag::FlyingPixelsNearDist_Float);
        mapping.add (ParameterKey::LOWER_SATURATION_THRESHOLD, ProcessingFlag::LowerSaturationThreshold_Int);
        mapping.add (ParameterKey::UPPER_SATURATION_THRESHOLD, ProcessingFlag::UpperSaturationThreshold_Int);
        mapping.add (ParameterKey::MPI_AMPLITUDE_THRESHOLD, ProcessingFlag::MPIAmpThreshold_Float);
        mapping.add (ParameterKey::MPI_DISTANCE_THRESHOLD, ProcessingFlag::MPIDistThreshold_Float);
        mapping.add (ParameterKey::MPI_NOISE_DISTANCE, ProcessingFlag::MPINoiseDistance_Float);
        mapping.add (ParameterKey::NOISE_THRESHOLD, ProcessingFlag::NoiseThreshold_Float);
        mapping.add (ParameterKey::ADAPTIVE_NOISE_FILTER_TYPE, ProcessingFlag::AdaptiveNoiseFilterType_Int);
        mapping.add (ParameterKey::USE_ADAPTIVE_NOISE_FILTER, ProcessingFlag::UseAdaptiveNoiseFilter_Bool);
        mapping.add (ParameterKey::USE_MPI_FLAG_AMPLITUDE, ProcessingFlag::UseMPIFlag_Amp_Bool);
        mapping.add (ParameterKey::USE_MPI_FLAG_AVERAGE, ProcessingFlag::UseMPIFlagAverage_Bool);
        mapping.add (ParameterKey::USE_MPI_FLAG_DISTANCE, ProcessingFlag::UseMPIFlag_Dist_Bool);
        mapping.add (ParameterKey::USE_REMOVE_FLYING_PIXEL, ProcessingFlag::UseRemoveFlyingPixel_Bool);
        mapping.add (ParameterKey::USE_REMOVE_STRAY_LIGHT, ProcessingFlag::UseRemoveStrayLight_Bool);
        mapping.add (ParameterKey::USE_VALIDATE_IMAGE, ProcessingFlag::UseValidateImage_Bool);
        mapping.add (ParameterKey::USE_FILTER_2FREQ, ProcessingFlag::UseFilter2Freq_Bool);
        mapping.add (ParameterKey::GLOBAL_BINNING, ProcessingFlag::GlobalBinning_Int);
        mapping.add (ParameterKey::USE_FLAG_SBI, ProcessingFlag::UseFlagSBI_Bool);
        mapping.add (ParameterKey::USE_SMOOTHING_FILTER, ProcessingFlag::UseSmoothingFilter_Bool);
        mapping.add (ParameterKey::SMOOTHING_ALPHA, ProcessingFlag::SmoothingAlpha_Float);
        mapping.add (ParameterKey::SMOOTHING_FILTER_TYPE, ProcessingFlag::SmoothingFilterType_Int);
        mapping.add (ParameterKey::USE_FILL_HOLES, ProcessingFlag::UseHoleFilling_Bool);
        mapping.add (ParameterKey::LOWER_EXPOSURE_LIMIT, ProcessingFlag::AutoExpoMin_Int);
        mapping.add (ParameterKey::UPPER_EXPOSURE_LIMIT, ProcessingFlag::AutoExpoMax_Int);
        return mapping;
    }

    static const SpectreRoyaleMapping S_MAPPING = initMapping();

    template<typename T>
    std::array<T, 3> getValues (const Parameter &par)
    {
        std::array<T, 3> ret;
        ret[1] = std::numeric_limits<T>::lowest();
        ret[2] = std::numeric_limits<T>::max();
        if (!par.getValue().get (ret[0]) ||
                (par.hasBounds() && (!par.lowerBound().get (ret[1]) || !par.upperBound().get (ret[2]))))
        {
            throw royale::common::RuntimeError ("Could not retrieve Parameter value or bounds.");
        }

        return ret;
    }

    royale::Variant spectre2RoyaleVariant (const Parameter &par)
    {
        royale::Variant var;
        std::array<bool, 3> boolVals;
        std::array<float, 3> floatVals;
        std::array<int32_t, 3> int32Vals;
        std::array<uint32_t, 3> uint32Vals;

        switch (par.type())
        {
            case ParameterType::BOOL_PARAMETER:
                boolVals = getValues<bool> (par);
                var = royale::Variant (boolVals[0]);
                break;
            case ParameterType::FLOAT_PARAMETER:
                floatVals = getValues<float> (par);
                var = royale::Variant (floatVals[0], floatVals[1], floatVals[2]);
                break;
            case ParameterType::INT32_PARAMETER:
                int32Vals = getValues<int32_t> (par);
                var = royale::Variant (royale::common::narrow_cast<int> (int32Vals[0]), royale::common::narrow_cast<int> (int32Vals[1]),
                                       royale::common::narrow_cast<int> (int32Vals[2]));
                break;
            case ParameterType::UINT32_PARAMETER:
                uint32Vals = getValues<uint32_t> (par);
                for (auto i = 0u; i <= 2u; ++i)
                {
                    if (uint32Vals[i] > static_cast<uint32_t> (std::numeric_limits<int>::max()))
                    {
                        uint32Vals[i] = std::numeric_limits<int>::max();
                    }
                }
                var = royale::Variant (royale::common::narrow_cast<int> (uint32Vals[0]), royale::common::narrow_cast<int> (uint32Vals[1]),
                                       royale::common::narrow_cast<int> (uint32Vals[2]));
                break;
            default:
                throw royale::common::RuntimeError ("Unexpected spectre::ParameterType encountered.");
        }

        return var;
    }

    spectre::ParameterVariant royale2SpectreVariant (const royale::Variant &royaleVar, ParameterType hint)
    {
        spectre::ParameterVariant spectreVariant;
        switch (royaleVar.variantType())
        {
            case VariantType::Bool:
                spectreVariant.set (royaleVar.getBool());
                break;
            case VariantType::Float:
                spectreVariant.set (royaleVar.getFloat());
                break;
            case VariantType::Int:
                if (hint == ParameterType::INT32_PARAMETER)
                {
                    spectreVariant.set (royale::common::narrow_cast<int32_t> (royaleVar.getInt()));
                }
                else if (hint == ParameterType::UINT32_PARAMETER)
                {
                    spectreVariant.set (royale::common::narrow_cast<uint32_t> (royaleVar.getInt()));
                }
                else
                {
                    throw royale::common::RuntimeError ("Unexpected VariantType / ParameterType hint encountered.");
                }
                break;
            default:
                throw royale::common::RuntimeError ("Unexpected royale::VariantType encountered");
        }

        return spectreVariant;
    }
}

namespace royale
{
    namespace processing
    {
        royale::ProcessingParameterMap convertConfiguration (const IExtendedConfiguration &config)
        {
            royale::ProcessingParameterMap map;
            for (const auto &p : config.getParameters())
            {
                auto it = S_MAPPING.spectre2Royale.find (p.key());
                if (it != S_MAPPING.spectre2Royale.cend())
                {
                    auto var = spectre2RoyaleVariant (p);
                    map[it->second] = var;
                }
            }

            return map;
        }

        bool applyRoyaleParameters (const royale::ProcessingParameterMap &map,
                                    IExtendedConfiguration &config)
        {
            for (const auto &par : map)
            {
                auto it = S_MAPPING.royale2Spectre.find (par.first);
                if (it != S_MAPPING.royale2Spectre.cend())
                {
                    auto spectrePar = config.getParameterByKey (it->second);
                    if (spectrePar.isValid())
                    {
                        auto spectreVar = royale2SpectreVariant (par.second, spectrePar.type());
                        if (spectrePar.setValue (spectreVar) &&  config.setParameter (spectrePar))
                        {
                            continue;
                        }
                        else
                        {
                            return false;
                        }
                    }
                }

                continue;
            }

            return true;
        }
    }
}
