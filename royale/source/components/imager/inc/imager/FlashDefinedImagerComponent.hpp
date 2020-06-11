/****************************************************************************\
 * Copyright (C) 2019 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <imager/IImagerComponent.hpp>
#include <imager/ImagerRegisterAccess.hpp>
#include <imager/ImagerParameters.hpp>
#include <vector>

namespace royale
{
    namespace imager
    {
        /**
        * Information about the current design step
        */
        struct DesignStepInfo
        {
            uint16_t ANAIP_DESIGNSTEP_Address;  // Address of the register where to read the design step
            std::vector<uint16_t> designSteps;  // The correct design step values
        };

        /**
        * Information about the registers used to configure the exposure time
        */
        struct ExpoTimeRegInfo
        {
            size_t nSequenceEntries;
            uint16_t CFGCNT_S00_EXPOTIME_Address;
            uint16_t CFGCNT_S01_EXPOTIME_Address;
            uint16_t CFGCNT_FLAGS_Address;
        };

        /**
          * This specialized interface is used by software imagers taking the imager configuration
          * from a flash memory device. Such imagers will initiate a transfer from the flash to
          * the hardware imager based on the given use case identifier.
          */
        class FlashDefinedImagerComponent : public IImagerComponent
        {
        public:
            /**
            * The lifecycle of the bridge and config object must be longer than the Imager.
            * This imager uses an external configuration, thus the externalConfig member of
            * the ImagerParameters parameter must point to a valid external configuration.
            *
            * \param   params   Set of parameters that have to be passed to a concrete imager object.
            */
            IMAGER_EXPORT explicit FlashDefinedImagerComponent (const ImagerParameters &params);
            ~FlashDefinedImagerComponent() override = default;

            void initialize() override;
            std::string getSerialNumber() override;
            void startCapture() override;
            uint16_t stopCapture() override;
            void sleep() override;
            void wake() override;
            void reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex) override;
            void reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex) override;
            std::vector<std::size_t> getMeasurementBlockSizes() const override;
            void setLoggingListener (IImageSensorLoggingListener *pListener) override;
            void setExternalTrigger (bool useExternalTrigger) override;

            /**
            * Verification of the use case. This only tells the caller that the imager is able to execute the
            * use case but it does not perform any interaction with the hardware.
            *
            * \param   useCaseIdentifier   A string based identifier (format is defined by the concrete imager).
            * \return  SUCCESS if use case is supported, the according verification status otherwise.
            */
            IMAGER_EXPORT ImagerVerificationStatus verifyUseCase (const ImagerUseCaseIdentifier &useCaseIdentifier);

            /**
            * Executes the use case. This takes the use case identifier and uses it to locate the
            * corresponding register configuration. The caller is responsive for bringing the imager into a
            * reconfigurable state by calling stopCapture() before.
            * A transmission of the register configuration will be initiated using the IBridgeImager interface.
            *
            * \param   useCaseIdentifier   A string based identifier (format is defined by the concrete imager).
            */
            IMAGER_EXPORT void executeUseCase (const ImagerUseCaseIdentifier &useCaseIdentifier);

        protected:
            virtual uint16_t calcRegExposure (uint32_t expoTime, uint32_t modfreq) const;
            const IImagerExternalConfig::UseCaseData &getUseCaseDataByIdentifier (const ImagerUseCaseIdentifier &useCaseIdentifier) const;

            /**
            * Check whether a configuration change is still pending on the imager during capturing.
            *
            * This should be checked before making changes to the CFGCNT registers while the imager is capturing;
            * it reads the CFGCNT_Flags register. Changes are only safely possible when this returns false.
            *
            * \param  CFGCNT_FLAGS_Address Address of the CFGCNT_FLAGS register
            * \return true if a config change is pending
            */
            bool configChangePending (uint16_t CFGCNT_FLAGS_Address);

            /**
            * Trigger imager reconfiguration.
            *
            * This sets config_changed in CFGCNT_Flags, which causes the imager to reconfigure while capturing according to the
            * Sxx_EXPOTIME and Sxx_FRAMETIME registers.
            * The value returned is the 12 bit reconfig counter which is also delivered in the pseudodata; the imager increments
            * this value to signal that the config change will be valid in the next measurement. This means the first frame with
            * the new counter value in the pseudodata would still be captured with the old settings.
            * This is the same mechanism as used for M2452 safe reconfig.
            *
            * Should only be called when configChangePending() returned false and the imager is capturing.
            *
            * \param  CFGCNT_FLAGS_Address Address of the CFGCNT_FLAGS register
            * \return the configuration change counter
            */
            uint16_t triggerConfigChange (uint16_t CFGCNT_FLAGS_Address);

            /**
            * Trigger usecase reconfiguration.
            *
            * This sets use_case_changed in CFGCNT_Flags, which causes the imager to reconfigure the usecase while capturing.
            * Used for changing overall frame rates (as opposed to raw frame rates or exposure times).
            * The value returned is the 12 bit reconfig counter which is also delivered in the pseudodata; the imager increments
            * this value to signal that the config change will be valid in the next measurement. This means the first frame with
            * the new counter value in the pseudodata would still be captured with the old settings.
            * This is the same mechanism as used for M2452 safe reconfig.
            *
            * Should only be called when configChangePending() returned false and the imager is capturing.
            *
            * \param  CFGCNT_FLAGS_Address Address of the CFGCNT_FLAGS register
            * \return the configuration change counter
            */
            uint16_t triggerUseCaseChange (uint16_t CFGCNT_FLAGS_Address);

            /**
            * Get the information about the current design step and where to read it
            * Must be implemented by each derivation
            * \return The design step information struct
            */
            virtual DesignStepInfo getDesignStepInfo() = 0;

            /**
            * Read the EFUSEVAL registers.
            */
            virtual std::vector <uint16_t> getSerialRegisters() = 0;

            /**
            * Get the information about the registers used to configure exposure time
            * Must be implemented by each derivation
            * \return The exposure time register information struct
            */
            virtual ExpoTimeRegInfo getExpoTimeRegInfo() = 0;

            /**
            * Initiates a transfer of the use case register configuration into the configuration container
            * of the imager.
            *
            * \param   useCaseData    A valid flashConfigAddress and flashConfigSize must be provided using this parameter
            */
            virtual void doFlashToImagerUseCaseTransfer (const IImagerExternalConfig::UseCaseData &useCaseData) = 0;

            std::shared_ptr<royale::hal::IBridgeImager> m_bridge;
            std::unique_ptr<ImagerRegisterAccess> m_registerAccess;
            ImagerState m_imagerState;
            std::shared_ptr<const IImagerExternalConfig> m_externalConfig;
            ImagerUseCaseIdentifier m_executingUseCaseIdentifier;
            uint16_t m_cachedConfigChangeCounter;
            uint16_t m_cachedUseCaseChangeCounter;
            bool m_usesInternalCurrentMonitor;

            /**
            * Clock to use internally for computing pauses.
            *
            * \todo ROYAL-3508 Using system_clock instead of steady_clock here due to broken chrono implementation in MSVC 2013
            */
            using WaitClockType = std::chrono::system_clock;

            /**
            * The time the imager was last stopped. This is used to determine when it is (eye-)safe to start capturing again.
            */
            std::chrono::time_point<WaitClockType> m_lastStopTime;

        };
    }
}
