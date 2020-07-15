/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#ifndef __PROCESSINGTYPE_HPP__
#define __PROCESSINGTYPE_HPP__

#include <iostream>

namespace spectre
{
    namespace common
    {
        /**
         * @brief A ProcessingType is used to identify a logical pipeline
         * (type).
         *
         * Each logical pipeline can have multiple implementations, which offer
         * a common set of features.
         */
        enum class ProcessingType
        {
            /**
             * @brief A general purpose pipeline.
             */
            WS,
            /**
             * @brief General purpose pipeline for scaled images
             */
            CB_FAST,
            /**
             * @brief General purpose pipeline with binned processing (2x1) for
             * scaled images
             */
            CB_BINNED_WS,
            /**
             * @brief Next generation general purpose pipeline
             */
            NG,
            /**
             * @brief Ng pipeline with binned processing (2x1) for checker-board
             * imagers
             */
            CB_BINNED_NG,
            /**
             * @brief A pipeline which offers only auto-exposure.
             *
             * This pipeline can be used without a calibration.
             */
            AUTO_EXPOSURE_ONLY,
            /**
             * @brief A pipeline which calculates only a gray image
             *
             * This pipeline requires a special sensor configuration, with an
             * illuminated & unilluminated intensity frame.
             */
            GRAY_IMAGE,
            /**
             * @brief Automatic selection of pipeline
             */
            AUTO,
            NUM_ENTRIES
        };

        inline std::ostream &operator<< (std::ostream &os, ProcessingType type)
        {
            return os << static_cast<int> (type);
        }

    } // namespace common
} // namespace spectre

#endif /*__PROCESSINGTYPE_HPP__*/
