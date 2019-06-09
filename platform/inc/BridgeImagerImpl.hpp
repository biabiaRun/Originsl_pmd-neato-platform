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

#include <memory>

#include <hal/IBridgeImager.hpp>
#include <pal/Access2I2cDeviceAdapter.hpp>

namespace platform
{
    class BridgeImagerImpl : public royale::hal::IBridgeImager
    {
    public:
        BridgeImagerImpl(std::shared_ptr<royale::pal::Access2I2cDeviceAdapter> i2c_adapter);
        ~BridgeImagerImpl();

        void setImagerReset (bool state) override;

        void readImagerRegister (uint16_t regAddr, uint16_t &value) override;

        void writeImagerRegister (uint16_t regAddr, uint16_t value) override;

        void readImagerBurst (uint16_t firstRegAddr, std::vector<uint16_t> &values) override;

        void writeImagerBurst (uint16_t firstRegAddr, const std::vector<uint16_t> &values) override;

        void sleepFor (std::chrono::microseconds sleepDuration) override;
    private:
        std::shared_ptr<royale::pal::Access2I2cDeviceAdapter> m_adapter;
    };
}
