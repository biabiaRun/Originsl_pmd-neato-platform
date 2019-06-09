/****************************************************************************\
 * Copyright (C) 2019 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <hal/IBridgeImager.hpp>
#include <royale/ICameraDevice.hpp>

#include <memory>

namespace spiFlashTool
{
    /**
     * Royale's L4 API allows arbitrary access to the imager, reading and writing registers.
     * Royale's internal components that talk directly to the imager use the IBridgeImager
     * interface.
     *
     * This is a wrapper to allow the internal SPI-via-Imager implementation (or any of the normal
     * IImager implementations) to run on top of the L4 API, so that it doesn't need a parallel
     * implementation to be maintained in the SPIFlashTool.
     *
     * It also tests the CameraStatus returned by the API functions, and throws exceptions as
     * needed.
     */
    class BridgeImagerL4 : public royale::hal::IBridgeImager
    {
    public:
        BridgeImagerL4 (std::shared_ptr <royale::ICameraDevice> camera);
        ~BridgeImagerL4() override;

        /**
         * Currently a no-op, which is OK for the flash tools because the imager will be reset
         * when the tool connects to the camera, and reset again when the tool disconnects.
         */
        void setImagerReset (bool state) override;

        void readImagerRegister (uint16_t regAddr, uint16_t &value) override;
        void writeImagerRegister (uint16_t regAddr, uint16_t value) override;
        void readImagerBurst (uint16_t firstRegAddr, std::vector<uint16_t> &values) override;
        void writeImagerBurst (uint16_t firstRegAddr, const std::vector<uint16_t> &values) override;
        void sleepFor (std::chrono::microseconds sleepDuration) override;

    private:
        std::shared_ptr<royale::ICameraDevice> m_cameraDevice;
    };
}
