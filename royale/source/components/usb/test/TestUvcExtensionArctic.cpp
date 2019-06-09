/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/bridge/IUvcExtensionAccess.hpp>
#include <usb/bridge/BridgeImagerArctic.hpp>
#include <usb/pal/I2cBusAccessArctic.hpp>
#include <usb/pal/SpiBusAccessArctic.hpp>
#include <usb/bridge/StorageSpiFlashArctic.hpp>
#include <pal/II2cDeviceAccess.hpp>
#include <pal/Access2I2cDeviceAdapter.hpp>
#include <common/SensorRoutingConfigI2c.hpp>
#include <common/SensorRoutingConfigSpi.hpp>

#include <EndianConversion.hpp>
#include <MakeUnique.hpp>
#include <NarrowCast.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/PossiblyUsbStallError.hpp>

#include <gtest/gtest.h>

#include <RoyaleLogger.hpp>

#include <algorithm>
#include <memory>
#include <cstddef>
#include <cstdint>
#include <limits>

// This file generally hardcodes the numbers instead of using the ArcticProtocolConstants.hpp,
// because hardcoding the numbers in the test will catch incompatible changes to the header.
// The header is included, but there's no "using namespace royale::usb::bridge::arctic;"
#include <usb/bridge/ArcticProtocolConstants.hpp>

using namespace royale::usb::bridge;
using namespace royale::usb::config;
using namespace royale::pal;
using namespace royale::common;
using namespace royale::config;

namespace
{
    /**
     * First register of a block of registers for the read and write tests.
     *
     * Arbitrary value, but matches a real imager.
     */
    const uint16_t READ_BASE_REGISTER_ADDRESS = 0xA800;
    /** Number of registers for the read test (more than MAXIMUM_DATA_SIZE / sizeof (uint16_t)) */
    const uint16_t COUNT_REGISTERS_TO_READ = 2100;

    /** The status code meaning "no error" */
    const uint32_t CY_U3P_SUCCESS = 0;
    /** Unknown / unsupported request */
    const uint32_t CY_U3P_ERROR_NOT_SUPPORTED = 0x46;
    /** This one is returned when an I2C device doesn't respond */
    const uint32_t CY_U3P_ERROR_FAILURE = 0x4a;
    /** Secondary error status meaning that the command never gives a secondary error status */
    const uint32_t VENDOR_ERROR_NO_ERROR = 0x69667600;

    /**
     * This is a stub for the CX3 firmware.
     */
    class MockArcticCX3 : public IUvcExtensionAccess
    {
    public:
        /** A test address of a device which is assumed to exist */
        static const uint8_t I2C_ADDRESS_TEMPERATURE = 0x48;
        /**
         * A non-existent device, for testing that reads and writes fail.
         */
        static const uint8_t I2C_ADDRESS_NONEXISTENT = 0x7f;

        MockArcticCX3 (bool onlyUseFixedSize) :
            m_onlyUseFixedSize {onlyUseFixedSize}
        {
        }

        ~MockArcticCX3() = default;

        UvcExtensionType getVendorExtensionType() const override
        {
            return UvcExtensionType::Arctic;
        }

        // From IUvcExtensionAccess
        std::unique_lock<std::recursive_mutex> lockVendorExtension() override
        {
            return std::unique_lock<std::recursive_mutex> {m_mutex};
        }

        bool onlySupportsFixedSize() const override
        {
            return m_onlyUseFixedSize;
        }

        void vendorExtensionGet (uint16_t dataId, std::vector<uint8_t> &data) override
        {
            auto lock = lockVendorExtension();

            // Check if this is a read of the error status, in the setup property
            if (arctic::VendorControlId::CONTROL_SETUP == dataId)
            {
                ASSERT_TRUE (m_setup.empty());
                ASSERT_EQ (8u, data.size());

                memset (data.data(), 0, data.size());
                if (m_primaryError != CY_U3P_SUCCESS)
                {
                    data [0] = 1;
                }
                return;
            }

            ASSERT_FALSE (m_setup.empty());
            auto setup = m_setup;
            m_setup.clear();
            ASSERT_EQ (8u, setup.size());
            ASSERT_EQ (0xC1, setup[0]); // This is a get with a READ request
            const auto request = setup[1];
            const auto dataSize = bufferToHostBe16 (&setup[6]);
            ASSERT_LE (dataSize, data.size());
            if (arctic::VendorControlId::CONTROL_VARIABLE_SIZE == dataId)
            {
                ASSERT_EQ (dataSize, data.size());
                if (m_onlyUseFixedSize)
                {
                    throw PossiblyUsbStallError ();
                }
            }

            memset (data.data(), 0, data.size());

            // Unless an error occurs, the error status will be cleared after the request
            uint32_t primaryError = CY_U3P_SUCCESS;
            uint32_t secondaryError = VENDOR_ERROR_NO_ERROR;

            switch (request)
            {
                case 0x00: // i2c read, no register address
                    {
                        auto i2cAddress = bufferToHostBe16 (&setup[2]);
                        if (i2cAddress == I2C_ADDRESS_TEMPERATURE)
                        {
                            // success case, leave the memset value in data
                        }
                        else
                        {
                            // assume a non-existent device, no ACK to the first byte
                            primaryError = CY_U3P_ERROR_FAILURE;
                            secondaryError = 0;
                        }
                        break;
                    }
                case 0x01: // i2c read, 8-bit register address
                    {
                        // assume ok, data has already been memset to zero
                        break;
                    }
                case 0x02: // i2c read, 16-bit register address
                case 0x03: // i2c read, imager
                    {
                        // assume 16-bit registers, and simulate that each register's value is the same as its address
                        ASSERT_EQ (0, dataSize % 2);
                        data.clear();
                        auto startReg = bufferToHostBe16 (&setup[4]);

                        ASSERT_LT (startReg + (dataSize / 2), std::numeric_limits<uint16_t>::max());

                        if (m_mockImager)
                        {
                            ASSERT_LT (static_cast<std::size_t> (startReg + (dataSize / 2)), m_mockImager->m_cells.size());
                            for (int i = 0; i < dataSize / 2; i++)
                            {
                                pushBackBe16 (data, m_mockImager->m_cells [startReg + i]);
                            }
                        }
                        else
                        {
                            // simulate that each register's value is the same as its address
                            for (int i = 0; i < dataSize / 2; i++)
                            {
                                pushBackBe16 (data, narrow_cast<uint16_t> (startReg + i));
                            }
                        }
                        break;
                    }
                case 0x04: // SPI read/write
                    {
                        auto address = bufferToHostBe32 (&setup[2]);
                        ASSERT_EQ (m_spiFlash.count (m_spiSelection), 1u);
                        auto &flash = m_spiFlash[m_spiSelection];
                        ASSERT_LE (address + dataSize, flash->m_cells.size());
                        std::memcpy (&data[0], &flash->m_cells[address], dataSize);
                        break;
                    }
                case 0x05: // SPI erase status
                    {
                        // return false, no erase in progress because it's instantaneous this mock
                        ASSERT_EQ (dataSize, 1u);
                        data[0] = 0;
                        break;
                    }

                case 0x20: // read the error status
                    {
                        ASSERT_TRUE (dataSize == 8u || dataSize == 12u);
                        data.clear();
                        uint32_t flags = 0;
                        if (isErrorFlagSet())
                        {
                            flags |= 0x01;
                        }
                        pushBack32 (data, flags);
                        pushBack32 (data, m_primaryError);
                        if (dataSize == 12u)
                        {
                            pushBack32 (data, m_secondaryError);
                        }
                        break;
                    }
                case 0x21: // read the version number
                    {
                        // The firmware supports >= 64 bytes, the test is stricter
                        ASSERT_EQ (64u, dataSize);
                        memset (data.data(), 0, dataSize);
                        data[0x03] = 14; // version 0.14
                        data[0x0f] = 1; // variable-width data supported
                        break;
                    }
                default:
                    {
                        primaryError = CY_U3P_ERROR_NOT_SUPPORTED;
                        break;
                    }
            }

            if (primaryError == CY_U3P_SUCCESS)
            {
                clearError();
            }
            else
            {
                simulateError (true, primaryError, secondaryError);
            }
        }

        void vendorExtensionSet (uint16_t id, const std::vector<uint8_t> &setupOrData) override
        {
            auto lock = lockVendorExtension();
            std::vector<uint8_t> data;

            if (1u == id)
            {
                // this is the setup packet
                ASSERT_TRUE (m_setup.empty());
                ASSERT_EQ (8u, setupOrData.size());
                m_setup = setupOrData;
                const auto dataSize = bufferToHostBe16 (&m_setup[6]);
                if (dataSize)
                {
                    // wait for the data packet
                    return;
                }
                // this is a request without a data phase, continue with data.empty()==true
            }
            else
            {
                // this is the data packet
                data = setupOrData;
            }

            ASSERT_FALSE (m_setup.empty());
            auto setup = m_setup;
            m_setup.clear();
            ASSERT_EQ (8u, setup.size());
            ASSERT_EQ (0x41, setup[0]); // This is a set with a WRITE request
            const auto request = setup[1];
            const auto dataSize = bufferToHostBe16 (&setup[6]);
            ASSERT_LE (dataSize, data.size());
            if (arctic::VendorControlId::CONTROL_VARIABLE_SIZE == id)
            {
                ASSERT_EQ (dataSize, data.size());
                if (m_onlyUseFixedSize)
                {
                    throw PossiblyUsbStallError ();
                }
            }


            // Unless an error occurs, the error status will be cleared after the request
            uint32_t primaryError = CY_U3P_SUCCESS;
            uint32_t secondaryError = VENDOR_ERROR_NO_ERROR;
            switch (request)
            {
                case 0x03: // i2c write, imager
                    {
                        // assume 16-bit registers, writing to the imager
                        ASSERT_NE (m_mockImager, nullptr);
                        ASSERT_EQ (0, dataSize % 2);

                        auto startReg = bufferToHostBe16 (&setup[4]);
                        ASSERT_LT (static_cast<std::size_t> (startReg + (dataSize / 2)), m_mockImager->m_cells.size());

                        for (int i = 0; i < dataSize / 2; i++)
                        {
                            m_mockImager->m_cells [startReg + i] = bufferToHostBe16 (&data[i * 2]);
                        }
                        break;
                    }
                case 0x04: // SPI write
                    {
                        auto address = bufferToHostBe32 (&setup[2]);
                        ASSERT_EQ (m_spiFlash.count (m_spiSelection), 1u);
                        auto &flash = m_spiFlash[m_spiSelection];
                        ASSERT_LE (address + dataSize, flash->m_cells.size());
                        for (std::size_t i = 0; i < dataSize ; i++)
                        {
                            // Check that the data has already been erased
                            ASSERT_EQ (flash->m_cells[address + i], 0xFF);
                        }
                        std::memcpy (&flash->m_cells[address], &data[0], dataSize);
                        break;
                    }
                case 0x05: // SPI erase
                    {
                        ASSERT_EQ (dataSize, 0u);
                        auto address = bufferToHostBe32 (&setup[2]);
                        ASSERT_EQ (m_spiFlash.count (m_spiSelection), 1u);
                        auto &flash = m_spiFlash[m_spiSelection];
                        const auto sectorSize = flash->m_config.sectorSize;
                        ASSERT_NE (sectorSize, 0u);
                        ASSERT_EQ (address % sectorSize, 0u);
                        ASSERT_LE (address + sectorSize, flash->m_cells.size());
                        std::memset (&flash->m_cells[address], 0xFF, sectorSize);
                        break;
                    }
                case 0x08: // SPI select device
                    {
                        auto spiSelection = bufferToHostBe16 (&setup[2]);
                        ASSERT_LE (spiSelection, std::numeric_limits<uint8_t>::max());
                        m_spiSelection = static_cast<uint8_t> (spiSelection);
                        break;
                    }
                case 0x20: // obsolete function to explicitly clear the error status
                    {
                        ASSERT_EQ (0u, dataSize);
                        // the clear-error function happens at the end of this function
                        break;
                    }
                default:
                    {
                        simulateError (false, CY_U3P_ERROR_NOT_SUPPORTED);
                        break;
                    }
            }

            if (primaryError == CY_U3P_SUCCESS)
            {
                clearError();
            }
            else
            {
                simulateError (true, primaryError, secondaryError);
            }
        }

        /**
         * The firmware running on the simulated CX3 has detected an error.
         *
         * \param stallable true if the real hardware would send a USB STALL (== is this a read)
         * \param primary Main error status (must not be CY_U3P_SUCCESS)
         * \param secondary command-specific detailed error
         *
         * @return the monotonic error count
         */
        uint32_t simulateError (bool stallable, uint32_t primary, uint32_t secondary = VENDOR_ERROR_NO_ERROR)
        {
            if (primary == CY_U3P_SUCCESS)
            {
                throw LogicError ("Simulating error 'success'");
            }

            auto lock = lockVendorExtension();
            m_monotonicErrorCount++;
            m_primaryError = primary;
            m_secondaryError = secondary;

            if (stallable)
            {
                throw PossiblyUsbStallError ();
            }

            return m_monotonicErrorCount;
        }

        /**
         * Resets the error status, which is the same as writing to the ERROR_COUNTER.
         */
        void clearError()
        {
            auto lock = lockVendorExtension();
            m_primaryError = CY_U3P_SUCCESS;
        }

        /**
         * Returns the ERROR_COUNTER's counter.
         *
         * The firmware's v0.6 had this, in v0.8 it's replaced by the error status enum.
         */
        bool isErrorFlagSet()
        {
            return m_primaryError != CY_U3P_SUCCESS;
        }

        /**
         * A counter which is incremented when an error is simulated, and is never reset
         * to zero.  The hardware doesn't have this, it's just for the unit tests.
         */
        uint32_t getMonotonicErrorCount()
        {
            return m_monotonicErrorCount;
        }

        /**
         * Create a simulated SPI flash memory.
         */
        void addSpiFlash (uint8_t id, const FlashMemoryConfig &config)
        {
            ASSERT_EQ (m_spiFlash.count (id), 0u);
            auto flash = makeUnique<MockSpiFlash> (config);
            m_spiFlash.emplace (id, std::move (flash));
        }

        /**
         * Create a simulated I2C device.  It will just store data and allow it to be read, there
         * will be no logic simulating the imager actually running.
         */
        void addI2cImager ()
        {
            ASSERT_FALSE (m_mockImager);
            m_mockImager = makeUnique<MockImager> ();
        }

    private:
        std::recursive_mutex m_mutex;
        uint32_t m_monotonicErrorCount = 0;
        /** The first simulated error after each clearError */
        uint32_t m_primaryError = 0;
        /** The secondary status for the same simulated error */
        uint32_t m_secondaryError = VENDOR_ERROR_NO_ERROR;

        /** The setup part of a SetGet or SetSet pair.  Empty unless we're in the middle of one of those pairs. */
        std::vector<uint8_t> m_setup;

        /** The data in an SPI chip */
        struct MockSpiFlash
        {
            FlashMemoryConfig m_config;
            std::vector<uint8_t> m_cells;

            explicit MockSpiFlash (const FlashMemoryConfig &config) :
                m_config {config},
                m_cells (config.imageSize)
            {
            }
        };
        /** Set of all simulated SPI flash chips */
        std::map<uint8_t, std::unique_ptr<MockSpiFlash>> m_spiFlash;

        /** The data in a device that provides 16-bit registers with 16-bit addresses */
        struct MockImager
        {
            std::vector<uint16_t> m_cells;

            MockImager () :
                m_cells (std::numeric_limits<uint16_t>::max() + 1)
            {
            }
        };
        /** The simulated imager, if addI2cImager has been called. */
        std::unique_ptr<MockImager> m_mockImager;

        /** Which SPI device is selected */
        uint8_t m_spiSelection = 1;

        /**
         * If true, using the CONTROL_VARIABLE_SIZE will fail. This isn't fully integrated in to
         * the simulation, as it should be simulated at the bridge layer instead of the device
         * layer.
         *
         * This does not affect SupportFlash::VARIABLE_LENGTH, because the firmware may support it
         * even if the bridge doesn't.
         */
        bool m_onlyUseFixedSize;
    };
}

template <bool onlyUseFixedSize>
class TestUvcExtensionArcticBase : public ::testing::Test
{
protected:
    TestUvcExtensionArcticBase()
        : m_bridgeImager (nullptr)
    {

    }

    virtual ~TestUvcExtensionArcticBase() override
    {

    }

    virtual void SetUp()
    {
        // These point to the same class at the moment, and may separate later.
        // They're already separated so that the tests can be written to match the
        // functionality that they're accessing.
        m_stubHardware = std::make_shared<MockArcticCX3> (onlyUseFixedSize);
        m_bridgeLowLevel = m_stubHardware;

        auto uvcExtensionArctic = std::make_shared<UvcExtensionArctic> (m_bridgeLowLevel);
        m_bridgeImager = std::make_shared<BridgeImagerArctic> (uvcExtensionArctic);
        m_i2cBusAccess = std::make_shared<royale::usb::pal::arctic::I2cBusAccessArctic> (uvcExtensionArctic);
        m_spiBusAccess = std::make_shared<royale::usb::pal::arctic::SpiBusAccessArctic> (uvcExtensionArctic);
    }

    virtual void TearDown()
    {
        m_i2cBusAccess.reset();
        m_bridgeImager.reset();
        m_bridgeLowLevel.reset();
    }

    /**
     * The simulated firmware, giving the test access to the "hardware state".
     */
    std::shared_ptr<MockArcticCX3> m_stubHardware;
    std::shared_ptr<IUvcExtensionAccess> m_bridgeLowLevel;
    std::shared_ptr<royale::hal::IBridgeImager> m_bridgeImager;
    std::shared_ptr<royale::pal::II2cBusAccess> m_i2cBusAccess;
    std::shared_ptr<royale::usb::pal::arctic::SpiBusAccessArctic> m_spiBusAccess;
};

using TestUvcExtensionArcticVariable = TestUvcExtensionArcticBase<false>;
using TestUvcExtensionArcticFixed = TestUvcExtensionArcticBase<true>;

/**
 * Reads an imager I2C register, checks that the error flag is not set.
 */
TEST_F (TestUvcExtensionArcticVariable, TestReadImagerRegister)
{
    uint16_t regValue;
    ASSERT_NO_THROW (m_bridgeImager->readImagerRegister (0u, regValue));

    ASSERT_EQ (0u, m_stubHardware->getMonotonicErrorCount());
    ASSERT_FALSE (m_stubHardware->isErrorFlagSet());
}

TEST_F (TestUvcExtensionArcticFixed, TestReadImagerRegister)
{
    uint16_t regValue;
    ASSERT_NO_THROW (m_bridgeImager->readImagerRegister (0u, regValue));

    ASSERT_EQ (0u, m_stubHardware->getMonotonicErrorCount());
    ASSERT_FALSE (m_stubHardware->isErrorFlagSet());
}

/**
 * Writes an imager I2C register, checks that the same value can be read back,
 * and that the error flag is not set.
 */
TEST_F (TestUvcExtensionArcticVariable, TestWriteImagerRegister)
{
    m_stubHardware->addI2cImager();

    const uint16_t regValue = 42;
    uint16_t readback;
    ASSERT_NO_THROW (m_bridgeImager->writeImagerRegister (0u, regValue));
    ASSERT_NO_THROW (m_bridgeImager->readImagerRegister (0u, readback));
    ASSERT_EQ (regValue, readback);

    ASSERT_EQ (0u, m_stubHardware->getMonotonicErrorCount());
    ASSERT_FALSE (m_stubHardware->isErrorFlagSet());
}

TEST_F (TestUvcExtensionArcticFixed, TestWriteImagerRegister)
{
    m_stubHardware->addI2cImager();

    const uint16_t regValue = 42;
    uint16_t readback;
    ASSERT_NO_THROW (m_bridgeImager->writeImagerRegister (0u, regValue));
    ASSERT_NO_THROW (m_bridgeImager->readImagerRegister (0u, readback));
    ASSERT_EQ (regValue, readback);

    ASSERT_EQ (0u, m_stubHardware->getMonotonicErrorCount());
    ASSERT_FALSE (m_stubHardware->isErrorFlagSet());
}

/**
 * Reads a non-existent I2C address, checks that the error flag is set and cleared.
 */
TEST_F (TestUvcExtensionArcticVariable, TestReadNonexistentI2c)
{
    std::vector<uint8_t> data (2);
    ASSERT_ANY_THROW (m_i2cBusAccess->readI2c (MockArcticCX3::I2C_ADDRESS_NONEXISTENT,
                      I2cAddressMode::I2C_NO_ADDRESS, 0, data));

    // An error should have been triggered, but the reset count should have been cleared
    ASSERT_EQ (1u, m_stubHardware->getMonotonicErrorCount());
    ASSERT_FALSE (m_stubHardware->isErrorFlagSet());
}

TEST_F (TestUvcExtensionArcticFixed, TestReadNonexistentI2c)
{
    std::vector<uint8_t> data (2);
    ASSERT_ANY_THROW (m_i2cBusAccess->readI2c (MockArcticCX3::I2C_ADDRESS_NONEXISTENT,
                      I2cAddressMode::I2C_NO_ADDRESS, 0, data));

    // An error should have been triggered, but the reset count should have been cleared
    ASSERT_EQ (1u, m_stubHardware->getMonotonicErrorCount());
    ASSERT_FALSE (m_stubHardware->isErrorFlagSet());
}

/**
 * Reads a temperature sensor, checks that the access works.
 *
 * This also tests Access2I2cDeviceAdapter.  It could use I2cBusAccessArctic directly, but
 * testing both layers together is a simple way to test Access2I2cDeviceAdapter.
 */
TEST_F (TestUvcExtensionArcticVariable, TestReadTemperatureI2c)
{
    std::unique_ptr<II2cDeviceAccess> device;
    auto addr = MockArcticCX3::I2C_ADDRESS_TEMPERATURE;
    std::vector<uint8_t> buf (2);
    ASSERT_NO_THROW (device = makeUnique<royale::pal::Access2I2cDeviceAdapter> (m_i2cBusAccess, addr));
    ASSERT_NO_THROW (device->readI2cNoAddress (buf));
    // the read should return a buffer of the right size, memset to zero
    ASSERT_EQ (buf[0], 0u);
    ASSERT_EQ (buf[1], 0u);
    ASSERT_EQ (0u, m_stubHardware->getMonotonicErrorCount());
}

TEST_F (TestUvcExtensionArcticFixed, TestReadTemperatureI2c)
{
    std::unique_ptr<II2cDeviceAccess> device;
    auto addr = MockArcticCX3::I2C_ADDRESS_TEMPERATURE;
    std::vector<uint8_t> buf (2);
    ASSERT_NO_THROW (device = makeUnique<royale::pal::Access2I2cDeviceAdapter> (m_i2cBusAccess, addr));
    ASSERT_NO_THROW (device->readI2cNoAddress (buf));
    // the read should return a buffer of the right size, memset to zero
    ASSERT_EQ (buf[0], 0u);
    ASSERT_EQ (buf[1], 0u);
    ASSERT_EQ (0u, m_stubHardware->getMonotonicErrorCount());
}

/**
 * Tests that reading registers in a burst gives the same results as reading them individually.
 */
TEST_F (TestUvcExtensionArcticVariable, BurstRegisterRead)
{
    std::vector<uint16_t> values;
    values.resize (COUNT_REGISTERS_TO_READ);

    ASSERT_NO_THROW (m_bridgeImager->readImagerBurst (READ_BASE_REGISTER_ADDRESS, values));

    // Some may be zero, but not all.
    ASSERT_NE (std::count (values.begin(), values.end(), 0), COUNT_REGISTERS_TO_READ);

    for (uint16_t i = 0; i < COUNT_REGISTERS_TO_READ; i++)
    {
        uint16_t value;
        uint16_t address = static_cast<uint16_t> (READ_BASE_REGISTER_ADDRESS + i);
        ASSERT_NO_THROW (m_bridgeImager->readImagerRegister (address, value));
        ASSERT_EQ (values[i], value);
    }
}

TEST_F (TestUvcExtensionArcticFixed, BurstRegisterRead)
{
    std::vector<uint16_t> values;
    values.resize (COUNT_REGISTERS_TO_READ);

    ASSERT_NO_THROW (m_bridgeImager->readImagerBurst (READ_BASE_REGISTER_ADDRESS, values));

    // Some may be zero, but not all.
    ASSERT_NE (std::count (values.begin(), values.end(), 0), COUNT_REGISTERS_TO_READ);

    for (uint16_t i = 0; i < COUNT_REGISTERS_TO_READ; i++)
    {
        uint16_t value;
        uint16_t address = static_cast<uint16_t> (READ_BASE_REGISTER_ADDRESS + i);
        ASSERT_NO_THROW (m_bridgeImager->readImagerRegister (address, value));
        ASSERT_EQ (values[i], value);
    }
}

/**
 * Tests writing registers in a burst and then reading them back.
 */
TEST_F (TestUvcExtensionArcticVariable, BurstRegisterWrite)
{
    m_stubHardware->addI2cImager();

    std::vector<uint16_t> values;
    values.resize (COUNT_REGISTERS_TO_READ);

    for (uint16_t i = 0; i < COUNT_REGISTERS_TO_READ; i++)
    {
        values[i] = static_cast<uint16_t> (i);
    }

    ASSERT_NO_THROW (m_bridgeImager->writeImagerBurst (READ_BASE_REGISTER_ADDRESS, values));

    std::vector<uint16_t> readback;
    readback.resize (COUNT_REGISTERS_TO_READ);
    ASSERT_NO_THROW (m_bridgeImager->readImagerBurst (READ_BASE_REGISTER_ADDRESS, readback));

    for (uint16_t i = 0; i < COUNT_REGISTERS_TO_READ; i++)
    {
        ASSERT_EQ (values[i], readback[i]);
    }
}

TEST_F (TestUvcExtensionArcticFixed, BurstRegisterWrite)
{
    m_stubHardware->addI2cImager();

    std::vector<uint16_t> values;
    values.resize (COUNT_REGISTERS_TO_READ);

    for (uint16_t i = 0; i < COUNT_REGISTERS_TO_READ; i++)
    {
        values[i] = static_cast<uint16_t> (i);
    }

    ASSERT_NO_THROW (m_bridgeImager->writeImagerBurst (READ_BASE_REGISTER_ADDRESS, values));

    std::vector<uint16_t> readback;
    readback.resize (COUNT_REGISTERS_TO_READ);
    ASSERT_NO_THROW (m_bridgeImager->readImagerBurst (READ_BASE_REGISTER_ADDRESS, readback));

    for (uint16_t i = 0; i < COUNT_REGISTERS_TO_READ; i++)
    {
        ASSERT_EQ (values[i], readback[i]);
    }
}

TEST_F (TestUvcExtensionArcticVariable, TestErrorsCheckedAndCleared)
{
    ASSERT_EQ (0u, m_stubHardware->getMonotonicErrorCount());
    ASSERT_EQ (1u, m_stubHardware->simulateError (false, 100u));
    ASSERT_EQ (1u, m_stubHardware->getMonotonicErrorCount());
    ASSERT_TRUE (m_stubHardware->isErrorFlagSet());

    // This read should succeed, and any suceeding operation should clear the error flag.
    //
    // Note: on v0.6 or before, it should catch the error simulated above, and the UvcExtensionArctic
    // should clear the error.  To simulate those versions, the readImagerRegister call should throw.
    uint16_t regValue;
    ASSERT_NO_THROW (m_bridgeImager->readImagerRegister (0u, regValue));

    ASSERT_EQ (1u, m_stubHardware->getMonotonicErrorCount());
    ASSERT_FALSE (m_stubHardware->isErrorFlagSet());
}

TEST_F (TestUvcExtensionArcticFixed, TestErrorsCheckedAndCleared)
{
    ASSERT_EQ (0u, m_stubHardware->getMonotonicErrorCount());
    ASSERT_EQ (1u, m_stubHardware->simulateError (false, 100u));
    ASSERT_EQ (1u, m_stubHardware->getMonotonicErrorCount());
    ASSERT_TRUE (m_stubHardware->isErrorFlagSet());

    // This read should succeed, and any suceeding operation should clear the error flag.
    //
    // Note: on v0.6 or before, it should catch the error simulated above, and the UvcExtensionArctic
    // should clear the error.  To simulate those versions, the readImagerRegister call should throw.
    uint16_t regValue;
    ASSERT_NO_THROW (m_bridgeImager->readImagerRegister (0u, regValue));

    ASSERT_EQ (1u, m_stubHardware->getMonotonicErrorCount());
    ASSERT_FALSE (m_stubHardware->isErrorFlagSet());
}

/**
 * Simulates having the calibration in an SPI chip, using the GPIO select line.
 *
 * EXAMPLE CONFIG: A device with two SPI-based memories, the calibration is in one and the other one
 * is the boot memory for the CX3 firmware.
 *
 * The reference hardware is 512kB with 64kB sectors, but smaller sizes are used for speed in this
 * test.  The simulater chip with the calibration is a 49kB memory, and data can only be erased in
 * 7kB blocks (these obviously-unusual sizes are used to help catch cut-and-paste errors).
 */
TEST_F (TestUvcExtensionArcticVariable, TestSpiSecondaryFlash)
{
    auto routing = SensorRoutingConfigSpi {1};
    auto config = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM}
                  .setImageSize (49 * 1024)
                  .setPageSize (256)
                  .setSectorSize (7 * 1024);

    m_stubHardware->addSpiFlash (routing.getAddress(), config);
    auto storage = std::make_shared<StorageSpiFlashArctic> (config, m_spiBusAccess, routing.getAddress());

    // The size of the test pattern is arbitrary, and bigger than a sector
    auto testPattern = std::vector<uint8_t> (10000);
    for (std::size_t i = 0; i < testPattern.size(); i++)
    {
        testPattern[i] = static_cast<uint8_t> (i);
    }
    storage->writeStorage (testPattern);

    auto readback = std::vector<uint8_t> (testPattern.size(), 0);
    storage->readStorage (0, readback);
    for (std::size_t i = 0; i < readback.size(); i++)
    {
        ASSERT_EQ (readback[i], static_cast<uint8_t> (i));
    }
}

TEST_F (TestUvcExtensionArcticFixed, TestSpiSecondaryFlash)
{
    auto routing = SensorRoutingConfigSpi {1};
    auto config = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM}
                  .setImageSize (49 * 1024)
                  .setPageSize (256)
                  .setSectorSize (7 * 1024);

    m_stubHardware->addSpiFlash (routing.getAddress(), config);
    auto storage = std::make_shared<StorageSpiFlashArctic> (config, m_spiBusAccess, routing.getAddress());

    // The size of the test pattern is arbitrary, and bigger than a sector
    auto testPattern = std::vector<uint8_t> (10000);
    for (std::size_t i = 0; i < testPattern.size(); i++)
    {
        testPattern[i] = static_cast<uint8_t> (i);
    }
    storage->writeStorage (testPattern);

    auto readback = std::vector<uint8_t> (testPattern.size(), 0);
    storage->readStorage (0, readback);
    for (std::size_t i = 0; i < readback.size(); i++)
    {
        ASSERT_EQ (readback[i], static_cast<uint8_t> (i));
    }
}

/**
 * Simulates configuring the calibration to be in the SPI chip that's selected with the default SPI
 * select line, and accessOffset == zero.  On a real device with the reference device that's
 * configured to boot from SPI, this would overwrite the firmware area.
 *
 * A device with one SPI-based memory, which is the boot memory for the CX3 firmware, and also
 * contains the calibration data.
 *
 * Unusual memory sizes are used in this config to help prevent cut-and-paste.
 */
TEST_F (TestUvcExtensionArcticVariable, TestSpiPrimaryFlashOverwritingFirmware)
{
    // Warning: do not use a config like this (primary flash with no accessOffset),
    // on a real device, as this configuration will wipe the firmware.
    auto routing = SensorRoutingConfigSpi {0};
    auto config = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM}
                  .setImageSize (49 * 1024)
                  .setPageSize (256)
                  .setSectorSize (7 * 1024);

    m_stubHardware->addSpiFlash (routing.getAddress(), config);
    auto storage = std::make_shared<StorageSpiFlashArctic> (config, m_spiBusAccess, routing.getAddress());

    // The size of the test pattern is arbitrary, and bigger than a sector
    auto testPattern = std::vector<uint8_t> (10000);
    for (std::size_t i = 0; i < testPattern.size(); i++)
    {
        testPattern[i] = static_cast<uint8_t> (i);
    }
    storage->writeStorage (testPattern);

    auto readback = std::vector<uint8_t> (testPattern.size(), 0);
    storage->readStorage (0, readback);
    for (std::size_t i = 0; i < readback.size(); i++)
    {
        ASSERT_EQ (readback[i], static_cast<uint8_t> (i));
    }
}

TEST_F (TestUvcExtensionArcticFixed, TestSpiPrimaryFlashOverwritingFirmware)
{
    // Warning: do not use a config like this (primary flash with no accessOffset),
    // on a real device, as this configuration will wipe the firmware.
    auto routing = SensorRoutingConfigSpi {0};
    auto config = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM}
                  .setImageSize (49 * 1024)
                  .setPageSize (256)
                  .setSectorSize (7 * 1024);

    m_stubHardware->addSpiFlash (routing.getAddress(), config);
    auto storage = std::make_shared<StorageSpiFlashArctic> (config, m_spiBusAccess, routing.getAddress());

    // The size of the test pattern is arbitrary, and bigger than a sector
    auto testPattern = std::vector<uint8_t> (10000);
    for (std::size_t i = 0; i < testPattern.size(); i++)
    {
        testPattern[i] = static_cast<uint8_t> (i);
    }
    storage->writeStorage (testPattern);

    auto readback = std::vector<uint8_t> (testPattern.size(), 0);
    storage->readStorage (0, readback);
    for (std::size_t i = 0; i < readback.size(); i++)
    {
        ASSERT_EQ (readback[i], static_cast<uint8_t> (i));
    }
}

/**
 * Simulates having the calibration the same SPI chip as the firmware, with accessOffset == zero.
 * The test writes a known pattern to the firmware area, then writes calibration data, and finally
 * tests that the firmware wasn't overwritten.
 *
 * EXAMPLE CONFIG: A device with one SPI-based memory, which is the boot memory for the CX3
 * firmware, and also contains the calibration data.  The chip is a 512kB memory, and data can only
 * be erased in 64kB blocks.
 *
 * The UVC firmware is slightly under 128kB (release) or about 150kB (debug), so the release
 * firmware fits in 2 sectors while the debug needs 3 sectors.  Putting the calibration in the
 * at address 192kB (accessOffset == 3 * sector size) allows either build to be used.
 *
 * The PicoFlexx calibration is slightly under 128kB, and so could fit with release firmware in a
 * 256kB chip.  Beware that the addresses may wrap round, so configuring the accessOffset to 192kB
 * on a 256kB chip may erase the first sector with the firmware.
 */
TEST_F (TestUvcExtensionArcticVariable, TestSpiPrimaryFlashAccessOffset)
{
    auto routing = SensorRoutingConfigSpi {0};
    auto config = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM}
                  .setImageSize (512 * 1024)
                  .setPageSize (256)
                  .setSectorSize (64 * 1024)
                  .setAccessOffset (3 * 64 * 1024);

    // The next lines create a way to access the firmware area.
    auto fullChipConfig = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM}
                          .setImageSize (512 * 1024)
                          .setPageSize (256)
                          .setSectorSize (64 * 1024);

    m_stubHardware->addSpiFlash (routing.getAddress(), fullChipConfig);
    auto storage = std::make_shared<StorageSpiFlashArctic> (config, m_spiBusAccess, routing.getAddress());
    auto firmwareStorage = std::make_shared<StorageSpiFlashArctic> (fullChipConfig, m_spiBusAccess, routing.getAddress());

    auto firmwarePattern = std::vector<uint8_t> (150 * 1024);
    for (std::size_t i = 0; i < firmwarePattern.size(); i++)
    {
        firmwarePattern[i] = 'F';
    }
    firmwareStorage->writeStorage (firmwarePattern);

    auto testPattern = std::vector<uint8_t> (config.sectorSize);
    for (std::size_t i = 0; i < testPattern.size(); i++)
    {
        testPattern[i] = static_cast<uint8_t> (i);
    }
    storage->writeStorage (testPattern);

    auto readback = std::vector<uint8_t> (testPattern.size(), 0);
    storage->readStorage (0, readback);
    for (std::size_t i = 0; i < readback.size(); i++)
    {
        ASSERT_EQ (readback[i], static_cast<uint8_t> (i));
    }

    auto firmwareReadback = std::vector<uint8_t> (firmwarePattern.size(), 0);
    firmwareStorage->readStorage (0, firmwareReadback);
    for (std::size_t i = 0; i < firmwareReadback.size(); i++)
    {
        ASSERT_EQ (firmwareReadback[i], firmwarePattern[i]);
    }
}

TEST_F (TestUvcExtensionArcticFixed, TestSpiPrimaryFlashAccessOffset)
{
    auto routing = SensorRoutingConfigSpi {0};
    auto config = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM}
                  .setImageSize (512 * 1024)
                  .setPageSize (256)
                  .setSectorSize (64 * 1024)
                  .setAccessOffset (3 * 64 * 1024);

    // The next lines create a way to access the firmware area.
    auto fullChipConfig = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM}
                          .setImageSize (512 * 1024)
                          .setPageSize (256)
                          .setSectorSize (64 * 1024);

    m_stubHardware->addSpiFlash (routing.getAddress(), fullChipConfig);
    auto storage = std::make_shared<StorageSpiFlashArctic> (config, m_spiBusAccess, routing.getAddress());
    auto firmwareStorage = std::make_shared<StorageSpiFlashArctic> (fullChipConfig, m_spiBusAccess, routing.getAddress());

    auto firmwarePattern = std::vector<uint8_t> (150 * 1024);
    for (std::size_t i = 0; i < firmwarePattern.size(); i++)
    {
        firmwarePattern[i] = 'F';
    }
    firmwareStorage->writeStorage (firmwarePattern);

    auto testPattern = std::vector<uint8_t> (config.sectorSize);
    for (std::size_t i = 0; i < testPattern.size(); i++)
    {
        testPattern[i] = static_cast<uint8_t> (i);
    }
    storage->writeStorage (testPattern);

    auto readback = std::vector<uint8_t> (testPattern.size(), 0);
    storage->readStorage (0, readback);
    for (std::size_t i = 0; i < readback.size(); i++)
    {
        ASSERT_EQ (readback[i], static_cast<uint8_t> (i));
    }

    auto firmwareReadback = std::vector<uint8_t> (firmwarePattern.size(), 0);
    firmwareStorage->readStorage (0, firmwareReadback);
    for (std::size_t i = 0; i < firmwareReadback.size(); i++)
    {
        ASSERT_EQ (firmwareReadback[i], firmwarePattern[i]);
    }
}

/**
 * Test that the image size limit causes an exception to be thrown.
 *
 * For this test, the storage class is configured with a very small memory, but the simulation is
 * still simulating a larger "physical" device than the storage class believes it has, so that any
 * exceptions are caused by checks in the storage class, not from the simulation.
 */
TEST_F (TestUvcExtensionArcticVariable, TestSpiSizeLimit)
{
    auto routing = SensorRoutingConfigSpi {1};
    auto config = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM}
                  .setImageSize (16 * 1024)
                  .setPageSize (256)
                  .setSectorSize (4 * 1024)
                  .setAccessOffset (12 * 1024);

    // Config for the simulated "physical" storage
    auto fullChipConfig = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM}
                          .setImageSize (512 * 1024)
                          .setPageSize (config.pageSize)
                          .setSectorSize (config.sectorSize);

    m_stubHardware->addSpiFlash (routing.getAddress(), fullChipConfig);
    auto storage = std::make_shared<StorageSpiFlashArctic> (config, m_spiBusAccess, routing.getAddress());

    // The size of the test pattern is arbitrary, and bigger than a sector
    auto testPattern = std::vector<uint8_t> (5000, 0);
    ASSERT_ANY_THROW (storage->writeStorage (testPattern));
}

TEST_F (TestUvcExtensionArcticFixed, TestSpiSizeLimit)
{
    auto routing = SensorRoutingConfigSpi {1};
    auto config = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM}
                  .setImageSize (16 * 1024)
                  .setPageSize (256)
                  .setSectorSize (4 * 1024)
                  .setAccessOffset (12 * 1024);

    // Config for the simulated "physical" storage
    auto fullChipConfig = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM}
                          .setImageSize (512 * 1024)
                          .setPageSize (config.pageSize)
                          .setSectorSize (config.sectorSize);

    m_stubHardware->addSpiFlash (routing.getAddress(), fullChipConfig);
    auto storage = std::make_shared<StorageSpiFlashArctic> (config, m_spiBusAccess, routing.getAddress());

    // The size of the test pattern is arbitrary, and bigger than a sector
    auto testPattern = std::vector<uint8_t> (5000, 0);
    ASSERT_ANY_THROW (storage->writeStorage (testPattern));
}
