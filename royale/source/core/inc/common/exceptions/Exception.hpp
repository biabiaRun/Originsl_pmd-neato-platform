/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <stdexcept>
#include <string>
#include <royale/Status.hpp>
#include <common/RoyaleLogger.hpp>

namespace royale
{
    namespace common
    {
        /// @brief Parent class for all Royale exceptions.
        ///
        /// Examples:
        ///    throw Exception ("Flux compensator not initialized", "There has been a problem with the time machine", ROYALE_ERROR_RUNTIME_ERROR);
        ///    throw RuntimeError ("Flux compensator not initialized", "There has been a problem with the time machine");
        ///    throw InvalidValue ("Modulation frequency 3.33 MHz not supported");
        class Exception : public std::exception
        {
        public:
            /// Constructor
            /// \param dt Technical description of what went wrong (for debugging).
            /// \param du User description of what went wrong (for display). Leave empty for default description.
            /// \param s Error code corresponding to the exception.
            Exception (std::string dt = "unknown error", std::string du = "", royale::CameraStatus s = royale::CameraStatus::UNKNOWN) :
                m_status (s),
                m_technicalDescription (dt),
                m_userDescription (du)
            {
                LOG (DEBUG) << "Exception thrown : " << m_technicalDescription;
                // construction of the m_what string is deferred until what() is called,
                // because it must be deferred until the subclass constructors have run
            }

            /// Get a type description of the exception.
            virtual std::string getExceptionType () const
            {
                return "Generic exception";
            }

            /// Get a technical description of what went wrong (for debugging).
            const std::string getTechnicalDescription () const
            {
                return m_technicalDescription;
            }

            /// Get the user description of what went wrong (for display).
            /// Only to be used when the application end user shall see a message.
            const std::string getUserDescription () const
            {
                if (m_userDescription == "")
                {
                    return "An unknown error occurred";
                }
                return m_userDescription;
            }

            /// Get the error code corresponding to the exception
            royale::CameraStatus getStatus () const
            {
                return m_status;
            }

            const char *what() const throw() override
            {
                if (m_what.empty())
                {
                    try
                    {
                        m_what = getExceptionType() + ": " + getTechnicalDescription();
                    }
                    catch (...)
                    {
                        return m_technicalDescription.c_str();
                    }
                }
                // from cplusplus.com:
                // This is guaranteed to be valid at least until the exception object from
                // which it is obtained is destroyed or until a non-const member function
                // of the exception object is called.
                return m_what.c_str ();
            }

        private:
            royale::CameraStatus m_status;
            std::string m_technicalDescription;
            std::string m_userDescription;
            mutable std::string m_what;
        };
    }
}

