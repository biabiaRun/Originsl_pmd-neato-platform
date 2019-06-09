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

#include <buffer/BridgeInternalBufferAlloc.hpp>
#include <hal/IBridgeImager.hpp>
#include <storage/IBridgeWithPagedFlash.hpp>
#include <pal/II2cBusAccess.hpp>
#include <common/EventForwarder.hpp>

#include <buffer/BufferDataFormat.hpp>
#include <buffer/OffsetBasedCapturedBuffer.hpp>

#include <atomic>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            /**
             * Requests that are sent to the chip at the other end of the USB cable start with this
             * 32-bit command code.
             *
             * The protocol requires the commands and acknowledgements to be treated as wMaxPacketSize
             * (512-byte or 1024-byte) blocks. Trying to use a smaller block for the commands will get a
             * USB ENOENT status (at the protocol level, below LibUSB).  Trying to use a smaller block
             * for reading the ack will get a USB EOVERFLOW.
             */
            enum EnclustraCommand : uint32_t
            {
                READ_FLASH              = 1,
                ERASE_FLASH             = 2,
                WRITE_FLASH             = 3,
                SET_FLASH_MODE          = 4,
                WRITE_FPGA              = 5,
                SEL_FLASH               = 6,
                WRITE_I2C               = 7,
                READ_I2C                = 8,
                INIT_SL_FIFO            = 9,
                /** This maps to a single 32-bit number in the protocol, with no extra data */
                START_IMAGER            = 10,
                /** This maps to a single 32-bit number in the protocol, with no extra data */
                STOP_IMAGER             = 11,
                /**
                 * This triggers the firmware to put the imager in to reset state, then out of reset state
                 * again.  The firmware does not support keeping the imager in reset state.
                 *
                 * This maps to a single 32-bit number in the protocol, with no extra data
                 */
                RESET_IMAGER            = 12,

                ACTIVATE_LDD_ENABLE     = 13,
                DEACTIVATE_LDD_ENABLE   = 14,
                ACTIVATE_HFM            = 15,
                DEACTIVATE_HFM          = 16,
                SET_GPIO                = 17,         /* Command: Set a GPIO pin */
                REQUEST_TEMP            = 18,         /* Command: Trigger a temperature measurement */
                FETCH_TEMP              = 19,         /* Command: Fetch the temperature value */

                IMAGER_RESTART          = 23,
            };

            /**
             * Enclustra is a type of firmware that can be used in Pico devices with Cypress USB chips.
             *
             * It can be accessed via either the Windows-based CyAPI library, or via LibUSB.  The two
             * options are handled by different subclasses of BridgeEnclustra, and the
             * bridge factory should create the appropriate subclass.
             */
            class BridgeEnclustra : public buffer::BridgeInternalBufferAlloc, public royale::pal::II2cBusAccess, public royale::hal::IBridgeImager, public royale::storage::IBridgeWithPagedFlash
            {
                typedef std::vector<uint16_t>::size_type reg16VectorSize_t;

            public:
                ROYALE_API BridgeEnclustra ();
                ROYALE_API ~BridgeEnclustra() = default;

                ROYALE_API std::size_t executeUseCase (int width, int height, std::size_t preferredBufferCount) override;

                // From IBridgeInternalBufferAlloc
                ROYALE_API bool shouldBlockForDequeue() override;
                ROYALE_API void setEventListener (royale::IEventListener *listener) override;

                // From IBridgeImager

                /**
                 * The Enclustra firmware does not support keeping the imager in reset state. Calling
                 * this with argument "true" will put the imager in reset state, and then turn it back
                 * to normal state.  With argument "false", this operation is a no-op.
                 */
                ROYALE_API void setImagerReset (bool state) override;
                ROYALE_API void readImagerRegister (uint16_t regAddr, uint16_t &value) override;
                ROYALE_API void writeImagerRegister (uint16_t regAddr, uint16_t value) override;
                ROYALE_API void readImagerBurst (uint16_t firstRegAddr, std::vector<uint16_t> &values) override;
                ROYALE_API void writeImagerBurst (uint16_t firstRegAddr, const std::vector<uint16_t> &values) override;
                ROYALE_API void sleepFor (std::chrono::microseconds sleepDuration) override;

                /**
                 * As readImagerBurst, but the registers will all be read in a single I2C operation.
                 */
                ROYALE_API void readImagerBurstAtomic (uint16_t firstRegAddr, std::vector<uint16_t> &values, reg16VectorSize_t start, reg16VectorSize_t end);

                /**
                 * As writeImagerBurst, but the registers will all be written in a single I2C operation.
                 * Depending on the imager, this may mean that all of the writes take effect at the same
                 * time, or that any error will write none of the registers.
                 *
                 * \param firstRegAddr address that the write starts from - the 'start' goes to this
                 * address, even if start is non-zero
                 * \param values vector to read data from
                 * \param start the first register will be set to values[start]
                 * \param end the last register will be set to values[end - 1]
                 */
                ROYALE_API void writeImagerBurstAtomic (uint16_t firstRegAddr, const std::vector<uint16_t> &values, reg16VectorSize_t start, reg16VectorSize_t end);

                /**
                 * How long each USB command is given, in milliseconds.  This is the time for the to USB read or
                 * write, not the total time between EnclustraCommand and the respective ACK).
                 */
                static const unsigned int ENCLUSTRA_TIMEOUT = 2000;

                /**
                 * When opening the connection (or clearing an error state), the time to wait while
                 * emptying the control input channel of previously-sent ACKs and responses.
                 *
                 * This should be short - openConnection() will block until this times out.
                 */
                static const unsigned int ENCLUSTRA_FLUSH_TIMEOUT = 20;

                /**
                 * I/O timeout for reading the image raw frames' data.
                 */
                static const unsigned int ENCLUSTRA_DATA_TIMEOUT = 500;

                /**
                 * Activates the data-receiving thread and hardware.
                 *
                 * This does not trigger the Imager to start sending data, it only triggers the Bridge
                 * to read the data.
                 */
                ROYALE_API void startCapture() override;

                /**
                 * Deactivates the data-receiving thread and hardware.
                 *
                 * The acquisition thread will be stopped before this function returns.
                 */
                ROYALE_API void stopCapture() override;

                ROYALE_API void readFlashPage (std::size_t page, std::vector<uint8_t> &recvBuffer, std::size_t noPages = 1) override;
                ROYALE_API void writeFlashPage (std::size_t page, const std::vector<uint8_t> &sndBuffer, std::size_t noPages = 1) override;
                ROYALE_API void eraseSectors (std::size_t startSector, std::size_t noSectors) override;

                // From II2cBusAccess
                ROYALE_API void readI2c (uint8_t devAddr, royale::pal::I2cAddressMode addrMode, uint16_t regAddr, std::vector<uint8_t> &buffer) override;
                ROYALE_API void writeI2c (uint8_t devAddr, royale::pal::I2cAddressMode addrMode, uint16_t regAddr, const std::vector<uint8_t> &buffer) override;
                ROYALE_API void setBusSpeed (uint32_t bps) override;
                ROYALE_API std::size_t maximumDataSize() override;

                ROYALE_API royale::Vector<royale::Pair<royale::String, royale::String>> getBridgeInfo() override;

            protected:

                /**
                 * Send a command to the Enclustra, and check that the Enclustra sends an ACK packet
                 * back. This method will handle concurrency, checking that sending commands in multiple
                 * threads does not get the ACKs mixed up.
                 *
                 * It will also handle buffer size, ensuring that the buffer is padded to the size
                 * required by the Enclustra firmware.
                 *
                 * \param command the EnclustraCommand to be performed.
                 *
                 * \param cmdBuffer complete buffer of the command to send, in little-endian byte order.
                 * Note that the first four bytes are the enumerated command, and will be checked
                 * against the first argument.
                 *
                 * \param recvBuffer if non-NULL, recvBuffer.size() bytes of data will be read in to
                 * this buffer.  The data is read before reading the ACK (the ACK is in the same data
                 * channel, so passing a recvBuffer to a command that doesn't expect the extra read will
                 * result in the ACK being read in to the recvBuffer, and either a read error or a
                 * timeout waiting for the ACK).
                 *
                 * \param sendBuffer if non-NULL and command is set to WRITE_FLASH, sendBuffer.size() bytes
                 * of data will be written. The data is written before reading the ACK.
                 */
                virtual void doCommand (EnclustraCommand command, const std::vector<uint8_t> &cmdBuffer,
                                        std::vector<uint8_t> *recvBuffer = nullptr,
                                        const std::vector<uint8_t> *sendBuffer = nullptr) = 0;

                /**
                 * For the simple commands that don't need to include additional data, sends the command
                 * and checks that an ACK is received.
                 */
                void doCommand (EnclustraCommand command);

                /**
                 * Maximum size (in bytes) of the cmdBuffer or recvBuffer arguments of doCommand.
                 * If the sizes are different (unlikely), returns the lesser of the two.
                 */
                virtual size_t getCommandMessageSize () = 0;

                /**
                 * Called from the acquisition thread. Fill this buffer with data.
                 *
                 * This does not try to interpret the data, nor to align it.  That logic is in
                 * checkAlignment, which is called from the BridgeEnclustra class, not the
                 * platform-specific subclasses.
                 *
                 * The offset is used when a previous read had the end-of-frame marker in the middle of
                 * the frame data. This method assumes that offset bytes have already been received, and
                 * only reads the last (frame size - offset) bytes to the end of the buffer.
                 *
                 * \return true if the expected amount of data was received, false if a recoverable error occured (timeout)
                 * \throw RuntimeException if an unrecoverable error occured (USB disconnect etc)
                 */
                virtual bool receiveRawFrame (royale::buffer::OffsetBasedCapturedBuffer &buffer, std::size_t offset) = 0;

                std::atomic<bool> m_isConnected {false};

                /**
                 * Size of the data as it as it transfers over the USB transport.  This is implicitly
                 * hardcoded in to BridgeEnclustra.cpp's buffer handling, but the subclasses only depend
                 * on this constant and the sizes of buffers that BridgeEnclustra.cpp tells them to use.
                 */
                static const auto ENCLUSTRA_TRANSFER_FORMAT = royale::buffer::BufferDataFormat::RAW16;

                /**
                * Event forwarder.
                */
                royale::EventForwarder m_eventForwarder;

            private:
                /**
                 * I2C address of the MAIN_IMAGER.
                 */
                uint8_t m_i2cAddressImager;

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
                std::atomic<bool> m_runAcquisition;

                /**
                 * Lock for starting / stopping the acquisition thread. This is to the situation that
                 * stopThread() followed by startThread() resets the m_runAcquisition variable before
                 * the original thread has finished.
                 *
                 * The m_acquisitionThread itself never locks this.
                 */
                std::mutex m_acquisitionStartStopLock;

                /**
                 * The pixel data is padded to a multiple of this constant.  If the pixel data is an
                 * exact multiple of this, a full ENCLUSTRA_FRAME_PADDING_SIZE bytes of padding are
                 * added.
                 */
                static const std::size_t ENCLUSTRA_FRAME_PADDING_SIZE = 1024;

                /**
                 * Bulk reads from the data channel (of the pixel data) fail unless the requested size
                 * is an exact multiple of this constant.  ENCLUSTRA_FRAME_PADDING_SIZE is a multiple of
                 * this.
                 *
                 * Technically this depends on the USB speed, on a USB Super Speed connection it's 1024.
                 * However, the only effect of using the higher value would be a tiny optimisation when
                 * trying to find the alignment for the frames in the data (and normally it is aligned).
                 */
                static const std::size_t ENCLUSTRA_MIN_DATA_PACKET_SIZE = 512;

                /**
                 * Checks for end-of-frame markers in the data.  If they're not found, calculates where the start of
                 * frame should be.
                 *
                 * The frame is passed to this function before normalisation.
                 *
                 * If a frame is not aligned, we do not have the data to align it.  Instead the offset is calculated
                 * so that the rest of the next frame can be read an discarded, and then reads after that should
                 * receive aligned frames.
                 *
                 * \return zero if the frame is aligned, a positive offset (in bytes) if the frame isn't aligned
                 *         or -1 if a frame can't be found in the data.
                 */
                std::ptrdiff_t checkAlignment (royale::buffer::OffsetBasedCapturedBuffer &buffer) const;
            };
        }
    }
}
