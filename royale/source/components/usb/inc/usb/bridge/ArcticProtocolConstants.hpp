/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            // This namespace is the vendor extension that IFX has written for the FX3 and CX3
            namespace arctic
            {
                /**
                 * Standard UVC properties, reproduced here to be platform independent.
                 */
                enum UvcRequest
                {
                    SET_CUR = 0x01,
                    GET_CUR = 0x81,
                };

                /**
                 * Bitmask for a bit indicating that the last command failed. This data is in byte zero
                 * of the data returned by reading CONTROL_SETUP.
                 */
                const uint8_t STALL_INDICATOR = 0x01;

                /**
                 * UVC controls have semi-fixed sizes (the host system's OS can query what the size of a
                 * variable-size property is, but the UVC device must know how big a variable-size
                 * property is and respond to the query).
                 *
                 * The Cypress vendor extension framework maps the property number to the size.
                 *
                 * These are mapped to the firmware's XUcontrolId, which is the low 8 bits of a 16-bit
                 * UVC property (the wValue).
                 */
                enum VendorControlId : uint8_t
                {
                    /**
                     * Every command is performed by first setting this. Reading it returns status
                     * data, including the STALL_INDICATOR.
                     *
                     * If the data length is zero, the command is performed immediately.  Otherwise, it
                     * has to be followed by a second SET or GET (depending on the desired effect),
                     * which provides the data buffer for the command.
                     */
                    CONTROL_SETUP = 1,
                    /** Always 64 bytes of data */
                    CONTROL_64 = 2,
                    /** Always 512 bytes of data */
                    CONTROL_512 = 3,
                    /** Always 4k of data */
                    CONTROL_4096 = 4,
                    /** Up to 4k, is the exact data size of the preceding CONTROL_SETUP */
                    CONTROL_VARIABLE_SIZE = 5,
                };

                enum VendorReqType : uint8_t
                {
                    READ = 0xC1,
                    WRITE = 0x41
                };

                enum VendorRequest : uint8_t
                {
                    I2C_NO_ADDRESS = 0,
                    I2C_8_ADDRESS = 1,
                    I2C_16_ADDRESS = 2,
                    /** Same as I2C_16_ADDRESS, but uses the imager address hardcoded in the firmware */
                    I2C_IMAGER = 3,
                    /**
                     * Set the I2C bus speed, in bits per second (the firmware will not reset this,
                     * setting it and not resetting it is expected to make all communication with the
                     * imager fail).  32-bit value, with the high 16 bits in wValue and the low in
                     * wIndex.
                     *
                     * This property is write-only, there's no support to read the current speed.
                     */
                    I2C_BUS_SPEED = 0x15,

                    /**
                     * Read/write SPI data, assuming that the SPI device is a flash memory of the
                     * type supported by the firmware.
                     *
                     * Writing may be limited to page-based writes, and reading may (depending on the
                     * device) be limited to sector-based reads.  In Royale v1.7.0 the writes are
                     * page-based, but the device is assumed to support arbitrary reads.
                     *
                     * These use a 24-bit address in the wIndex and wValue, with the high 8 bits
                     * reserved for the command ID.  Those high 8 bits are ignored, in the firmware a
                     * command ID is chosen based on whether the command is read or write.
                     */
                    SPI_PAGES = 4,
                    /**
                     * For WRITE requests, this erases sectors of the SPI storage.  Reading it returns a
                     * flag that is true if an erase operation is still in progress.
                     *
                     * This assumes that the SPI device is a flash memory of the type supported by
                     * the firmware.
                     */
                    SPI_ERASE = 5,
                    /**
                     * Read or write SPI data. This is very similar to SPI_PAGES, but the high 8
                     * bits are not ignored.
                     *
                     * These use a 24-bit address in the wIndex and wValue, with the high 8 bits
                     * used for the command ID, the command ID is passed to the device. For GET
                     * requests, it is not possible to read any of the first four bytes returned by
                     * the device.
                     */
                    SPI_GENERIC = 6,

                    /**
                     * Full SPI read access using the currently active bus configuration, as chosen
                     * by SPI_SELECT_DEVICE. This command has a firmware implementation for a
                     * vendor read request and a vendor write request.
                     *
                     * This command must always be sent as a SET-then-GET pair. First a SET request
                     * must be issued. If the SET request succeeded a subsequent GET request must be
                     * issued. Between these two calls the firmware will not alter the SPI chip
                     * select line, except when a call fails.
                     *
                     * wIndex and wValue are ignored, the full buffer to send and the full buffer to
                     * receive are in the data parts of the USB transactions.
                     */
                    SPI_READ = 0x12,

                    /**
                     * Full SPI write access using the currently active bus configuration
                     *
                     * wIndex and wValue are ignored, the full buffer to send is in the data part of
                     * the request.
                     */
                    SPI_WRITE = 0x14,

                    /**
                     * Control the supported IO pins, using pin IDs which are portable at the Arctic
                     * Protocol layer.  These may be mapped to dedicated IO lines as well as GPIOs (for
                     * example, the RESET pin may control the CX3's camera-reset pin on the MIPI block).
                     *
                     * Previously called RESET_OR_STANDBY, when it only supported two GPIOs.
                     */
                    GPIO_LOGICAL = 7,
                    /**
                     * Choose which SPI SSN line will be activated for all of the other SPI_
                     * commands.  wIndex contains a logical ID in the low 8 bits, the high bits are
                     * ignored.
                     */
                    SPI_SELECT_DEVICE = 8,
                    /**
                     * Reset the device.  The type of reset is selected by the DeviceResetType
                     * which is sent as the wValue.
                     */
                    DEVICE_RESET = 0x18,
                    /**
                     * Allow a different device to act as the SPI bus master, all of its SPI IO pins
                     * are set to high impedance.  A value from the DisableAndHighZ enum is sent as
                     * the wValue.
                     */
                    PERIPHERAL_DISABLE_AND_HIGH_Z = 0x19,
                    /**
                     * Read the details of any error in the last operation.
                     *
                     * Since firmware version v0.7: Any successful operation (including the I2C ones)
                     * will clear the error status; writing to the ERROR_DETAILS itself is a no-op with
                     * that as a side effect.
                     *
                     * In v06 firmware: Check if any firmware-detected errors have occurred, write to
                     * clear the error counter.
                     */
                    ERROR_DETAILS = 0x20,
                    /** Firmware version number, and data for checking compatibility */
                    VERSION_AND_SUPPORT = 0x21,
                    /** There is no action by the firmware when it receives this */
                    INVALID_VALUE = 255
                };

                /**
                 * Offsets for interpreting the VendorRequest::VERSION_AND_SUPPORT structure.
                 */
                enum VersionAndSupport : std::size_t
                {
                    /** uint16_t version number */
                    MAJOR = 0x0,
                    /** uint16_t version number */
                    MINOR = 0x2,
                    /** uint32_t version number */
                    PATCH = 0x4,
                    /** Using the uint8_t SupportFlags enum */
                    FLAGS = 0xf,
                    /** Using the uint8_t ArcticUsbSpeed enum */
                    USB_SPEED = 0x10,
                    /** Using the uint8_t ArcticUsbTransferFormat enum */
                    TRANSFER_FORMAT = 0x11,
                };

                /**
                 * Optional features that the firmware may support.  This is currently a uint8_t,
                 * but could expand to a uint32_t using the preceding 3 bytes (which are currently
                 * reserved).
                 */
                enum SupportFlags : uint8_t
                {
                    /** Set if VendorControlId::VARIABLE_LENGTH is supported by the firmware. */
                    VARIABLE_LENGTH = 1 << 0,
                    /**
                     * Only expected with development firmware.  Could also be caused by broken
                     * hardware or by flashing the wrong firmware to a device.
                     */
                    BOOT_INCOMPLETE = 1 << 1,
                };

                /**
                 * For CX3VendorRequest::GPIO_LOGICAL, one of these values should be in wIndex, and a
                 * CX3GenericGpioState should be in wValue.
                 */
                enum LogicalGpio : uint16_t
                {
                    RESET = 0,
                    STANDBY,
                    START,
                    EN_3V3 = 0x04,
                    EN_1V5,
                    EN_VIL,
                    EN_1V8,
                    IO_0 = 0x10,
                    IO_1,
                    IO_2,
                    IO_3,
                    IO_4
                };

                /**
                 * Values for CX3VendorRequest::GPIO_LOGICAL
                 */
                enum ArcticGpioState : uint16_t
                {
                    LOW = 0,
                    HIGH = 1,
                    /** High-Z or input */
                    Z = 2,
                    /** Alias for the inverted logic of the reset and standby GPIOs */
                    RESET_ENABLE = LOW,
                    /** Alias for the inverted logic of the reset and standby GPIOs */
                    RESET_DISABLE = HIGH
                };

                /**
                 * Options for VendorRequest::DEVICE_RESET.  No firmware is likely to support all of
                 * the options, but all will implement ANY_SUPPORTED which resets the device using a
                 * firmware-chosen method.
                 *
                 * For details of the different methods, please see the firmwares' documentation.
                 */
                enum DeviceResetType : uint16_t
                {
                    ANY_SUPPORTED = 0,
                    BUS_AND_KNOWN_PERIPHERALS = 0x10,
                    API_HARD_RESET = 0x20,
                    API_WARM_RESET = 0x21,
                };

                /**
                 * Options for VendorRequest::PERIPHERAL_DISABLE_AND_HIGH_Z.
                 */
                enum DisableAndHighZ : uint16_t
                {
                    /**
                     * Turn the SPI block's IO pins to high impedence, which may allow another
                     * device to become the bus master.
                     */
                    SPI_DISABLE_AND_HIGH_Z = 2,
                    /**
                     * Re-enable the SPI block.
                     */
                    SPI_ENABLE = 3
                };

                /**
                 * Connection speed, returned in VERSION_AND_SUPPORT since firmware v0.11.3.
                 */
                enum class ArcticUsbSpeed : uint8_t
                {
                    UNKNOWN = 0,
                    FULL,
                    HIGH,
                    SUPER
                };

                /**
                 * Data format, returned in VERSION_AND_SUPPORT since firmware v0.13.1.
                 */
                enum class ArcticUsbTransferFormat : uint8_t
                {
                    /** The firmware doesn't provide this information, it must be auto-detected. */
                    UNKNOWN = 0,
                    /** The data format that's unpacked by BufferUtils::normalizeRaw12  */
                    RAW12 = 1,
                    /** The data format that's unpacked by BufferUtils::normalizeRaw16  */
                    RAW16 = 2
                };

                /**
                 * The Arctic protocol limits all writes and reads to this maximum size (in bytes).
                 */
                const std::size_t MAXIMUM_DATA_SIZE = 4096;

                /**
                 * An error enum that can be returned by the ERROR_DETAILS request.
                 */
                enum VendorError : uint32_t
                {
                    VENDOR_ERROR_BASE = ('i' << 24 | 'f' << 16 | 'x' << 8),
                    VENDOR_ERROR_NO_STATUS = VENDOR_ERROR_BASE, /**< Secondary status, when nothing has caused a secondary status to be set */
                    VENDOR_ERROR_IN_GETERROR, /**< Secondary status if the function to retrieve the secondary error status itself returns an error */
                };
            }
        }
    }
}
