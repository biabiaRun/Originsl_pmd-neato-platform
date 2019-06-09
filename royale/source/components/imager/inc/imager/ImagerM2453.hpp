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

#include <imager/IImagerComponent.hpp>
#include <imager/ImagerParameters.hpp>
#include <imager/ImagerRegisterAccess.hpp>
#include <imager/ImagerSimpleHexSerialNumber.hpp>
#include <imager/ImagerUseCaseIdentifier.hpp>

#include <chrono>
#include <iomanip>
#include <string>
#include <vector>

#include <royale/Vector.hpp>
#include <royale/Pair.hpp>

namespace royale
{
    namespace imager
    {
        using ImagerM2453Serial = ImagerSimpleHexSerialNumber;

        /**
        * Abstract base class for M2453 imagers, currently also used for the M2455.
        */
        class ImagerM2453 : public IFlashDefinedImagerComponent
        {
        public:
            /**
            * The lifecycle of the bridge and config object must be longer than the Imager.
            * This imager uses an external configuration, thus the externalConfig member of
            * the ImagerParameters parameter must point to a valid external configuration.
            *
            * \param   params   Set of parameters that have to be passed to a concrete imager object.
            */
            IMAGER_EXPORT explicit ImagerM2453 (const ImagerParameters &params);
            ~ImagerM2453() override = default;

            void initialize() override;
            void sleep() override;
            void wake() override;
            std::vector<std::size_t> getMeasurementBlockSizes() const override;
            std::string getSerialNumber() override;
            void startCapture() override;
            void reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex) override;
            void reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex) override;
            uint16_t stopCapture() override;
            virtual std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter() override = 0;
            void setLoggingListener (IImageSensorLoggingListener *pListener) override;
            void setExternalTrigger (bool useExternalTrigger) override;
            ImagerVerificationStatus verifyUseCase (const ImagerUseCaseIdentifier &useCaseIdentifier) override;
            void executeUseCase (const ImagerUseCaseIdentifier &useCaseIdentifier) override;

        protected:
            virtual uint16_t calcRegExposure (uint32_t expoTime, uint32_t modfreq) const;

            std::shared_ptr<royale::hal::IBridgeImager> m_bridge;
            std::unique_ptr<ImagerRegisterAccess> m_registerAccess;

        protected:
            ImagerState m_imagerState;
            std::shared_ptr<const IImagerExternalConfig> m_externalConfig;
            uint32_t m_systemFrequency;
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

            /**
            * Read the EFUSEVAL registers. These have moved in design stepping B11.
            */
            virtual std::vector < uint16_t > getSerialRegisters() = 0;

            const TimedRegisterList &getTimedRegisterMapByUseCaseIdentifier (const ImagerUseCaseIdentifier &useCaseIdentifier);

            const IImagerExternalConfig::UseCaseData &getUseCaseDataByIdentifier (const ImagerUseCaseIdentifier &useCaseIdentifier) const;

            /**
            * Check whether a configuration change is still pending on the imager during capturing.
            *
            * This should be checked before making changes to the CFGCNT registers while the imager is capturing;
            * it reads the CFGCNT_Flags register. Changes are only safely possible when this returns false.
            *
            * \return true if a config change is pending
            */
            bool configChangePending();

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
            * \return the configuration change counter
            */
            uint16_t triggerConfigChange();

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
            * \return the configuration change counter
            */
            uint16_t triggerUseCaseChange();

            /**
            * Initiates a transfer of the use case register configuration into the configuration container
            * of the imager.
            *
            * \param   useCaseData    A valid flashConfigAddress and flashConfigSize must be provided using this parameter
            */
            virtual void doFlashToImagerUseCaseTransfer (const IImagerExternalConfig::UseCaseData &useCaseData);

            /**
            * Initiates a SPI transfer from an attached flash memory into the imager configuration container.
            *
            * \param   flashAddress   A 24 bit address of the flash memory the read operation should start from
            * \param   cfgcntOffset   The register offset relative to CFGCNT the write operation should start
            * \param   payloadSize    The count of 16 bit words to transfer
            */
            void doFlashToImagerBlockTransfer (uint32_t flashAddress, uint16_t cfgcntOffset, size_t payloadSize);
        };
    }
}
