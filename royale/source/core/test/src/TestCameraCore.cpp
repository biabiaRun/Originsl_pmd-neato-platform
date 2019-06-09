/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <gtest/gtest.h>

#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <config/ICoreConfig.hpp>
#include <usecase/UseCaseFourPhase.hpp>
#include <device/CameraCore.hpp>
#include <hal/IImager.hpp>
#include <common/PseudoDataTwelveBitCalculator.hpp>
#include <common/MakeUnique.hpp>
#include <royale/CameraAccessLevel.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::config;
using namespace royale::usecase;

namespace
{
    class TestPDI : public PseudoDataTwelveBitCalculator
    {
    public:
        const uint16_t MIN_WIDTH = 20; // arbitrary value

        IPseudoDataInterpreter *clone() override
        {
            return new TestPDI (*this);
        }
    private:
        uint16_t getFrameNumber (const common::ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }
        uint16_t getReconfigIndex (const common::ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }
        uint16_t getSequenceIndex (const common::ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }
        uint8_t getBinning (const common::ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }
        uint16_t getHorizontalSize (const common::ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }
        uint16_t getVerticalSize (const common::ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }
        float getImagerTemperature (const common::ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }
        void getTemperatureRawValues (const common::ICapturedRawFrame &frame, uint16_t &vRef1, uint16_t &vNtc1, uint16_t &vRef2, uint16_t &vNtc2, uint16_t &offset) const override
        {
            throw NotImplemented();
        }
        uint16_t getRequiredImageWidth() const override
        {
            return MIN_WIDTH;
        }

        void getEyeSafetyError (const common::ICapturedRawFrame &frame, uint32_t &eyeError) const override
        {
            eyeError = 0u;
        }

        bool supportsExposureFromPseudoData() const override
        {
            return false;
        }

        uint32_t getExposureTime (const common::ICapturedRawFrame &frame, uint32_t modulationFrequency)  const override
        {
            throw royale::common::NotImplemented();
        }

    }; // class TestPDI

    class TestImager : public hal::IImager
    {
        void wake() override
        {
            // this gets called by CameraCore::init(). We'll just ignore it.
            // throw NotImplemented();
        }
        void initialize() override
        {
            throw NotImplemented();
        }
        void startCapture() override
        {
            throw NotImplemented();
        }
        void reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex) override
        {
            throw NotImplemented();
        }
        void reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex) override
        {
            throw NotImplemented();
        }
        void stopCapture() override
        {
            throw NotImplemented();
        }
        void sleep() override
        {
            // this gets called by CameraCore::init(). We'll just ignore it.
            // throw NotImplemented();
        }
        royale::usecase::VerificationStatus verifyUseCase (const royale::usecase::UseCaseDefinition &useCase, uint16_t roiCMin, uint16_t roiRMin, uint16_t flowControlRate) override
        {
            // this gets called by CameraCore::verifyUseCase().
            return VerificationStatus::SUCCESS; // laissez-faire
        }
        void executeUseCase (const royale::usecase::UseCaseDefinition &useCase, uint16_t roiCMin, uint16_t roiRMin, uint16_t flowControlRate) override
        {
            throw NotImplemented();
        }
        royale::Vector<std::size_t> getMeasurementBlockSizes() const override
        {
            throw NotImplemented();
        }
        std::string getSerialNumber() override
        {
            throw NotImplemented();
        }
        std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter() override
        {
            return makeUnique<TestPDI>();
        }
        void writeRegisters (const royale::Vector<royale::Pair<royale::String, uint64_t>> &registers) override
        {
            throw NotImplemented();
        }
        void readRegisters (royale::Vector<royale::Pair<royale::String, uint64_t>> &registers) override
        {
            throw NotImplemented();
        }
        void setExternalTrigger (bool useExternalTrigger) override
        {
            throw NotImplemented();
        }

    }; // class TestImager

    class TestBridgeDataReceiver : public hal::IBridgeDataReceiver
    {
        // implement IBufferCaptureReleaser
        void queueBuffer (royale::hal::ICapturedBuffer *buffer) override
        {
            throw NotImplemented();
        }

        // implement IBridgeDataReceiver
        void setBufferCaptureListener (hal::IBufferCaptureListener *collector) override
        {
            // this gets called by CameraCore::init(). We'll just ignore it.
            // throw NotImplemented();
        }
        std::size_t executeUseCase (int width, int height, std::size_t preferredBufferCount) override
        {
            throw NotImplemented();
        }
        float getPeakTransferSpeed() override
        {
            throw NotImplemented();
        }
        void startCapture() override
        {
            throw NotImplemented();
        }
        void stopCapture() override
        {
            throw NotImplemented();
        }
        bool isConnected() const override
        {
            throw NotImplemented();
        }
        royale::Vector<royale::Pair<royale::String, royale::String>> getBridgeInfo() override
        {
            throw NotImplemented();
        }
        void setEventListener (royale::IEventListener *listener) override
        {
            // this gets called by CameraCore::init(). We'll just ignore it.
            // throw NotImplemented();
        }

    }; // class TestBridgeDataReceiver

    class TestTemperatureSensor : public hal::ITemperatureSensor
    {
        float getTemperature() override
        {
            // This may get called from the FC conveyance thread even without capturing active
            return 15.f; // ICAO ISA MSL temperature
        }

    }; // class TestTemperatureSensor

    class TestStorage : public hal::INonVolatileStorage
    {
        royale::Vector<uint8_t> getModuleIdentifier() override
        {
            throw NotImplemented();
        }
        royale::String getModuleSuffix() override
        {
            throw NotImplemented();
        }
        royale::String getModuleSerialNumber() override
        {
            throw NotImplemented();
        }
        royale::Vector<uint8_t> getCalibrationData() override
        {
            throw NotImplemented();
        }
        uint32_t getCalibrationDataChecksum() override
        {
            throw NotImplemented();
        }
        void writeCalibrationData (const royale::Vector<uint8_t> &data) override
        {
            throw NotImplemented();
        }
        void writeCalibrationData (const royale::Vector<uint8_t> &calibrationData,
                                   const royale::Vector<uint8_t> &identifier,
                                   const royale::String &suffix,
                                   const royale::String &serialNumber) override
        {
            throw NotImplemented();
        }

    }; // class TestStorage

    class TestCoreConfig : public ICoreConfig
    {
    public:
        TestCoreConfig (uint16_t width, uint16_t height, const UseCaseList &useCaseList) :
            m_width (width),
            m_height (height),
            m_useCaseList (useCaseList)
        {
        }

        void getLensCenterDesign (uint16_t &column, uint16_t &row) const override
        {
            column = m_width / 2;
            row = m_height / 2;
        }

        uint16_t getMaxImageWidth() const override
        {
            return m_width;
        }

        uint16_t getMaxImageHeight() const override
        {
            return m_height;
        }

        const usecase::UseCaseList &getSupportedUseCases() const override
        {
            return m_useCaseList;
        }

        FrameTransmissionMode getFrameTransmissionMode() const override
        {
            return FrameTransmissionMode::SUPERFRAME;
        }

        royale::String getCameraName() const override
        {
            return "TestConfig";
        }

        float getTemperatureLimitSoft() const override
        {
            return 60.f;
        }

        float getTemperatureLimitHard() const override
        {
            return 65.f;
        }

        bool isAutoExposureSupported() const override
        {
            return false;
        }

        BandwidthRequirementCategory getBandwidthRequirementCategory() const override
        {
            return BandwidthRequirementCategory::NO_THROTTLING;
        }

    private:
        const uint16_t m_width;
        const uint16_t m_height;
        const UseCaseList m_useCaseList;
    };

} // anonymous namespace


TEST (TestCameraCore, InvalidImageWidth)
{

    // Create a valid core config
    royale::Vector<royale::ProcessingParameterMap> ppMap{ {} };
    auto ucd = std::make_shared<UseCaseDefinition> (UseCaseFourPhase (5, 30000000, { 50u, 1000u }, 1000u, 0));
    ucd->setImage (48, 96);
    UseCaseList ucList {UseCase ("testcase", ucd, ppMap) };
    auto coreConfig = std::make_shared<TestCoreConfig> (96, 96, ucList);

    auto imager = std::make_shared<TestImager>();
    auto bridgeDataReceiver = std::make_shared<TestBridgeDataReceiver>();
    auto tempSensor = std::make_shared<TestTemperatureSensor>();
    auto flash = std::make_shared<TestStorage>();

    auto core = makeUnique< royale::device::CameraCore > (
                    coreConfig,
                    imager,
                    bridgeDataReceiver,
                    tempSensor,
                    flash,
                    nullptr,
                    nullptr,
                    royale::CameraAccessLevel::L1);

    // Test the "good" case first:
    EXPECT_EQ (core->verifyUseCase (ucd.get()), VerificationStatus::SUCCESS);

    // Tweak config to produce an image too small to hold the pseudodata...
    ucd->setImage (16, 96);

    // ...and check.
    EXPECT_EQ (core->verifyUseCase (ucd.get()), VerificationStatus::REGION);
}
