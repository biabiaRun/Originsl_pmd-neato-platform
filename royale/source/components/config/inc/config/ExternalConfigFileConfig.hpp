/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>

#include <string>

namespace royale
{
    namespace config
    {
        /**
         * For modules that use the ModuleConfigFactoryByStorageId, and either
         * - use a Lena or Zwetschge file (on the host, separate from the calibration storage), or
         * - use a Lena-as-a-string embedded in Royale itself,
         * then this class holds the filename in which that configuration is stored (or the whole
         * data, in the case of Lena-as-a-string).
         *
         * If the module doesn't use an ExternalConfig, all of these must be empty.
         *
         * If the module uses a Zwetschge flash image, then all of these must be empty, the
         * Zwetschge will instead be read while reading the product ID.
         *
         * If the imager doesn't use an IImagerExternalConfig, either it must use no ExternalConfig,
         * or it must use an ExternalConfig that doesn't include an IImagerExternalConfig.
         */
        struct ExternalConfigFileConfig
        {
        private:
            explicit ExternalConfigFileConfig (
                std::string lenaFile = {},
                std::string lenaString = {},
                std::string zwetschgeFile = {}) :
                lenaFile (lenaFile),
                lenaString (lenaString),
                zwetschgeFile (zwetschgeFile)
            {
            }
        public:
            static ExternalConfigFileConfig empty()
            {
                return ExternalConfigFileConfig ("", "", "");
            }
            static ExternalConfigFileConfig fromLenaFile (const std::string &uri)
            {
                return ExternalConfigFileConfig (uri, "", "");
            }
            static ExternalConfigFileConfig fromLenaString (const std::string &data)
            {
                return ExternalConfigFileConfig ("", data, "");
            }
            static ExternalConfigFileConfig fromZwetschgeFile (const std::string &uri)
            {
                return ExternalConfigFileConfig ("", "", uri);
            }

            /**
             * Defines an external imager configuration file in the Lena format which can be used
             * with the M2453 imagers.
             */
            std::string lenaFile;

            /**
             * Defines an external imager configuration in the Lena format which can be used with
             * the M2453 imagers (as a string). This means that the configuration is converted to a
             * string at compile time, and included in the Royale binary.
             */
            std::string lenaString;

            /**
             * Defines an external imager configuration file in the Zwetschge format which can be
             * used with the M2453 imagers. This requires Royale to send all registers to the
             * imager, and not expect the imager to be able to read directly from the storage.
             */
            std::string zwetschgeFile;

            operator bool() const
            {
                return ! (lenaFile.empty() && lenaString.empty() && zwetschgeFile.empty());
            }
        };
    }
}
