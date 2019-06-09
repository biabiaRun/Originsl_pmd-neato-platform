/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <imager/IImagerComponent.hpp>
#include <hal/IBridgeImager.hpp>

#include <map>

namespace royale
{
    namespace imager
    {
        /**
        * This is a base class for imagers using 16 bit registers (address and data).
        * It implements helper methods to read and write them via the IBridgeImager interface
        * and it also adds logging functionality. This base class offers support
        * functions for imagers generating their register configurations. For this reason this
        * class drives from ISoftwareDefinedImagerComponent and flash based imagers for example
        * should not use this class as a base class.
        */
        class ImagerBase : public ISoftwareDefinedImagerComponent
        {
        public:
            explicit ImagerBase (const std::shared_ptr<royale::hal::IBridgeImager> &bridge);
            virtual ~ImagerBase() = default;

            void setLoggingListener (IImageSensorLoggingListener *pListener) override;

            /**
            * Helper for emitting a log message
            */
            void logMessage (ImageSensorLogType logType, const std::string &logMessage);

            /**
            * This provides a default implementation for IImager::writeRegisters that throws a NotImplemented exception.
            * If a specific imager implementation permits direct register access it should override this default
            * implementation and redirect the call to ImagerBase::writeRegistersInternal.
            */
            void writeRegisters (const std::vector<uint16_t> &registerAddresses,
                                 const std::vector<uint16_t> &registerValues) override;

            /**
            * This provides a default implementation for IImager::readRegisters that throws a NotImplemented exception.
            * If a specific imager implementation permits direct register access it should override this default
            * implementation and redirect the call to ImagerBase::readRegistersInternal.
            */
            void readRegisters (const std::vector<uint16_t> &registerAddresses,
                                std::vector<uint16_t> &registerValues) override;

        protected:
            /**
            *  Wrapper for the bridge call that includes logging
            */
            void readImagerRegister (uint16_t regAddr, uint16_t &value);

            /**
            *  Wrapper for the bridge call that includes logging
            */
            void writeImagerRegister (uint16_t regAddr, uint16_t value);

            /**
            *  Wrapper for the bridge call that includes logging
            */
            void writeImagerBurst (uint16_t firstRegAddr, const std::vector<uint16_t> &values);

            void writeRegistersInternal (const std::vector<uint16_t> &registerAddresses,
                                         const std::vector<uint16_t> &registerValues);
            void readRegistersInternal (const std::vector<uint16_t> &registerAddresses,
                                        std::vector<uint16_t> &registerValues);

            /**
            * Track and write register. A call to this method stores the
            * address/value pairs in a map that represents the current state
            * of the settings on the imager hardware. The address/value pairs
            * are stored only if the hardware register write call succeeded.
            *
            * The caller must handle thread safety of the tracked register cache.
            *
            * \param  registers       The register address/value pairs.
            */
            void trackAndWriteRegisters (const std::map < uint16_t, uint16_t > &registers);

            /**
            * Keeps track of written registers only, but does not write them.
            * This method should be used by safe-reconfig imagers which use an
            * indirect write method.
            *
            * This method indicates that commands have been sent to the hardware imager which, if
            * successful, will write the registers given as the argument. When the result is known,
            * there must be a call to commitOrRollbackShadowedRegisters().
            *
            * When this method is called, the values on the hardware are marked as indeterminate by
            * erasing them from the map of tracked registers.
            *
            * The caller must handle thread safety of the tracked register cache.
            *
            * \param  registers       The register address/value pairs.
            */
            void trackShadowedRegisters (const std::map < uint16_t, uint16_t > &registers);

            /**
            * Must be called when the result of an operation that called trackShadowedRegisters() is
            * known.
            *
            * The caller must handle thread safety of the tracked register cache.
            *
            * \param  success If true, the registers were written as expected. If false, the values
            *                 of those registers are now unknown.
            */
            void commitOrRollbackShadowedRegisters (bool success);

            /**
            * Write single register, use tracked value to perform a masked write to preserve the bits
            * defined by the mask, use the reset value if the register wasn't tracked before.
            *
            * \param  address     The 16 bit register address
            * \param  mask        A 16 bit value, zero bits mean preserve, a one means permission to overwrite
            * \param  value       A 16 bit value that should be written to the register
            * \param  resetValue  The 16 bit default value that should be used if the register was not part
            *                     of a tracked write before.
            */
            void maskedWriteRegister (uint16_t address, uint16_t mask, uint16_t value, uint16_t resetValue);

            /**
            * This method compares the current hardware register configuration
            * with the new register values specified in the regChanges parameter.
            * It will return all register/value pairs which are not up-to-date.
            *
            * \param  regChanges   The desired register changes as map of address/value pairs.
            * \return              The address/value pairs that differs from the current hardware configuration.
            */
            std::map < uint16_t, uint16_t > resolveConfiguration (const std::map < uint16_t, uint16_t > &regChanges);

            virtual std::vector < uint16_t > getSerialRegisters() = 0;

            std::shared_ptr<royale::hal::IBridgeImager> m_bridge;
            /**
             * Registers which have been successfully set on the hardware.
             */
            std::map < uint16_t, uint16_t > m_regDownloaded;
            /**
             * The argument to trackShadowedRegisters, which is merged or discarded by
             * commitOrRollbackShadowedRegisters.
             */
            std::map < uint16_t, uint16_t > m_regPendingShadow;
            std::string m_serial;

        private:
            IImageSensorLoggingListener *m_loggingListener;
            static uint32_t s_imagerIdCounter;
            uint32_t m_imagerMyId;

            /**
            *  Helper for emitting a specialized message for register operations
            */
            void logRegister (ImageSensorLogType logType, uint16_t address, uint16_t value);
        };
    }
}
