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

#include <common/IPseudoDataInterpreter.hpp>
#include <usecase/UseCaseDefinition.hpp>
#include <cstddef>

namespace royale
{
    namespace hal
    {
        /**
         * This interface provides the main functionality of the system's imager component.
         */
        class IImager
        {
        public:
            ROYALE_API virtual ~IImager() = default;

            /**
             * Brings the imager out of reset, so that its registers can be read and written.
             * This does not upload any firmware, and functions other than reading/writing registers will throw.
             * To properly initialize the imager, initialize() must be called.
             * It is mandatory to call this method before calling any other method (besides setExternalTrigger)
             * of this interface.
             */
            ROYALE_API virtual void wake() = 0;

            /**
             * Prepares the imager for the execution of use cases, e.g. by
             * sending a base configuration and an imager firmware to the imager silicon.
             */
            ROYALE_API virtual void initialize() = 0;

            /**
             * Starts imaging by setting the imagers start trigger bit. If configured correctly this will
             * result in an image stream on the imagers interface. Any problem that can be checked via
             * the imager itself is tested by this method and an exception will be raised. However, it is not
             * guaranteed that an image stream is reaching the bridge because of possible error sources of
             * the underlying hardware signal path.
             */
            ROYALE_API virtual void startCapture() = 0;

            /**
             * Change the exposure times while the imager is capturing.
             *
             * \param  exposureTimes    New exposure times (one entry per raw frame set)
             * \param  reconfigIndex    Returns the frame number of the last frame or an index that marks the last configuration,
             *                          either can be used to recognize frames captured with the old exposure time configuration
             */
            ROYALE_API virtual void reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex) = 0;

            /**
             * Change the target frame rate while the imager is capturing.
             *
             * \param  targetFrameRate  New target frame rate
             * \param  reconfigIndex    Returns the frame number of the last frame or an index that marks the last configuration,
             *                          either can be used to recognize frames captured with the old exposure time configuration
             */
            ROYALE_API virtual void reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex) = 0;

            /**
             * Stops imaging by setting the imager's stop trigger bit.
             *
             * This is a synchronous function, it will wait for the imager to stop before returning. But
             * buffered image data may still be transferring after this function returns.
             *
             * \return the frame number of the last frame before the imager stopped
             */
            ROYALE_API virtual void stopCapture() = 0;

            /**
             * Puts the imager into the power down mode.
             *
             * This method must be called before destroying the imager object.
             */
            ROYALE_API virtual void sleep() = 0;

            /**
             * Verification of the use case. This only tell the caller that the imager is able to execute the
             * use case but it does not perform any interaction with the hardware.
             *
             * \param   useCase   The use case definition.
             * \param   roiCMin   The zero-based index of the first active column
             * \param   roiRMin   The zero-based index of the first active row
             * \param   flowControlRate   If non-zero, enables more time for the readout phase
             * \return  SUCCESS if use case is supported, the according verification status otherwise.
             */
            ROYALE_API virtual royale::usecase::VerificationStatus verifyUseCase (const royale::usecase::UseCaseDefinition &useCase, uint16_t roiCMin, uint16_t roiRMin, uint16_t flowControlRate) = 0;

            /**
             * Executes the use case. This takes the use case descriptive information and translates it into
             * a register configuration. The caller is responsive for bringing the imager into a
             * reconfigurable state by calling stopCapture() before.
             * The new configuration will be transmitted to the imager using the IBridgeImager interface.
             * \param   useCase   The use case definition.
             * \param   roiCMin   The zero-based index of the first active column
             * \param   roiRMin   The zero-based index of the first active row
             * \param   flowControlRate   If non-zero, enables more time for the readout phase
             */
            ROYALE_API virtual void executeUseCase (const royale::usecase::UseCaseDefinition &useCase, uint16_t roiCMin, uint16_t roiRMin, uint16_t flowControlRate) = 0;

            /**
             * The Use Case passed to executeUseCase() is split in to a sequence of measurement
             * blocks, and the exact distribution affects the size of superframes when using
             * TransmissionMode::SUPERFRAME.  This method returns an ordered list of the number of
             * frames in each measurement block.
             */
            ROYALE_API virtual royale::Vector<std::size_t> getMeasurementBlockSizes() const = 0;

            /**
             * Gets imager serial number.
             * \return  The imager serial number.
             */
            ROYALE_API virtual std::string getSerialNumber() = 0;

            /**
             * Creates a pseudo data interpreter based on actual imager class object.
             */
            ROYALE_API virtual std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter() = 0;

            /**
             * Directly calls the underlying bridge method for writing a single register for each element
             * of the Vector.
             * \param   registers   Contains elements of possibly not-unique (String, uint64_t) duplets.
             *                      The String component can consist of:
             *                      a) a base-10 decimal number
             *                      b) a base-16 hexadecimal number preceded by a "0x"
             *                      c) a string identifier that is known by the concrete imager implementation
             *                      The concrete implementation may restrict the integer type size by using narrow_casts.
             */
            ROYALE_API virtual void writeRegisters (const royale::Vector<royale::Pair<royale::String, uint64_t>> &registers) = 0;

            /**
             * Directly calls the underlying bridge method for reading a single register for each element
             * of the Vector.
             * \param   registers   Contains elements of possibly not-unique (String, uint64_t) duplets.
             *                      The String component can consist of:
             *                      a) a base-10 decimal number
             *                      b) a base-16 hexadecimal number preceded by a "0x"
             *                      c) a string identifier that is known by the concrete imager implementation
             *                      The concrete implementation may restrict the integer type size by using narrow_casts.
             */
            ROYALE_API virtual void readRegisters (royale::Vector<royale::Pair<royale::String, uint64_t>> &registers) = 0;

            /**
             * Enable/Disable the use of the external trigger.
             * This function should only be called when the imager is in the virgin, power down or
             * power up state.
             * The setting becomes active as soon as the capturing is started.
             * If the external trigger defined in the ModuleConfig is also I2C, calling this
             * function with useExternalTrigger set to true will throw an InvalidValue exception.
             */
            ROYALE_API virtual void setExternalTrigger (bool useExternalTrigger) = 0;
        };
    }
}
