/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/SensorRoutingConfigI2c.hpp>
#include <common/SensorRoutingConfigSpi.hpp>
#include <common/NarrowCast.hpp>
#include <gtest/gtest.h>

#include <memory>

using namespace royale::common;

namespace
{
    /** I2C address of the light sensor (standard for all modules) */
    const uint8_t ADDRESS_MAIN_IMAGER = 0x3d;

    /** SPI address used for the UVC devices (an enum handled by the device) */
    const uint8_t ADDRESS_SECONDARY_SPI = 0x1;
}

TEST (TestSensorRoutingConfig, I2cRouteConfig)
{
    SensorRoutingConfigI2c route {ADDRESS_MAIN_IMAGER};
    ASSERT_EQ (ADDRESS_MAIN_IMAGER, route.getAddress());
}

TEST (TestSensorRoutingConfig, SpiRouteConfig)
{
    SensorRoutingConfigSpi route {ADDRESS_SECONDARY_SPI};
    ASSERT_EQ (ADDRESS_SECONDARY_SPI, route.getAddress());
}

/**
 * Some bridge types can support I2C and SPI versions of the same device, and choose the right type
 * based on the sensor routing type.  For this it's important that the I2C and SPI classes are
 * different, even though both classes are just wrapped uint8_t's.
 */
TEST (TestSensorRoutingConfig, DistinctSubclasses)
{
    std::unique_ptr<ISensorRoutingConfig> i2c {new SensorRoutingConfigI2c {ADDRESS_MAIN_IMAGER}};
    ASSERT_NE (dynamic_cast<SensorRoutingConfigI2c *> (i2c.get()), nullptr);
    ASSERT_EQ (dynamic_cast<SensorRoutingConfigSpi *> (i2c.get()), nullptr);

    std::unique_ptr<ISensorRoutingConfig> spi {new SensorRoutingConfigSpi {ADDRESS_SECONDARY_SPI}};
    ASSERT_EQ (dynamic_cast<SensorRoutingConfigI2c *> (spi.get()), nullptr);
    ASSERT_NE (dynamic_cast<SensorRoutingConfigSpi *> (spi.get()), nullptr);
}

/**
 * Check if invalid I2C addresses are rejected by the SensorRoutingConfigI2c ctor.
 */
TEST (TestSensorRoutingConfig, InvalidI2cAddress)
{
    for (auto addr = 0x00u; addr < 0x08u; ++addr)
    {
        ASSERT_THROW (auto dummy = SensorRoutingConfigI2c (narrow_cast<uint8_t> (addr)), InvalidValue);
    }
    for (auto addr = 0x78u; addr < 0x100u; ++addr)
    {
        ASSERT_THROW (auto dummy = SensorRoutingConfigI2c (narrow_cast<uint8_t> (addr)), InvalidValue);
    }
}

/**
 * Check if valid I2C addresses are accepted by the SensorRoutingConfigI2c ctor.
 */
TEST (TestSensorRoutingConfig, ValidI2cAddress)
{
    for (auto addr = 0x08u; addr < 0x78u; ++addr)
    {
        ASSERT_NO_THROW (auto dummy = SensorRoutingConfigI2c (narrow_cast<uint8_t> (addr)));
    }
}
