/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#ifndef __RESULTTYPE_HPP__
#define __RESULTTYPE_HPP__

#include <type_traits>
#include <stdint.h>

namespace spectre
{
#define SPECTRE_RESULT_TYPES(X)                                         \
    /*! 3D coordinates with confidence (in x1,y1,z1,c1,...,xn,yn,zn,cn format)*/ X(COORDINATES) \
    /*! Amplitudes for each pixel */ X(AMPLITUDES)                       \
    /*! Flags for each pixel */ X(FLAGS)                                \
    /*! Distance noise for each pixel (in m) */ X(DISTANCE_NOISES)      \
    /*! Intensity for each pixel (gray value picture without illumnation) */ X(INTENSITIES) \
    /*! Radial distance for each pixel */ X(DISTANCES)

#define SPECTRE_ENUM_NAME(KEY,...) KEY,
    enum class ResultType
    {
        SPECTRE_RESULT_TYPES (SPECTRE_ENUM_NAME)
        EXPOSURE_TIMES,
        NUM_ENTRIES
    };
#undef SPECTRE_ENUM_NAME


    /**
     * @brief Data type returned by the depth-processing for a ResultType value
     */
    template<ResultType R>
    using ResultTypeType = typename std::enable_if < R != ResultType::NUM_ENTRIES, typename std::conditional < (R == ResultType::FLAGS || R == ResultType::EXPOSURE_TIMES), uint32_t, float >::type >::type;

}  // spectre

#endif /*__RESULTTYPE_HPP__*/
