/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#pragma once

#include <common/exceptions/Exception.hpp>

namespace royale
{
    namespace common
    {
        /**
         * This class represents the error that occurs when the external imager configuration file
         * is not found.
         */
        class ImagerConfigNotFoundError : public Exception
        {
        public:
            /**
             * Create an error object.
             * @param configFileName the name of the configuration file that was not found.
             */
            explicit ImagerConfigNotFoundError (const String &configFileName = "")
                : m_configFileName (configFileName)
            {
            }

            /**
             * Get the name of the configuration file that could not be found.
             * @return the configuration file name or an empty string if the file is unknown.
             */
            const String &getConfigFileName() const
            {
                return m_configFileName;
            }

            /**
             * Set the name of the configuration file that could not found.
             * @param configFileName the configuration file name.
             */
            void setConfigFileName (const String &configFileName)
            {
                m_configFileName = configFileName;
            }

            /**
             * @inheritdoc
             * @todo ROYAL-3377 Ensure that this method does not throw an exception.
             */
            const char *what() const throw() override
            {
                m_description = "The external imager configuration file ";

                if (!m_configFileName.empty())
                {
                    m_description += "'";
                    m_description += m_configFileName;
                    m_description += "' ";
                }

                m_description += "was not found.";

                return m_description.c_str();
            }

        private:
            String m_configFileName;
            mutable String m_description;
        };
    }
}
