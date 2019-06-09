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

#include <config/IImagerExternalConfig.hpp>
#include <hal/INonVolatileStorage.hpp>
#include <usecase/UseCase.hpp>

namespace royale
{
    namespace config
    {
        /**
         * The full set of data needed for Royale to use a module with a Flash Defined Imager.
         *
         * It is expected that there will be a 1:1 correspondence between the use cases in
         * imagerExternalConfig and royaleUseCaseList, however this should not be relied upon,
         * instead the two lists should be connected by UseCaseIdentifiers only.  Nor should code
         * assume that the use cases in the two structures are in the same order, even if they have
         * the same number of use cases.
         */
        struct ExternalConfig
        {
            /**
             * Config for the Imager component when using a Flash Defined Imager.
             */
            std::shared_ptr<royale::imager::IImagerExternalConfig> imagerExternalConfig;

            /**
             * Config for the Processing and Frame Collector.  The use cases in this list do not
             * have sufficient information for the imager, and therefore these can only be used when
             * another source is providing the information for the imager itself, for example when
             * there is additionally an IImagerExternalConfig.
             */
            std::vector<royale::usecase::UseCase> royaleUseCaseList;

            /**
             * Holder for the calibration data and product code, always non-nullptr.
             *
             * The product code will be available from INonVolatileStorage::getModuleIdentifier even
             * if there was no (or corrupt) calibration data. With Zwetschge, the product code is in
             * the ToC, so it can be validated even if the checksum for the calibration data fails.
             *
             * The INonVolatileStorage's methods may require further I/O, see the documentation of
             * the function that returned this ExternalConfig for requirements and guarantees.
             */
            std::shared_ptr<royale::hal::INonVolatileStorage> calibration;
        };
    }
}
