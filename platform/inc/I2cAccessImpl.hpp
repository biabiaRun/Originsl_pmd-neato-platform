/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <pal/II2cBusAccess.hpp>

namespace platform
{
    class I2cAccessImpl : public royale::pal::II2cBusAccess
    {
    public:
        I2cAccessImpl(const char* devNode);
        virtual ~I2cAccessImpl() override;

        void readI2c (uint8_t devAddr,
                      royale::pal::I2cAddressMode addrMode,
                      uint16_t regAddr,
                      std::vector<uint8_t> &buffer) override;

        void writeI2c (uint8_t devAddr,
                       royale::pal::I2cAddressMode addrMode,
                       uint16_t regAddr,
                       const std::vector<uint8_t> &buffer) override;

        void setBusSpeed (uint32_t bps) override;

        std::size_t maximumDataSize () override;
    private:
        int m_fd;
    };
}
