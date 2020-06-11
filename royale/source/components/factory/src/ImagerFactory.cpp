/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <factory/ImagerFactory.hpp>

#include <factory/ImagerConfigAdapter.hpp>
#include <factory/FlashDefinedImagerInterfaceAdapter.hpp>
#include <factory/SoftwareDefinedImagerInterfaceAdapter.hpp>
#include <imager/ImagerM2450_A11.hpp>
#include <imager/ImagerM2450_A12_AIO.hpp>
#include <imager/ImagerM2452_B1x_AIO.hpp>
#include <imager/ImagerM2453_A11.hpp>
#include <imager/ImagerM2453_B11.hpp>
#include <imager/ImagerM2455_A11.hpp>
#include <imager/ImagerM2455_A14.hpp>
#include <imager/ImagerM2455_B12.hpp>
#include <imager/ImagerM2457_A11.hpp>

#include <imager/ImagerDirectAccess.hpp>

#include <imager/M2450_A11/PseudoDataInterpreter.hpp>
#include <imager/M2450_A12/PseudoDataInterpreter.hpp>
#include <imager/M2450_A12/PseudoDataInterpreter_AIO.hpp>
#include <imager/M2452/PseudoDataInterpreter.hpp>
#include <imager/M2452/PseudoDataInterpreter_AIO.hpp>
#include <imager/M2453/PseudoDataInterpreter.hpp>
#include <imager/M2453/PseudoDataInterpreter_B11.hpp>
#include <imager/M2455/PseudoDataInterpreter.hpp>
#include <imager/M2457/PseudoDataInterpreter.hpp>
#include <imager/M2457/PseudoDataInterpreter_IC.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/RuntimeError.hpp>

using namespace royale;
using namespace royale::factory;
using namespace royale::imager;

namespace
{

    std::shared_ptr<royale::imager::FlashDefinedImagerComponent> createFlashDefinedImager (
        std::shared_ptr<royale::hal::IBridgeImager> bridge,
        const std::shared_ptr<const royale::config::ICoreConfig> &coreConfig,
        const std::shared_ptr<const royale::config::ImagerConfig> &imagerConfig,
        const royale::config::IlluminationConfig &illuminationConfig)
    {
        auto externalConfig = imagerConfig->externalImagerConfig;
        if (!externalConfig)
        {
            // This shouldn't happen, we need the configuration
            LOG (WARN) << "ImagerFactory : No external imager configuration specified";
            throw royale::common::RuntimeError ("External configuration couldn't be loaded");
        }

        const auto newParams = royale::factory::ImagerConfigAdapter().createImagerParameters (
                                   bridge, std::move (externalConfig), coreConfig, imagerConfig, illuminationConfig);
        switch (imagerConfig->imagerType)
        {
            case config::ImagerType::M2453_A11:
                return std::make_shared<royale::imager::ImagerM2453_A11> (newParams);
            case config::ImagerType::M2453_B11:
                return std::make_shared<royale::imager::ImagerM2453_B11> (newParams);
            case config::ImagerType::M2455_A11:
                return std::make_shared<royale::imager::ImagerM2455_A11> (newParams);
            case config::ImagerType::M2455_A14:
                return std::make_shared<royale::imager::ImagerM2455_A14> (newParams);
            case config::ImagerType::M2455_B12:
                return std::make_shared<royale::imager::ImagerM2455_B12> (newParams);
            case config::ImagerType::M2457_A11:
                return std::make_shared<royale::imager::ImagerM2457_A11> (newParams);
            case config::ImagerType::M2457_A11_SIC:
                return std::make_shared<royale::imager::ImagerM2457_A11> (newParams);
            default:
                // should not be here
                throw royale::common::LogicError ("Imager type is not supported");
        }
    }

}

royale::String ImagerFactory::getImagerTypeName (config::ImagerType imagerType)
{
    switch (imagerType)
    {
        case config::ImagerType::M2450_A11:
            return "IRS1125C_A11";
            break;
        case config::ImagerType::M2450_A12_AIO:
            return "IRS1125A/C_A12";
            break;
        case config::ImagerType::M2452_B1x_AIO:
            return "IRS1645_B1x";
            break;
        case config::ImagerType::M2453_A11:
            return "IRS2381_A11";
            break;
        case config::ImagerType::M2453_B11:
            return "IRS2381_B11";
            break;
        case config::ImagerType::M2455_A11:
            return "IRS2771_A11";
            break;
        case config::ImagerType::M2455_A14:
            return "IRS2771_A14";
            break;
        case config::ImagerType::M2455_B12:
            return "IRS2771_B12";
            break;
        case config::ImagerType::M2457_A11:
            return "IRS2877_A11";
            break;
        case config::ImagerType::M2457_A11_SIC:
            return "IRS2877_A11_SIC";
            break;
        default:
            // should not be here
            throw royale::common::LogicError ("Imager type is not supported");
    }
}

/**
* Retrieves the pseudo data interpreter based on a given ImagerType.
* @param imagerType imagerType that should be used
*/
royale::String ImagerFactory::getPseudoDataInterpreter (config::ImagerType imagerType)
{
    switch (imagerType)
    {
        case config::ImagerType::M2450_A11:
            return "M2450_A11";
            break;
        case config::ImagerType::M2450_A12_AIO:
            return "M2450_A12_AIO";
            break;
        case config::ImagerType::M2452_B1x_AIO:
            return "M2452_AIO";
            break;
        case config::ImagerType::M2453_A11:
            return "M2453_A11";
            break;
        case config::ImagerType::M2453_B11:
            return "M2453_B11";
            break;
        case config::ImagerType::M2455_A11:
            return "M2455_A11";
            break;
        case config::ImagerType::M2455_A14:
            return "M2455_A14";
            break;
        case config::ImagerType::M2455_B12:
            return "M2455_B12";
            break;
        case config::ImagerType::M2457_A11:
            return "M2457_A11";
            break;
        case config::ImagerType::M2457_A11_SIC:
            return "M2457_A11_SIC";
            break;
        default:
            // should not be here
            throw royale::common::LogicError ("Imager type is not supported");
    }
}

/**
* Creates a pseudo data interpreter based on a given string.
* @param pseudoDataInterpreter pseudoDataInterpreter that should be used
*/
std::unique_ptr<common::IPseudoDataInterpreter> ImagerFactory::createPseudoDataInterpreter (const royale::String &pseudoDataInterpreter)
{
    std::unique_ptr<royale::common::IPseudoDataInterpreter> pseudoDataInter;
    if (pseudoDataInterpreter.compare ("M2450_A11") == 0)
    {
        pseudoDataInter.reset (new M2450_A11::PseudoDataInterpreter);
    }
    else if (pseudoDataInterpreter.compare ("M2450_A12") == 0)
    {
        pseudoDataInter.reset (new M2450_A12::PseudoDataInterpreter);
    }
    else if (pseudoDataInterpreter.compare ("M2450_A12_AIO") == 0)
    {
        pseudoDataInter.reset (new M2450_A12::PseudoDataInterpreter_AIO);
    }
    else if (pseudoDataInterpreter.compare ("M2452") == 0)
    {
        pseudoDataInter.reset (new M2452::PseudoDataInterpreter);
    }
    else if (pseudoDataInterpreter.compare ("M2452_AIO") == 0)
    {
        pseudoDataInter.reset (new M2452::PseudoDataInterpreter_AIO);
    }
    else if (pseudoDataInterpreter.compare ("M2453_A11") == 0)
    {
        pseudoDataInter.reset (new M2453_A11::PseudoDataInterpreter);
    }
    else if (pseudoDataInterpreter.compare ("M2453_B11") == 0)
    {
        pseudoDataInter.reset (new M2453_B11::PseudoDataInterpreterB11);
    }
    else if (pseudoDataInterpreter.compare ("M2455_A11") == 0)
    {
        pseudoDataInter.reset (new M2455::PseudoDataInterpreter);
    }
    else if (pseudoDataInterpreter.compare ("M2455_A14") == 0)
    {
        pseudoDataInter.reset (new M2455::PseudoDataInterpreter);
    }
    else if (pseudoDataInterpreter.compare ("M2455_B12") == 0)
    {
        pseudoDataInter.reset (new M2455::PseudoDataInterpreter);
    }
    else if (pseudoDataInterpreter.compare ("M2457_A11") == 0 ||
             pseudoDataInterpreter.compare ("M2457") == 0)
    {
        pseudoDataInter.reset (new M2457::PseudoDataInterpreter);
    }
    else if (pseudoDataInterpreter.compare ("M2457_A11_SIC") == 0)
    {
        pseudoDataInter.reset (new M2457::PseudoDataInterpreter_IC);
    }
    else
    {
        // should not be here
        throw royale::common::LogicError ("Pseudo data interpreter type is not supported");
    }
    return pseudoDataInter;
}

/**
* Creates a pseudo data interpreter based on the given ImagerType.
* @param imagerType imager type of the module
*/
std::unique_ptr<common::IPseudoDataInterpreter> ImagerFactory::createPseudoDataInterpreter (config::ImagerType imagerType)
{
    return createPseudoDataInterpreter (getPseudoDataInterpreter (imagerType));
}

bool ImagerFactory::getRequiresUseCaseDefGuids (config::ImagerType imagerType)
{
    switch (imagerType)
    {
        case config::ImagerType::M2450_A11:
            return false;
        case config::ImagerType::M2450_A12_AIO:
            return false;
        case config::ImagerType::M2452_B1x_AIO :
            return false;
        case config::ImagerType::M2453_A11:
            return true;
        case config::ImagerType::M2453_B11:
            return true;
        case config::ImagerType::M2455_A11:
            return true;
        case config::ImagerType::M2455_A14:
            return true;
        case config::ImagerType::M2455_B12:
            return true;
        case config::ImagerType::M2457_A11:
            return true;
        case config::ImagerType::M2457_A11_SIC:
            return true;
        default:
            // should not be here
            throw royale::common::LogicError ("Imager type is not supported");
    }
}

/**
* Creates an imager instance based on the given ImagerType.
* @param bridge an instance of an IBridgeImager
* @param coreConfig CoreConfig of the camera module
* @param imagerConfig Imager configuration data (including imager type and base config)
* @param illuminationConfig configuration data for the illumination unit
* @param directAccessEnabled create an imager that allows to read and write registers
*/
std::shared_ptr<hal::IImager> ImagerFactory::createImager (
    std::shared_ptr<royale::hal::IBridgeImager> bridge,
    const std::shared_ptr<const royale::config::ICoreConfig> &coreConfig,
    const std::shared_ptr<const royale::config::ImagerConfig> &imagerConfig,
    const royale::config::IlluminationConfig &illuminationConfig,
    bool directAccessEnabled)
{
    ImagerParameters params = royale::factory::ImagerConfigAdapter().createImagerParameters (bridge, nullptr, coreConfig, imagerConfig, illuminationConfig);

    std::shared_ptr<royale::imager::ISoftwareDefinedImagerComponent> sdimager;
    std::shared_ptr<royale::imager::FlashDefinedImagerComponent>    fdimager;
    std::shared_ptr<royale::imager::ImagerRegisterAccess>            fdDirectAccess;
    switch (imagerConfig->imagerType)
    {
        case config::ImagerType::M2450_A11:
            sdimager.reset (directAccessEnabled ? new royale::imager::ImagerDirectAccess<royale::imager::ImagerM2450_A11> { params } :
                            new royale::imager::ImagerM2450_A11{ params });
            break;
        case config::ImagerType::M2450_A12_AIO:
            sdimager.reset (directAccessEnabled ? new royale::imager::ImagerDirectAccess<royale::imager::ImagerM2450_A12_AIO> { params } :
                            new royale::imager::ImagerM2450_A12_AIO{ params });
            break;
        case config::ImagerType::M2452_B1x_AIO :
            sdimager.reset (directAccessEnabled ? new royale::imager::ImagerDirectAccess<royale::imager::ImagerM2452_B1x_AIO> { params } :
                            new royale::imager::ImagerM2452_B1x_AIO{ params });
            break;
        case config::ImagerType::M2453_A11:
        case config::ImagerType::M2453_B11:
        case config::ImagerType::M2455_A11:
        case config::ImagerType::M2455_A14:
        case config::ImagerType::M2455_B12:
        case config::ImagerType::M2457_A11:
        case config::ImagerType::M2457_A11_SIC:
            fdimager = createFlashDefinedImager (bridge, coreConfig, imagerConfig, illuminationConfig);
            if (directAccessEnabled)
            {
                fdDirectAccess = std::make_shared<royale::imager::ImagerRegisterAccess> (bridge);
            }
            break;
        default:
            // should not be here
            throw royale::common::LogicError ("Imager type is not supported");
    }

    if (!sdimager)
    {
        if (!fdimager)
        {
            throw royale::common::LogicError ("Imager type is not yet supported (ROYAL-2874)");
        }

        return FlashDefinedImagerInterfaceAdapter::createImager (fdimager, fdDirectAccess);
    }

    return SoftwareDefinedImagerInterfaceAdapter::createImager (sdimager);
}
