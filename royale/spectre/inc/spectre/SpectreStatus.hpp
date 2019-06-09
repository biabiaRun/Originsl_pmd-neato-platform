/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#ifndef __SPECTRESTATUS_HPP__
#define __SPECTRESTATUS_HPP__

#include "common/Field.hpp"
#include "spectre/SpectreConfig.hpp"

namespace spectre
{

    /// Used to return the status of a Spectre operation
    class SpectreStatus
    {
    public:
        /**
         * Status codes of a Spectre operation
         *
         */
        enum class StatusCode
        {
            /// The operation succeeded
            SUCCESS,
            /// An error occurred
            ERROR,
            /// An error during calibration handling occurred. SpectreStatus::additional() contains an error code from libcalibration.
            CALIBRATION_ERROR,
            /// Invalid arguments were passed to the function
            INVALID_ARGUMENTS,
            /// An error during processing initialization occurred. SpectreStatus::additional() contains an error code from libprocessing.
            PROCESSING_INIT_ERROR,
            /// The passed persistent initialization blob is invalid
            INVALID_PERINIT_DATA,
            /// The filtered backend descriptor list was empty (e.g., no backend registered, no backend is not compatible with basic configuration)
            BACKEND_DESCRIPTOR_LIST_EMPTY,
            /// No suitable backend for the extended configuration found
            NO_SUITABLE_BACKEND_FOUND,
            /// An extended configuration which does not belong to the current <instance,basic config> pair has been to passed
            STALE_CONFIGURATION,
            /// An output proxy could not be initialized
            OUTPUT_PROXY_ERROR
        };

        /**
         * @brief Ctor
         *
         * @param code error code
         * @param additionalCodes list of additional internal codes
         */
        SpectreStatus (StatusCode code,
                       std::initializer_list<int> additionalCodes)
            : m_code (code), m_additional (additionalCodes)
        { }

        /**
         * @brief Ctor
         *
         * @param code error code
         * @param additionalCodes list of additional internal codes
         */
        SpectreStatus (StatusCode code,
                       common::ArrayHolder<int> additionalCodes)
            : m_code (code), m_additional (std::move (additionalCodes))
        { }



        /**
         * @brief Ctor
         *
         * @param code error code
         * @param additional list of additional internal codes
         */
        SpectreStatus (StatusCode code,
                       int additional)
            : SpectreStatus (code, std::initializer_list<int> (
        {
            additional
        }))
        {}

        /**
         * @brief Ctor
         *
         * @param code error code
         */
        explicit SpectreStatus (StatusCode code)
            : SpectreStatus (code, {})
        {}


        /**
         * @brief Gets the status code of a Spectre operation
         *
         *
         * @return status code
         */
        inline StatusCode code() const
        {
            return m_code;
        }

        /**
         * @brief Gets an additional (internal) error codes
         *
         * The returned array can be empty if no additional code is available.
         *
         * @return internal error codes
         */
        inline common::ArrayHolder<int> additional() const
        {
            return m_additional;
        }

        inline operator bool() const
        {
            return m_code == StatusCode::SUCCESS;
        }

        inline static SpectreStatus success()
        {
            return SpectreStatus (StatusCode::SUCCESS);
        }

        inline static SpectreStatus error (StatusCode code = StatusCode::ERROR)
        {
            return SpectreStatus (code);
        }

        /**
         * @brief Gets a description of the error
         *
         * The returned null-terminated string is owned by the SpectreStatus instance.
         *
         * @return error description
         */
        SPECTRE_API const char *description() const;


    private:
        StatusCode m_code;
        common::ArrayHolder<int> m_additional;
        mutable common::ArrayHolder<char> m_errorMsg;
    };

}  // spectre

#endif /*__SPECTRESTATUS_HPP__*/
