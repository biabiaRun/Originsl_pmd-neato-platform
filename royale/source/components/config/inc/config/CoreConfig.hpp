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
#include <usecase/UseCase.hpp>
#include <config/ICoreConfig.hpp>

#include <cstdint>
#include <map>

namespace royale
{
    namespace config
    {
        /**
         * CoreConfig is a container used in the ModuleConfig, used for collecting all information
         * that is required for the Royale core component.  However CoreConfig is organised for
         * collecting data, and it needs cross-references between its data members to be resolved.
         * For example, the UseCaseList's members do not have their image size initialized, and
         * their processing parameters need to be either combined with the standardParameters or
         * resolved with external data from the IProcessingParameterMapFactory.
         *
         * The CoreConfigFactory will take the CoreConfig and return an object implementing
         * ICoreConfig; and access via ICoreConfig will return data with those cross-references
         * resolved, for example the use cases will have the same image size as the imager.
         *
         * The CoreConfig should be treated as const after creation.
         */
        struct CoreConfig
        {
            royale::Pair<uint16_t, uint16_t> lensCenterDesign;         //!< center of the pixel array for the final imager (column/row)
            royale::Pair<uint16_t, uint16_t> maxImageSize;             //!< image size (width/height)
            usecase::UseCaseList             useCases;                 //!< list of supported use cases for the module
            BandwidthRequirementCategory     bandwidthCategory;        //!< throttling parameter for USB-based devices
            FrameTransmissionMode            frameTransmissionMode;    //!< defines the data organization coming from the imager
            royale::String                   cameraName;               //!< custom string which can be used to name the camera module
            float                            temperatureHardLimit;     //!< temperature limit which is used to switch off the module
            float                            temperatureSoftLimit;     //!< temperature limit which is used to inform the user
            bool                             isAutoExposureSupported;  //!< specifies if the module supports auto-exposure
            /**
             * For use cases that include processing parameter maps, this contents of this map will
             * be used as the default values that can be overrided by the use-case-specific values.
             *
             * This is not used for use cases that link to a map by ProcessingParameterId.
             */
            royale::ProcessingParameterMap   standardParameters;
        };
    }
}
