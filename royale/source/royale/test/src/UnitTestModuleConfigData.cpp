/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <common/MakeUnique.hpp>
#include <common/SensorRoutingConfigI2c.hpp>
#include <factory/CoreConfigFactory.hpp>
#include <factory/FlowControlStrategyFactory.hpp>
#include <factory/ImagerFactory.hpp>
#include <factory/ModuleConfigFactoryFixed.hpp>
#include <factory/IProcessingParameterMapFactory.hpp>
#include <modules/ModuleConfigData.hpp>
#include <modules/UsbProbeDataListRoyale.hpp>
#include <usecase/UseCaseSlave.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace royale::factory;
using namespace royale::config;
using namespace royale::common;
using namespace testing;

/* Note: files in this directory that are named UnitTest* can run without hardware */

namespace
{
    /** I2C address of the light sensor in all of our modules */
    const uint8_t ADDRESS_MAIN_IMAGER = 0x3d;

    /**
     * Test the contents of ModuleConfig::essentialSensors.  This doesn't test the ImagerConfig, but
     * it uses it to check whether the SensorMap should contain a temperature sensor.
     *
     * It's in a separate function so that the caller can add the failing module's name to the
     * assert message.
     */
    void TestSensorMap (const SensorMap &essential, const ImagerConfig &imagerConfig)
    {
        for (auto &r : essential)
        {
            ASSERT_NE (r.second, nullptr);
        }

        ASSERT_EQ (essential.count (SensorRole::MAIN_IMAGER), 1ul);
        auto routeBase = essential.find (SensorRole::MAIN_IMAGER)->second;
        auto routeI2c = std::dynamic_pointer_cast<const SensorRoutingConfigI2c> (routeBase);
        ASSERT_NE (routeI2c, nullptr);
        ASSERT_EQ (ADDRESS_MAIN_IMAGER, routeI2c->getAddress());

        if (imagerConfig.tempSensor == ImConnectedTemperatureSensor::NONE)
        {
            ASSERT_EQ (essential.count (SensorRole::TEMP_ILLUMINATION), 1ul);
        }
    }

    class MockBridgeFactory : public IBridgeFactory
    {
    public:
        MOCK_METHOD0 (initialize, void());
    };
}

/**
 * Check that each VID/PID pair only appears once in the list returned by getUsbProbeDataRoyale().
 *
 * For example, this catches the mistake of leaving a ModuleConfigFactoryFixed for the default
 * version of a device, and later in the list adding a ModuleConfigFactoryByStorageId for the same
 * VID/PID; a mistake which results in the probing always choosing the default ModuleConfig for this
 * type of device.
 */
TEST (UnitTestModuleConfig, UsbModulesUniqueVidPid)
{
    const auto moduleList = getUsbProbeDataRoyale();
    for (auto x = 0u; x < moduleList.size(); x++)
    {
        const auto &moduleX = moduleList.at (x);
        for (auto y = x + 1; y < moduleList.size(); y++)
        {
            const auto &moduleY = moduleList.at (y);
            ASSERT_FALSE (moduleX.vid == moduleY.vid && moduleX.pid == moduleY.pid)
                    << "The USB VID:PID "
                    << std::hex << std::setw (4) << std::setfill ('0') << moduleX.vid << ":"
                    << std::hex << std::setw (4) << std::setfill ('0') << moduleX.pid
                    << " is listed twice in getUsbProbeDataRoyale()";
        }
    }
}

TEST (UnitTestModuleConfig, AllModulesConfigFactory)
{
    for (const auto &pd : getUsbProbeDataRoyale())
    {
        // We can only test devices that don't require a working bridge for probing.
        auto mcf = std::dynamic_pointer_cast<ModuleConfigFactoryFixed> (pd.moduleConfigFactory);
        if (mcf == nullptr)
        {
            continue;
        }
        MockBridgeFactory dummyBf;
        auto moduleConfig = mcf->probeAndCreate (dummyBf);

        ASSERT_NO_FATAL_FAILURE (TestSensorMap (moduleConfig->essentialSensors, moduleConfig->imagerConfig))
                << "... in the config for " << moduleConfig->coreConfigData.cameraName;
    }
}

TEST (UnitTestModuleConfig, AllModulesCoreConfigFactory)
{
    const auto paramFactory = getProcessingParameterMapFactoryRoyale();
    for (const auto &pd : getUsbProbeDataRoyale())
    {
        for (const auto &moduleConfig : pd.moduleConfigFactory->enumerateConfigs())
        {
            auto ccf = CoreConfigFactory (*moduleConfig, paramFactory);

            ASSERT_NO_THROW (ccf())
                    << "... in the config for " << moduleConfig->coreConfigData.cameraName;
        }
    }
}

TEST (UnitTestModuleConfig, AllModulesUseCases)
{
    using namespace royale::usecase;

    const auto paramFactory = getProcessingParameterMapFactoryRoyale();
    for (const auto &pd : getUsbProbeDataRoyale())
    {
        for (const auto &moduleConfig : pd.moduleConfigFactory->enumerateConfigs())
        {
            auto ccf = CoreConfigFactory (*moduleConfig, paramFactory);
            auto config = ccf();

            if (ImagerFactory::getRequiresUseCaseDefGuids (moduleConfig->imagerConfig.imagerType))
            {
                // Testing the ids for uniqueness
                std::set<UseCaseIdentifier> ids;
                for (const auto &uc : config->getSupportedUseCases())
                {
                    auto id = uc.getDefinition()->getIdentifier();
                    ASSERT_NE (UseCaseIdentifier{}, id)
                            << "... in the config for " << moduleConfig->coreConfigData.cameraName;
                    ASSERT_EQ (0u, ids.count (id))
                            << "... in the config for " << moduleConfig->coreConfigData.cameraName;
                    ids.insert (id);
                }
            }
            else
            {
                for (const auto &uc : config->getSupportedUseCases())
                {
                    ASSERT_EQ (UseCaseIdentifier{}, uc.getDefinition()->getIdentifier());
                }
            }

            // Most devices need to have at least one use case in the ModuleConfig.  An exception is
            // that a Zwetschge ExternalConfig can provide the use case list, so it's OK for a
            // device that's using Zwetschge to have an empty use case list in the ModuleConfig.
            ASSERT_FALSE (config->getSupportedUseCases().empty()
                          && moduleConfig->imagerConfig.externalConfigFileConfig.zwetschgeFile.empty());

            for (const auto &uc : config->getSupportedUseCases())
            {
                ASSERT_EQ (uc.getDefinition()->getStreamIds().size(), uc.getProcessingParameters().size())
                        << "... in the config for " << moduleConfig->coreConfigData.cameraName
                        << ", use case " << uc.getName();
            }
            // The IProcessingParameterMapFactory's mapping of identifiers to parameter maps is
            // indirectly tested by the previous loop, by testing the parameters that
            // CoreConfigFactory added.  Configs that don't use the IPPMF aren't required to be
            // supported by the IPPMF, and the CoreConfigFactory itself handles choosing which set
            // of parameters we test here.
            //
            // Testing the IPPMF on a config that doesn't expect the IPPMF is expected to fail if
            // that config has any mixed modes, as the default value will be parameters for a
            // single-stream use case.
        }
    }
}

namespace
{
    /**
     * Returns true if a UuidlikeIdentifier looks as if it might have been generated by using the
     * hash-any-string function on a string that should have used the parseRfc4122 function.
     */
    bool couldBeATextUuid (const std::array<uint8_t, 16> &id)
    {
        // The hashUuidlikeIdentifier function keeps the first 12 characters of the input string
        // unaltered, this test pastes those characters back in to a string in the right format.
        royale::String s = "{xxxxxxxx-xx00-0000-0000-00000000000}";
        for (auto i = std::size_t (0); i < 12; i++)
        {
            s.at (i) = static_cast<char> (id.at (i));
        }
        try
        {
            parseRfc4122AsUuid (s);
        }
        catch (...)
        {
            return false;
        }
        return true;
    }
}

/**
 * The UuidlikeIdentifier has separate UUID and non-UUID constructors.
 *
 * In the original implementation, there was one constructor which auto-detected UUID strings, so a
 * string of the form "{69F0E9EA-0DFF-47C9-8311-E236F35AB6FF}" would be parsed as a UUID, with an
 * automatic fallback to hashing it if the parsing failed.
 *
 * As of v3.19, there are two constructors, one which always hashes (never parses) and the other
 * which always parses (and throws if it fails).
 *
 * This test looks for identifiers that were created by passing strings that start "{69F0E9EA-0D..."
 * to the hashing constructor.  If all places that create an ID use the hashing constructor then
 * everything works, however if something is parsing UUIDs (or handling them in already-in-binary
 * form, as Zwetschge does) then the mismatch causes the IDs to be different.
 *
 * The test could also get false positives, but it's probably better than not having it.
 */
TEST (UnitTestModuleConfig, UuidlikeStringsHashed)
{
    using namespace royale::usecase;

    const auto paramFactory = getProcessingParameterMapFactoryRoyale();
    for (const auto &pd : getUsbProbeDataRoyale())
    {
        for (const auto &moduleConfig : pd.moduleConfigFactory->enumerateConfigs())
        {
            auto ccf = CoreConfigFactory (*moduleConfig, paramFactory);
            auto config = ccf();

            if (ImagerFactory::getRequiresUseCaseDefGuids (moduleConfig->imagerConfig.imagerType))
            {
                for (const auto &uc : config->getSupportedUseCases())
                {
                    auto id = uc.getDefinition()->getIdentifier();
                    EXPECT_FALSE (couldBeATextUuid (id.data()))
                            << "... in the config for " << moduleConfig->coreConfigData.cameraName;
                }
            }
        }
    }
}

/**
 * From v3.11 to v3.20, the ImagerFactory loaded the IImagerExternalConfig, and therefore loading
 * Lena files was supported for all devices independent of which ModuleConfigFactory they used.
 *
 * This changed in ROYAL-2885, when the support moved to the ModuleConfigFactoryByStorageId. This
 * test checks that same the functionality doesn't need to be supported in ModuleConfigFactoryFixed.
 * If the test fails, the support can be added.
 */
TEST (UnitTestModuleConfig, NoCombinationsOfFactoryFixedWithExternalConfig)
{
    for (const auto &pd : getUsbProbeDataRoyale())
    {
        auto mcf = std::dynamic_pointer_cast<ModuleConfigFactoryFixed> (pd.moduleConfigFactory);
        if (mcf)
        {
            MockBridgeFactory dummyBf;
            auto moduleConfig = mcf->probeAndCreate (dummyBf);
            ASSERT_FALSE (moduleConfig->imagerConfig.externalConfigFileConfig)
                    << "broken assumption that this ModuleConfigFactoryFixed doesn't need ExternalConfig support "
                    << "... in the config for " << moduleConfig->coreConfigData.cameraName;
        }
    }
}

/**
 * If a module can be used as a slave (detected by it declaring an ImagerConfig::Trigger that isn't
 * I2C), then ensure that the slave versions of the use cases can be constructed.
 */
TEST (UnitTestModuleConfig, SlaveUseCases)
{
    using namespace royale::usecase;

    const auto paramFactory = getProcessingParameterMapFactoryRoyale();
    for (const auto &pd : getUsbProbeDataRoyale())
    {
        for (const auto &moduleConfig : pd.moduleConfigFactory->enumerateConfigs())
        {
            auto ccf = CoreConfigFactory (*moduleConfig, paramFactory);
            auto config = ccf();
            if (moduleConfig->imagerConfig.externalTrigger != ImagerConfig::Trigger::I2C)
            {
                for (const auto &uc : config->getSupportedUseCases())
                {
                    ASSERT_NO_THROW (UseCaseSlave ucs (*uc.getDefinition()))
                            << "... in the config for " << moduleConfig->coreConfigData.cameraName;
                }
            }
        }
    }
}

/**
 * If a module can enable flow control, ensure that all use cases are supported by all of the flow
 * control strategies.
 */
TEST (UnitTestModuleConfig, FlowControlStrategies)
{
    using namespace royale::usecase;

    const auto paramFactory = getProcessingParameterMapFactoryRoyale();
    for (const auto &pd : getUsbProbeDataRoyale())
    {
        for (const auto &moduleConfig : pd.moduleConfigFactory->enumerateConfigs())
        {
            auto ccf = CoreConfigFactory (*moduleConfig, paramFactory);
            auto config = ccf();

            for (const auto &flowControl : FlowControlStrategyFactory::enumerateStrategies (config->getBandwidthRequirementCategory()))
            {
                for (const auto &uc : config->getSupportedUseCases())
                {
                    // There isn't a check for valid values here, because it's not obvious which
                    // values should be considered invalid.
                    ASSERT_NO_THROW (flowControl->getRawFrameRate (*uc.getDefinition()))
                            << "... in the config for " << moduleConfig->coreConfigData.cameraName;
                }
            }
        }
    }
}
