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

#include <hal/IBridgeImager.hpp>
#include <hal/IBridgeDataReceiver.hpp>
#include <hal/ITemperatureSensor.hpp>
#include <hal/INonVolatileStorage.hpp>
#include <hal/IImager.hpp>
#include <config/ICoreConfig.hpp>
#include <collector/IFrameCollector.hpp>
#include <common/IFlowControlStrategy.hpp>
#include <common/EventQueue.hpp>
#include <device/RoiLensCenter.hpp>

namespace royale
{
    namespace device
    {
        /**
         * This is the main class for controlling the underlying hardware components
         */
        class CameraCore : public royale::IEventListener
        {
        public:

            enum class UpdateComponent : uint16_t
            {
                Imager = 0x1,
                FrameCollector = 0x2,
                All = 0xffff
            };

            /**
            * This constructor is building a CameraCore object based on the given interfaces.
            *
            * The imager passed here is expected to be in ImagerState::Virgin or ImagerState::Ready state,
            * and the CameraCore constructor will reset it (by calling sleep() and initialize() on it).
            * There are two different illumination temperature sensor interfaces, because there are
            * temperature sensor that are independent of the imager and temperature sensors that
            * are read out via the imager. Both sensor types require different utilizations and usage
            * characteristics.
            *
            * \param config CoreConfig for the camera module
            * \param imager Imager for the camera module
            * \param bridgeReceiver IBridgeDataReceiver interface corresponding to the imager
            * \param tempSensor ITemperatureSensor interface for the illumination unit temperature
            * \param storage INonVolatileStorage interface for accessing calibration data (may be nullptr)
            * \param flowControl IFlowControlStrategy (may be nullptr)
            * \param psdTemperatureSensor IPsdTemperatureSensor interface for the illumination unit temperature
            * \param access to get access level
            * \throw InvalidValue if any of the required parameters above is nullptr
            * \throw WrongState if the imager is in the wrong state (e.g. is already in use)
            */
            ROYALE_API CameraCore (
                std::shared_ptr<const royale::config::ICoreConfig> config,
                std::shared_ptr<royale::hal::IImager> imager,
                std::shared_ptr<royale::hal::IBridgeDataReceiver> bridgeReceiver,
                std::shared_ptr<royale::hal::ITemperatureSensor> tempSensor,
                std::shared_ptr<royale::hal::INonVolatileStorage> storage,
                std::shared_ptr<royale::common::IFlowControlStrategy> flowControl,
                std::shared_ptr<royale::hal::IPsdTemperatureSensor> psdTemperatureSensor,
                royale::CameraAccessLevel access);

            ROYALE_API ~CameraCore ();

            /**
             * Prepares the imager for executing an use case (transfer of firmware,
             * base config, and so on). This doesn't mean that the imager is ready to capture
             * afterwards.
             *
             * \throw if the initialization didn't work
             */
            ROYALE_API void initializeImager();


            ROYALE_API void startCapture ();
            ROYALE_API void stopCapture ();

            ROYALE_API bool isCapturing() const;
            ROYALE_API bool isConnected() const;

            /**
             * The interface that the CaptureListener must use to return data buffers to the Camera
             * Access. The pointer will remain valid until the end of the CameraModule's life cycle.
             */
            ROYALE_API royale::collector::IFrameCaptureReleaser *getCaptureReleaser();

            /**
             * Only verifies a use case if it is supported by the CameraModule and Imager, does not apply anything (read-only)
             *
             * \param  useCase const pointer to an allocated use case definition
             * \return SUCCESS if supported, the according error if not supported
             */
            ROYALE_API royale::usecase::VerificationStatus verifyUseCase (const royale::usecase::UseCaseDefinition *useCase);

            /**
             * Sets a new use case and replaces the old one. This is a blocking call and is synchronized w/ the acquisition thread and
             * callback listeners. If the module is in capture mode, capturing is stopped and the new use case is set. Afterwards, the
             * capture mode is set again. This call is optional, the CameraCore is getting initialized by the default simple use case.
             * If the new use case is not accepted, the old one is still active.
             *
             * \param useCase pointer to the use case definition
             * \param component specifies component should be updated, default is all
             * \throw if the use case is not supported, an exception will be thrown
             */
            ROYALE_API void setUseCase (royale::usecase::UseCaseDefinition *useCase, UpdateComponent component = UpdateComponent::All);

            /**
             * Returns the imager serial number as a string which will be used to uniquely identify
             * each camera module
             *
             * \return string containing the serial number stored in the imager
             */
            ROYALE_API std::string getImagerSerialNumber() const;

            /**
             * If this device has calibration data, return the data.
             * \throw if the device does not have calibration data
             */
            ROYALE_API std::vector<uint8_t> getCalibrationData() const;

            /**
             * Flashes a block of calibration data onto the device.
             * The underlying implementation will add the appropriate header
             * in front of the calibration data.
             *
             * \param data data that should be flashed
             * \throw LogicError if the flashing did not work
             */
            ROYALE_API void flashCalibrationData (const std::vector<uint8_t> &data);

            /**
             * Flashes a block of data onto the device.
             *
             * Where the data will be written to is implementation defined. After using this
             * function, the eye safety of the device is not guaranteed, even after reopening the
             * device with L1 access.  This method may overwrite the product identifier, and
             * possibly even the firmware.
             *
             * \param data data that should be flashed
             * \throw LogicError if the flashing did not work
             */
            ROYALE_API void flashData (const std::vector<uint8_t> &data);

            /**
             * Change the exposure times of all exposure groups at once.
             *
             * As the imager does not know about exposure groups and expects exposures
             * in RFS order, the current usecase is consulted for the mapping.
             *
             * \param useCase the currently executing usecase
             * \param exposureTimes the new exposure times, in exposure group order
             */
            ROYALE_API void reconfigureImagerExposureTimes (const royale::usecase::UseCaseDefinition *useCase, const std::vector<uint32_t> &exposureTimes);

            /**
             * Change the target frame rate.
             *
             * \param targetFrameRate the new raw frame rate
             */
            ROYALE_API void reconfigureImagerTargetFrameRate (uint16_t targetFrameRate);

            /*!
             * Set the relative offset of the lens given in pixel coordinates. The image size which is
             * specified in the UseCaseDefinition is centered around the module config's design lens center
             * adjusted by this offset value. This correction value is most probably changed after the calibration
             * is read. If this method is not called, the lens center is assumed to be module config's design center.
             *
             * The new lens center will be used in verifyUseCase and activated in setUseCase().  The
             * new lens center may make verifyUseCase fail, but this will only be tested when
             * verifyUseCase is called.
             *
             * \param pixelColumn relative correction value for the lens center (X)
             * \param pixelRow    relative correction value for the lens center (Y)
             */
            ROYALE_API void setLensOffset (int16_t pixelColumn, int16_t pixelRow);

            ROYALE_API void setCaptureListener (royale::collector::IFrameCaptureListener *listener);

            /**
             * Uses the active imager to call the underlying bridge method for writing
             * a single register for each element of the Vector.
             *
             * \param   registers   Contains elements of possibly not-unique (String, uint64_t) duplets.
             *                      The String component may contain:
             *                      a) a base-10 decimal number
             *                      b) a base-16 hexadecimal number preceded by a "0x"
             *                      c) a string identifier that is known by the concrete imager implementation
             *                      The concrete implementation may restrict the integer type size by using narrow_casts.
             */
            ROYALE_API void writeRegisters (const royale::Vector<royale::Pair<royale::String, uint64_t>> &registers);

            /**
             * Uses the active imager to call the underlying bridge method for reading
             * a single register for each element of the Vector.
             *
             * \param   registers   Contains elements of possibly not-unique (String, uint64_t) duplets.
             *                      The String component may contain:
             *                      a) a base-10 decimal number
             *                      b) a base-16 hexadecimal number preceded by a "0x"
             *                      c) a string identifier that is known by the concrete imager implementation
             *                      The concrete implementation may restrict the integer type size by using narrow_casts.
             */
            ROYALE_API void readRegisters (royale::Vector<royale::Pair<royale::String, uint64_t>> &registers);

            /*!
             * Set listener for events generated by the various components held by CameraCore.
             * The onEvent() callback in IEventListener is invoked asynchronously and serialized.
             * Only one listener can be active at any time. Call with listener==nullptr to deregister.
             *
             * \param listener Callback interface (or nullptr)
             */
            ROYALE_API void setEventListener (royale::IEventListener *listener);

            /**
             * Retrieve extended information about the module. This gives information about all the components
             * that are part of the core (e.g. bridge and storage).
             *
             * \return A vector of string pairs that gives keys with corresponding values
             */
            ROYALE_API royale::Vector<royale::Pair<royale::String, royale::String>> getCoreInfo();

            /**
             * Add information to the vector that is returned by getCoreInfo.
             *
             * \param info Key/Value pair of information that should be added
             */
            ROYALE_API void addToCoreInfo (const royale::Pair<royale::String, royale::String> &info);

            /**
             * Enable or disable the use of the external trigger.
             * The external trigger has to be defined in the ModuleConfig!
             *
             * \param   useExternalTrigger   Enable/Disable external trigger.
             */
            ROYALE_API void setExternalTrigger (bool useExternalTrigger);

            void onEvent (std::unique_ptr<royale::IEvent> &&event) override;

        private:
            /**
             * Is called by all constructors
             */
            void init ();

            /**
             * Retrieve extended information about the module. This depends on the implementation
             * of the bridge that is used.
             *
             * \return A vector of string pairs that gives keys with corresponding values
             */
            royale::Vector<royale::Pair<royale::String, royale::String>> getBridgeInfo();

            /**
             * Retrieve information about the storage of the device (e.g. module identifier, module suffix, ...)
             *
             * \return A vector of string pairs that gives keys with corresponding values
             */
            royale::Vector<royale::Pair<royale::String, royale::String>> getStorageInfo();

        protected:
            std::shared_ptr<const royale::config::ICoreConfig> m_config;
            std::shared_ptr<royale::hal::INonVolatileStorage> m_storage;
            std::shared_ptr<royale::hal::IPsdTemperatureSensor> m_psdTemperatureSensor;
            std::shared_ptr<royale::hal::ITemperatureSensor> m_temperatureSensor;
            std::shared_ptr<royale::hal::IBridgeDataReceiver> m_bridgeReceiver;
            std::shared_ptr<royale::hal::IImager> m_imager;
            std::unique_ptr<royale::collector::IFrameCollector> m_frameCollector;
            std::shared_ptr<royale::common::IFlowControlStrategy> m_flowControl;
            std::unique_ptr<royale::device::RoiLensCenter> m_roiLensCenter;

            /**
             * For the event handling the CameraCore uses two EventQueue objects.
             * m_eventQueueInt receives the events from the components and calls the
             * onEvent method of the CameraCore for processing of some of the events.
             * Afterwards the event is sent to m_eventQueueExt which is the connection
             * to a possible external event listener.
             */
            EventQueue m_eventQueueInt;
            EventQueue m_eventQueueExt;

            bool m_isCapturing;

            royale::Vector<royale::Pair<royale::String, royale::String>> m_additionalCoreInfo;
            royale::CameraAccessLevel m_access;
        };
    }
}
