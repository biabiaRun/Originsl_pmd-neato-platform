#include <storage/StorageFormatPicoLegacy.hpp>
#include <pal/IStorageWriteFullOverwrite.hpp>
#include <config/FlashMemoryConfig.hpp>

#include <EndianConversion.hpp>
#include <common/Crc32.hpp>
#include <common/exceptions/OutOfBounds.hpp>
#include <royale/Vector.hpp>
#include <NarrowCast.hpp>

#include <gtest/gtest.h>

#include <RoyaleLogger.hpp>

#include <algorithm>
#include <memory>

using namespace royale::storage;
using namespace royale::common;
using namespace royale::pal;
using namespace royale::config;
using namespace royale;

namespace
{
    FlashMemoryConfig mockMemoryConfig{ FlashMemoryConfig{ FlashMemoryConfig::FlashMemoryType::PICO_PAGED }
        .setImageSize (2000000)
        .setPageSize (256)
        .setSectorSize (1024 * 64) };


    class MockBridgeWithPagedFlash :
        public IBridgeWithPagedFlash
    {
    public:
        MockBridgeWithPagedFlash (const FlashMemoryConfig &memConfig, const std::vector<uint8_t> &data) :
            m_memConfig (memConfig)
        {

            auto realSize = memConfig.imageSize + 2 * memConfig.sectorSize - 1;
            m_rawData.resize (realSize, 0u);
            if (!data.empty())
            {
                std::memcpy (&m_rawData[memConfig.imageSize - data.size()], &data[0], data.size());
            }
        }

        ~MockBridgeWithPagedFlash() = default;

        void readFlashPage (std::size_t page, std::vector<uint8_t> &recvBuffer, std::size_t noPages) override
        {
            if (page * m_memConfig.pageSize + noPages * m_memConfig.pageSize > m_rawData.size())
            {
                throw OutOfBounds();
            }
            recvBuffer.resize (m_memConfig.pageSize * noPages);
            std::fill (recvBuffer.begin(), recvBuffer.end(), 0);

            std::memcpy (&recvBuffer[0], &m_rawData[page * m_memConfig.pageSize], m_memConfig.pageSize * noPages);
        }

        void writeFlashPage (std::size_t page, const std::vector<uint8_t> &sndBuffer, std::size_t noPages) override
        {
            if (page * m_memConfig.pageSize + noPages * m_memConfig.pageSize > m_rawData.size())
            {
                throw OutOfBounds();
            }

            std::memcpy (&m_rawData[page * m_memConfig.pageSize], &sndBuffer[0], m_memConfig.pageSize * noPages);

        }

        void eraseSectors (std::size_t startSector, std::size_t noSectors) override
        {
            if (startSector * m_memConfig.sectorSize + noSectors * m_memConfig.sectorSize > m_rawData.size())
            {
                throw OutOfBounds();
            }

            std::fill (&m_rawData[startSector * m_memConfig.sectorSize],
                       &m_rawData[startSector * m_memConfig.sectorSize + noSectors * m_memConfig.sectorSize], 0);
        }

    private:
        std::vector<uint8_t> m_rawData;

        FlashMemoryConfig m_memConfig;
    };
}

namespace
{
    const std::vector<uint8_t> magic {'P', 'M', 'D', 'T', 'E', 'C'};

    /**
     * The header for a pico flexx.
     */
    std::vector<uint8_t> getFlexxImage (const uint32_t &serialNumber, const uint32_t &hardwareRevision,
                                        const std::vector<uint8_t> &calibration, const FlashMemoryConfig &memConfig)
    {
        std::vector<uint8_t> image;

        pushBackIterable (image, calibration);

        pushBackIterable (image, magic);
        image.push_back (0); // 2 Bytes padding
        image.push_back (0);
        pushBack32 (image, 100u); // headerVersion
        pushBack32 (image, serialNumber); // serialNumber
        pushBack32 (image, hardwareRevision); // hardwareRevision
        pushBack32 (image, narrow_cast<uint32_t> (memConfig.imageSize - 28 - calibration.size())); // calibrationAddress (28 = sizeof header)
        pushBack32 (image, narrow_cast<uint32_t> (calibration.size())); // calibrationSize
        return image;
    }
}

TEST (TestStorageFormatPicoLegacy, TestWriteRead)
{
    const std::vector<uint8_t> calib{ 1, 2, 3, 4 };
    auto bridge = std::make_shared<MockBridgeWithPagedFlash> (mockMemoryConfig, std::vector<uint8_t>());

    std::unique_ptr<StorageFormatPicoLegacy> flash;
    ASSERT_NO_THROW (flash.reset (new StorageFormatPicoLegacy (bridge, mockMemoryConfig)));

    ASSERT_NO_THROW (flash->writeCalibrationData (Vector<uint8_t>::fromStdVector (calib)));

    Vector<uint8_t> data;
    ASSERT_NO_THROW (data = flash->getCalibrationData());
    ASSERT_EQ (data.size(), calib.size());
    for (std::size_t i = 0; i < calib.size(); i++)
    {
        ASSERT_EQ (data[i], calib[i]);
    }
}

TEST (TestStorageFormatPicoLegacy, TestV7Functions)
{
    const std::vector<uint8_t> calib{ 1, 2, 3, 4 };
    const auto image = getFlexxImage (1234, 0x1156DA3A, calib, mockMemoryConfig);
    auto bridge = std::make_shared<MockBridgeWithPagedFlash> (mockMemoryConfig, image);

    std::unique_ptr<StorageFormatPicoLegacy> flash;
    ASSERT_NO_THROW (flash.reset (new StorageFormatPicoLegacy (bridge, mockMemoryConfig)));

    Vector<uint8_t> data;
    ASSERT_NO_THROW (data = flash->getCalibrationData());
    ASSERT_EQ (data.size(), calib.size());
    for (std::size_t i = 0; i < calib.size(); i++)
    {
        ASSERT_EQ (data[i], calib[i]);
    }

    // Check module identifier
    Vector<uint8_t> moduleID;
    ASSERT_NO_THROW (moduleID = flash->getModuleIdentifier());
    ASSERT_EQ (moduleID[0], 0x3A);
    ASSERT_EQ (moduleID[1], 0xDA);
    ASSERT_EQ (moduleID[2], 0x56);
    ASSERT_EQ (moduleID[3], 0x11);

    // Check module serial
    String moduleSerial;
    ASSERT_NO_THROW (moduleSerial = flash->getModuleSerialNumber());
    ASSERT_STREQ (moduleSerial.c_str (), "1234");

    // Check module suffix
    String moduleSuffix;
    ASSERT_NO_THROW (moduleSuffix = flash->getModuleSuffix());
    ASSERT_STREQ (moduleSuffix.c_str(), "");
}

