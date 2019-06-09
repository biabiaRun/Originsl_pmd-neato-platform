/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <buffer/BridgeInternalBufferAlloc.hpp>
#include <buffer/BufferDataFormat.hpp>
#include <usb/pal/UsbSpeed.hpp>

#include <common/EventForwarder.hpp>

#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#pragma once

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            /**
             * Amundsen is a UVC-like but vendor-specific protocol.  Devices running Amundsen
             * firmware don't identify themselves as UVC, so that the device isn't claimed by any
             * OS-provided camera server. Amundsen devices have all of their endpoints on a single
             * interface, instead of having separate interfaces for control and data.
             *
             * The protocol is similar to a subset of UVC, the host-side BridgeAmundsen can be used
             * with both Amundsen and UVC firmware.  This may change in future, as with Amundsen the
             * host knows exactly which cameras are supported, and we can change the protocol to
             * suit them.
             *
             * Similarly to the BridgeUvc, control of the I2C, SPI and GPIOs is handled by the
             * Arctic protocol, working via an IUvcExtensionAccess.
             *
             * Naming: this is named after the Arctic Ocean's Amundsen Basin, which is itself named
             * after the first explorer who reached both poles and was the first to reach the South
             * Pole.
             */
            class BridgeAmundsenCommon : public royale::buffer::BridgeInternalBufferAlloc
            {
            public:
                /**
                 * How long each USB command is given, in milliseconds.  This is the time for each
                 * USB read or write, not the total time between the command and the respective
                 * check for errors.
                 */
                static const unsigned int AMUNDSEN_TIMEOUT = 2000;

                /**
                 * I/O timeout for reading the image raw frames' data.
                 */
                static const unsigned int AMUNDSEN_DATA_TIMEOUT = 500;

                /**
                 * The protocol is based on receiving an image split in to multiple payloads, each
                 * individual payload fits in to this number of bytes.  The function calls between
                 * BridgeAmundsenCommon and the platform-dependent subclasses assume this fixed
                 * value, and omit the arguments that would be necessary if it was variable.
                 *
                 * The existing FX3 Arctic firmware uses maximum payloads of almost 16kB (it uses
                 * 0x3ffc, including the 12 bytes for the header), while the CX3 firmware (in both
                 * Amundsen and UVC versions) uses a smaller maximum of almost 12kB (it uses 0x2fdc,
                 * including the header).
                 */
                static const std::size_t AMUNDSEN_STRIDE_SIZE = 16 * 1024;

                /**
                 * Return codes for prepareToReceivePayload
                 */
                enum class PrepareStatus
                {
                    /** Buffer is waiting for I/O */
                    SUCCESS,
                    /**
                     * A request to prepareToReceivePayload reached a (Bridge or native library)
                     * limit on the number of pending requests. This is an expected state, the
                     * queued buffers will receive data.
                     */
                    ALL_BUFFERS_ALREADY_PREPARED,
                    /**
                     * Something went wrong.
                     */
                    FAILURE
                };

                ROYALE_API BridgeAmundsenCommon ();
                ROYALE_API ~BridgeAmundsenCommon() override;

                // From IBridgeDataReceiver
                ROYALE_API std::size_t executeUseCase (int width, int height, std::size_t preferredBufferCount) override;
                ROYALE_API void startCapture() override;
                ROYALE_API void stopCapture() override;
                ROYALE_API royale::Vector<royale::Pair<royale::String, royale::String>> getBridgeInfo() override;
                ROYALE_API void setEventListener (royale::IEventListener *listener) override;
                ROYALE_API float getPeakTransferSpeed() override;

                // From BridgeInternalBufferAlloc
                ROYALE_API bool shouldBlockForDequeue() override;

                /**
                 * The data passed to this is included in the info returned by getBridgeInfo.
                 *
                 * At the time of writing, it's implementation defined whether calling it a second
                 * time merges the two sets of data or simply overwrites all data from previous
                 * calls.
                 */
                ROYALE_API void setUvcBridgeInfo (royale::Vector<royale::Pair<royale::String, royale::String>> &&info);

                /**
                 * Sets the format that buffers are received in.  This is only expected to be called
                 * during bridge creation before image data is received (for example by the
                 * BridgeFactory), it's provided for bridges that have to be constructed and query their
                 * device before they know which data format will be used.
                 *
                 * If it should be called later in the Bridge lifecycle, then it will need thread safety
                 * to be added.
                 *
                 * BufferDataFormat::UNKNOWN is unsupported, it will cause frames to be dropped until this
                 * is called again with a supported format.
                 */
                ROYALE_API void setTransferFormat (royale::buffer::BufferDataFormat format);

                /**
                 * Sets the value that getPeakTransferSpeed will return. The peak transfer speed
                 * depends on whether the connection is USB High Speed or USB SuperSpeed, but as
                 * this isn't available to the application on all platforms, we instead ask the
                 * firmware via the UVC Extension.
                 *
                 * This does not change the USB connection itself, just the number that
                 * getPeakTransferSpeed returns.
                 */
                ROYALE_API void setUsbSpeed (royale::usb::pal::UsbSpeed speed);

            protected:
                /**
                 * Called from the acquisition thread, will be followed by either corresponding
                 * calls to receivePayload(), or a call to cancelPendingPayloads().
                 *
                 * The pointer first points to an area of AMUNDSEN_STRIDE_SIZE bytes, which the
                 * platform-specific subclass should read data in to.  The platform-specific
                 * subclass may write data in to this area at any time between this call to
                 * prepareToReceivePayload and the corresponding receivePayload returning.
                 *
                 * If the subclass successfully queues this data area for IO, it should return
                 * SUCCESS. If it reaches an internal limit on the number of data areas that can be
                 * queued, it should return ALL_BUFFERS_ALREADY_PREPARED.
                 *
                 * This may (and optimally will) be called before all of the previously-prepared
                 * buffers have been used, which does not cancel the previous call.  Separate calls to
                 * this function may use non-contiguous areas of memory, or may wrap-round on a
                 * large circular buffer; the only guarantee is that the BridgeAmundsenCommon will
                 * ensure that an already-prepared area won't be reused until after the
                 * corresponding call to receivePayload or cancelPendingPayloads.
                 *
                 * The implementation of stopCapture() will call cancelPendingPayloads().
                 */
                virtual PrepareStatus prepareToReceivePayload (uint8_t *first) = 0;

                /**
                 * Called from the acquisition thread, this must do a blocking read until the data
                 * is received. A USB payload of up to AMUNDSEN_STRIDE_SIZE bytes must be read, in
                 * to the buffer that starts at first, which has previously been passed to
                 * prepareToReceivePayloads.
                 *
                 * This does not try to interpret the data, nor to align it.  That logic is in the
                 * BridgeAmundsenCommon class, not the platform-specific subclasses.  Each call to
                 * this function is expected to receive one USB payload, which may have different
                 * sizes.  The protocol includes a header at the start of each payload, so the
                 * implementation must not read more than one payload.
                 *
                 * The value returned is the number of bytes actually received, which is expected to
                 * be slightly less than AMUNDSEN_STRIDE_SIZE (and much less for the final payload
                 * of each image).
                 *
                 * If a timeout occurs and retryOnTimeout is true, then the platform-specific code
                 * may keep waiting for data, or it may return zero.  If it returns zero, then the
                 * caller will assume that, on this platform, the native read that was started in
                 * prepareToReceivePayload has finished, and this data area will not be reused until
                 * prepareToReceivePayload is called again; therefore the next call to
                 * receivePayload must use the data area which was prepared subsequently to the
                 * current one.
                 *
                 * Timeout behavior of the existing implementations: BridgeAmundsenLibUsb will
                 * return zero, BridgeAmundsenCyApi will retry with the current buffer.
                 *
                 * \return the number of bytes received, or zero if a recoverable error occured (timeout)
                 * \throw RuntimeException if an unrecoverable error occured (USB disconnect etc)
                 */
                virtual std::size_t receivePayload (uint8_t *first, std::atomic<bool> &retryOnTimeout) = 0;

                /**
                 * Called when the acquisition thread is stopping (which will happen before reallocating buffers).
                 *
                 * Any I/O that was queued by prepareToReceivePayloads won't get a call to receivePayload.
                 */
                virtual void cancelPendingPayloads() = 0;

                /**
                 * Event forwarder.
                 */
                royale::EventForwarder m_eventForwarder;

            private:
                /**
                 * Thread that reads image data from the module.  This Bridge talks directly to the
                 * USB layer, and needs to be ready to receive data when the module sends it,
                 * otherwise the data is simply dropped.
                 *
                 * This thread passes the captured buffers to the IBufferCaptureListener's callback,
                 * which must return quickly.
                 */
                std::thread m_acquisitionThread;

                /**
                 * Main loop of m_acquisitionThread
                 */
                void acquisitionFunction();

                /**
                 * Control variable, set to false to signal the m_acquisitionThread to finish.
                 *
                 * Must only be set/unset with the m_acquisitionStartStopLock held.
                 */
                std::atomic<bool> m_runAcquisition {false};

                /**
                 * Lock for starting / stopping the acquisition thread. This is to prevent the
                 * situation that stopThread() followed by startThread() resets the m_runAcquisition
                 * variable before the original thread has finished.
                 *
                 * The m_acquisitionThread itself never locks this.
                 */
                std::mutex m_acquisitionStartStopLock;

                /**
                 * Size of the buffer (including protocol overhead) required for the acquisition
                 * thread. This is used as a circular buffer divided in to AMUNDSEN_STRIDE_SIZE
                 * strides, and the value here is stored in strides, not bytes.
                 *
                 * Must only be changed while the acquisition thread is stopped.
                 */
                std::size_t m_receiveBufferStrides;

                /**
                 * Whether the data received will be in RAW12 or RAW16 format, or auto-detect.
                 *
                 * Amundsen devices are always expected to declare their transfer format, but some
                 * UVC devices with firmware older than v0.13.1 do not.
                 *
                 * Should only be changed during bridge creation as in setTransferFormat(), or by
                 * code in the acquisition thread (when a format has been auto-detected).
                 */
                royale::buffer::BufferDataFormat m_transferFormat = royale::buffer::BufferDataFormat::UNKNOWN;

                /**
                 * Size of the data, excluding protocol overhead, that will be converted to the
                 * received ICapturedBuffer. Corresponds to BufferUtils::getExpectedRawSize().
                 *
                 * Must only be changed either while the acquisition thread is stopped, or by
                 * code running in the acquisition thread (when a format has been auto-detected).
                 */
                std::size_t m_rawDataSize;

                /**
                 * Information from setUvcBridgeInfo, used in getBridgeInfo.
                 */
                royale::Vector<royale::Pair<royale::String, royale::String>> m_uvcBridgeInfo;

                /**
                 * For calculating the return value of getPeakTransferSpeed.
                 */
                royale::usb::pal::UsbSpeed m_usbSpeed {royale::usb::pal::UsbSpeed::UNKNOWN};
            };
        }
    }
}
