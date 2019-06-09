/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <imager/IImageSensorLoggingListener.hpp>
#include <common/IPseudoDataInterpreter.hpp>
#include <imager/ImagerUseCaseDefinition.hpp>
#include <imager/ImagerUseCaseIdentifier.hpp>

#include <memory>

namespace royale
{
    namespace imager
    {
        /**
        * If a use case is verified and fails verification, an error flag can be set.
        *
        * This enum lists different reasons why use cases cannot be accepted.
        * The ISoftwareDefinedImagerComponent and IFlashDefinedImagerComponent
        * interface provides methods for executing and verifying
        * use cases. It is optional to verify use cases before executing them.
        * In contrast to using the executeUseCase calls doing a verifyUseCase call
        * will provide the caller with a specific reason if it is not feasible to
        * run the use case.
        */
        enum class ImagerVerificationStatus
        {
            SUCCESS = IMG_ENUM,   //!< It is feasible to run the use case
            FRAMERATE,            //!< The timing of the use case is not feasible
            PHASE,                //!< A measurement has a not feasible phase setting
            DUTYCYCLE,            //!< A measurement has a not feasible duty cycle setting
            MODULATION_FREQUENCY, //!< A measurement has a not feasible modulation frequency setting
            EXPOSURE_TIME,        //!< A measurement has a not feasible expsure time setting
            REGION,               //!< A measurement has a not feasible region setting
            STREAM_COUNT,         //!< The requested stream count is not supported
            USECASE_IDENTIFIER,   //!< The provided use case identifier is unkown or points to an unknown location
            FLASH_CONFIG,         //!< The provided use case contains an invalid flash based configuration
            SEQUENCER             //!< The given set of measurements cannot be assigned to the hardware imager's sequencer unit
        };

        /**
        * This enum can be used by software imagers to keep track of the state
        * of the underlying hardware imager.
        */
        enum class ImagerState : uint16_t
        {
            Virgin = IMG_ENUM,
            PowerDown,
            PowerUp,
            Ready,
            Capturing,
        };

        /**
        * This specialized interface is used by software imagers creating the imager configuration
        * on their own, based on the input object of type ImagerUseCaseDefinition.
        */
        class ISoftwareDefinedImagerComponent
        {
        public:
            virtual ~ISoftwareDefinedImagerComponent() = default;
            /**
            * Verification of the use case. This only tells the caller that the imager is able to execute the
            * use case but it does not perform any interaction with the hardware.
            *
            * \param   useCase   The use case definition.
            * \return  SUCCESS if use case is supported, the according verification status otherwise.
            */
            virtual ImagerVerificationStatus verifyUseCase (const ImagerUseCaseDefinition &useCase) = 0;

            /**
            * Executes the use case. This takes the use case descriptive information and translates it into
            * a register configuration. The caller is responsive for bringing the imager into a
            * reconfigurable state by calling stopCapture() before.
            * The new configuration will be transmitted to the imager using the IBridgeImager interface.
            *
            * \param   useCase   The use case definition.
            */
            virtual void executeUseCase (const ImagerUseCaseDefinition &useCase) = 0;


            virtual void wake() = 0;

            /**
            * Prepares the imager for the execution of use cases, e.g. by
            * sending a base configuration and an imager firmware to the imager silicon.
            */
            virtual void initialize() = 0;

            /**
            * Starts imaging by setting the imager's start trigger bit. If configured correctly this will
            * result in an image stream on the imagers interface. Any problem that can be checked via
            * the imager itself is tested by this method and an exception will be raised. However, it is not
            * guaranteed that an image stream is reaching the bridge because of possible error sources of
            * the underlying hardware signal path.
            */
            virtual void startCapture() = 0;

            /**
            * Change the exposure times while the imager is capturing.
            *
            * The number and order of exposure times expected here is defined by the imager implementation.
            * For the software defined imagers it's generally one entry per raw frame set (which is available as
            * part of the UseCaseDefinition).
            * For the flash defined imagers (e.g. MiraBelle) the use cases are defined by register configuration
            * in the flash, and the corresponding mappings need to be maintained separately (e.g. as part of the
            * royale-specific description of the imager configs which might also be stored in the flash).
            *
            * \param  exposureTimes    New exposure times
            * \param  reconfigIndex    Returns the frame number of the last frame or an index that marks the last configuration,
            *                          either can be used to recognize frames captured with the old exposure time configuration
            */
            virtual void reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex) = 0;

            /**
            * Change the target frame rate while the imager is capturing.
            *
            * \param  targetFrameRate  New frame rate
            * \param  reconfigIndex    Returns the frame number of the last frame or an index that marks the last configuration,
            *                          either can be used to recognize frames captured with the old exposure time configuration
            */
            virtual void reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex) = 0;

            /**
            * Stops imaging by setting the imager's stop trigger bit.
            *
            * This is a synchronous function, it will wait for the imager to stop before returning. But
            * buffered image data may still be transferring after this function returns.
            */
            virtual void stopCapture() = 0;

            /**
            * Puts the imager into the power down mode.
            *
            * This method must be called before destroying the imager object.
            */
            virtual void sleep() = 0;

            /**
            * The Use Case passed to executeUseCase() is split in to a sequence of measurement
            * blocks, and the exact distribution affects the size of superframes when using
            * TransmissionMode::SUPERFRAME.  This method returns an ordered list of the number of
            * frames in each measurement block.
            */
            virtual std::vector<std::size_t> getMeasurementBlockSizes() const = 0;

            /**
            * Gets imager serial number.
            * \return  The imager serial number.
            */
            virtual std::string getSerialNumber() = 0;

            /**
            * The callback to the Royale API layer, called whenever useful information should be logged
            */
            virtual void setLoggingListener (IImageSensorLoggingListener *pListener) = 0;

            /**
            * Creates a pseudo data interpreter based on actual imager class object.
            */
            virtual std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter() = 0;

            /**
            * Directly calls the underlying bridge method for writing a single register for each element
            * of the vector. If the registerAddresses are continuous a burst write (IBridgeImager::writeImagerBurst)
            * will be issued, otherwise a standard write will be used for each address/value pair.
            *
            * \param   registerAddresses   A list of integer elements, each value represents a possibly not-unique register address.
            * \param   registerValues      A list of integer elements, each value will be written to the address defined
            *                              by the the registerAddresses vector element that has the same index as the
            *                              integer vector element.
            *                              The concrete implementation may restrict the integer type size by using narrow_casts.
            */
            virtual void writeRegisters (const std::vector<uint16_t> &registerAddresses,
                                         const std::vector<uint16_t> &registerValues) = 0;

            /**
            * Directly calls the underlying bridge method for reading a single register for each element
            * of the vector. If the registerAddresses are continuous a burst read (IBridgeImager::readImagerBurst)
            * will be issued, otherwise a standard read will be used for each address/value pair.
            *
            * \param   registerAddresses   A list of integer elements, each value represents a possibly not-unique register address.
            * \param   registerValues      A list of integer elements, each value will be read from the address defined
            *                              by the the registerAddresses vector element that has the same index as the
            *                              integer vector element.
            *                              The concrete implementation may restrict the integer type size by using narrow_casts.
            */
            virtual void readRegisters (const std::vector<uint16_t> &registerAddresses,
                                        std::vector<uint16_t> &registerValues) = 0;

            /**
            * Enable/Disable the use of the external trigger.
            * This function should only be called when the imager is in the virgin, power down or
            * power up state.
            * The setting becomes active as soon as the capturing is started.
            * If the external trigger defined in the ModuleConfig is also I2C, calling this
            * function with useExternalTrigger set to true will throw an InvalidValue exception.
            */
            virtual void setExternalTrigger (bool useExternalTrigger) = 0;


        };

        /**
        * This specialized interface is used by software imagers taking the imager configuration
        * from a flash memory device. Such imagers will initiate a transfer from the flash to
        * the hardware imager based on the given use case identifier.
        */
        class IFlashDefinedImagerComponent
        {
        public:
            virtual ~IFlashDefinedImagerComponent() = default;
            /**
            * Verification of the use case. This only tells the caller that the imager is able to execute the
            * use case but it does not perform any interaction with the hardware.
            *
            * \param   useCaseIdentifier   A string based identifier (format is defined by the concrete imager).
            * \return  SUCCESS if use case is supported, the according verification status otherwise.
            */
            virtual ImagerVerificationStatus verifyUseCase (const ImagerUseCaseIdentifier &useCaseIdentifier) = 0;

            /**
            * Executes the use case. This takes the use case identifier and uses it to locate the
            * corresponding register configuration. The caller is responsive for bringing the imager into a
            * reconfigurable state by calling stopCapture() before.
            * A transmission of the register configuration will be initiated using the IBridgeImager interface.
            *
            * \param   useCaseIdentifier   A string based identifier (format is defined by the concrete imager).
            */
            virtual void executeUseCase (const ImagerUseCaseIdentifier &useCaseIdentifier) = 0;

            /**
            * Brings the imager out of reset, so that its registers can be read and written.
            * This does not upload any firmware, and functions other than reading/writing registers will throw.
            * To properly initialize the imager, initialize() must be called.
            * It is mandatory to call this method before calling any other method (besides setExternalTrigger)
            * of this interface.
            */
            virtual void wake() = 0;

            /**
            * Prepares the imager for the execution of use cases, e.g. by
            * sending a base configuration and an imager firmware to the imager silicon.
            */
            virtual void initialize() = 0;

            /**
            * Starts imaging by setting the imager's start trigger bit. If configured correctly this will
            * result in an image stream on the imagers interface. Any problem that can be checked via
            * the imager itself is tested by this method and an exception will be raised. However, it is not
            * guaranteed that an image stream is reaching the bridge because of possible error sources of
            * the underlying hardware signal path.
            */
            virtual void startCapture() = 0;

            /**
            * Change the exposure times while the imager is capturing.
            *
            * The number and order of exposure times expected here is defined by the imager implementation.
            * For the software defined imagers it's generally one entry per raw frame set (which is available as
            * part of the UseCaseDefinition).
            * For the flash defined imagers (e.g. MiraBelle) the use cases are defined by register configuration
            * in the flash, and the corresponding mappings need to be maintained separately (e.g. as part of the
            * royale-specific description of the imager configs which might also be stored in the flash).
            *
            * \param  exposureTimes    New exposure times
            * \param  reconfigIndex    Returns the frame number of the last frame or an index that marks the last configuration,
            *                          either can be used to recognize frames captured with the old exposure time configuration
            */
            virtual void reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex) = 0;

            /**
            * Change the target frame rate while the imager is capturing.
            *
            * \param  targetFrameRate  New frame rate
            * \param  reconfigIndex    Returns the frame number of the last frame or an index that marks the last configuration,
            *                          either can be used to recognize frames captured with the old exposure time configuration
            */
            virtual void reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex) = 0;

            /**
            * Stops imaging by setting the imager's stop trigger bit.
            *
            * This is a synchronous function, it will wait for the imager to stop before returning. But
            * buffered image data may still be transferring after this function returns.
            *
            * \return the frame number of the last frame before the imager stopped
            */
            virtual uint16_t stopCapture() = 0;

            /**
            * Puts the imager into the power down mode.
            *
            * This method must be called before destroying the imager object.
            */
            virtual void sleep() = 0;

            /**
            * The Use Case passed to executeUseCase() is split in to a sequence of measurement
            * blocks, and the exact distribution affects the size of superframes when using
            * TransmissionMode::SUPERFRAME.  This method returns an ordered list of the number of
            * frames in each measurement block.
            */
            virtual std::vector<std::size_t> getMeasurementBlockSizes() const = 0;

            /**
            * Gets imager serial number.
            * \return  The imager serial number.
            */
            virtual std::string getSerialNumber() = 0;

            /**
            * The callback to the Royale API layer, called whenever useful information should be logged
            */
            virtual void setLoggingListener (IImageSensorLoggingListener *pListener) = 0;

            /**
            * Creates a pseudo data interpreter based on actual imager class object.
            */
            virtual std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter() = 0;

            /**
            * Enable/Disable the use of the external trigger.
            * This function should only be called when the imager is in the virgin, power down or
            * power up state.
            * The setting becomes active as soon as the capturing is started.
            * If the external trigger defined in the ModuleConfig is also I2C, calling this
            * function with useExternalTrigger set to true will throw an InvalidValue exception.
            */
            virtual void setExternalTrigger (bool useExternalTrigger) = 0;
        };
    }
}
