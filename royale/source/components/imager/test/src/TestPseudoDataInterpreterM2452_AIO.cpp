#include <gmock/gmock.h>

#include <common/ICapturedRawFrame.hpp>
#include <imager/M2452/PseudoDataInterpreter_AIO.hpp>

#include <vector>

using testing::Eq;

namespace
{
    /**
     * Wraps one of the sample data vectors in to the right interface for
     * PseudoDataInterpreter to interpret.
     */
    class TpdiCapturedRawFrame : public royale::common::ICapturedRawFrame
    {
    public:
        explicit TpdiCapturedRawFrame (const std::vector<uint16_t> &buffer) :
            m_buffer (buffer)
        {
        }

        uint16_t *getImageData() override
        {
            throw royale::common::NotImplemented ("not supported for this test");
        }

        const uint16_t *getPseudoData() const override
        {
            return m_buffer.data();
        }

    private:
        std::vector<uint16_t> m_buffer;
    };

    /**
     * The start of a PseudoData line captured from a C2/M2452_B1x, after
     * normalisation by the Bridge, for the first superframe of the first
     * capture.
     */
    const std::vector<uint16_t> SAMPLE =
    {
        0x0109, 0x0000, 0x00e0, 0x00ac, 0x0003, 0x0332, 0x0076, 0x0002, 0x0039, 0x006a, 0x004f, 0x001b, 0x00e8, 0x0003, 0x0000, 0x0dac, 0x093d, 0x0b78, 0x0eff, 0x0a76, 0x0732, 0x0022, 0x0947, 0x02c7, 0x0e69, 0x0bf0, 0x0b79, 0x0669, 0x0030, 0x0524, 0x03a6, 0x09f8,
        0x02ba, 0x0910, 0x0f99, 0x0eb5, 0x06a6, 0x0d78, 0x08b4, 0x0bc9, 0x02c4, 0x095a, 0x0771, 0x015c, 0x020b, 0x052c, 0x0946, 0x09cd, 0x021b, 0x0027, 0x0fff, 0x01da, 0x0e13, 0x05ac, 0x0d52, 0x0940, 0x0a04, 0x0b30, 0x0dad, 0x0f34, 0x020b, 0x0f02, 0x0334, 0x06a3,
        0x0df4, 0x02b9, 0x09d1, 0x0437, 0x0026, 0x042c, 0x0c36, 0x0cda, 0x0cc4, 0x0820, 0x057f, 0x015b, 0x071d, 0x0b09, 0x06e9, 0x0558, 0x0f8a, 0x01a6, 0x0825, 0x0e1b, 0x0e30, 0x0d25, 0x0fae, 0x050d, 0x0c0a, 0x003d, 0x0405, 0x0918, 0x0c2a, 0x0571, 0x0039, 0x0b0f,
        0x0602, 0x0a05, 0x0ba1, 0x077a, 0x00ab, 0x0329, 0x0c08, 0x091a, 0x0070, 0x00d6, 0x07e8, 0x094d, 0x0d26, 0x028e, 0x01bf, 0x00ba, 0x002c, 0x0112, 0x0feb, 0x0006, 0x03af, 0x040e, 0x0a1b, 0x0063, 0x0ca1, 0x0020, 0x0d65, 0x0a00, 0x0018, 0x0020, 0x0104, 0x086b,
        0x0c01, 0x05a5, 0x00e7, 0x09f1, 0x0228, 0x03b1, 0x0d69, 0x0d8a, 0x028a, 0x0016, 0x0ee3, 0x0214, 0x0231, 0x0038, 0x0320, 0x0bde, 0x0f7c, 0x0065, 0x0039, 0x01f1, 0x0bc1, 0x0d03, 0x0820, 0x0de2, 0x0e60, 0x0920, 0x0d6f, 0x0a74, 0x066f, 0x0064, 0x0da0, 0x0f29,
        0x0c80, 0x017e, 0x060e, 0x0103, 0x00a0, 0x0010, 0x00b0, 0x000a, 0x0c03, 0x059c, 0x0d2b, 0x028a, 0x00a9, 0x0128, 0x08fc, 0x0d4a, 0x0688, 0x0132, 0x0bb3, 0x0eb0, 0x0a3e, 0x0f2d, 0x00b9, 0x0e0a, 0x0206, 0x08e1, 0x0eef, 0x0430, 0x0837, 0x0912, 0x0480, 0x0ae9
    };
}

class TestPseudoDataInterpreterM2452_AIO: public ::testing::Test
{
public:
    royale::imager::M2452::PseudoDataInterpreter_AIO interpreter;
    static const unsigned int vRef1I = 166;
    static const unsigned int vRef2I = 164;
    static const unsigned int vNtc1I = 165;
    static const unsigned int vNtc2I = 167;
};

TEST_F (TestPseudoDataInterpreterM2452_AIO, VerifyRawDataInterpretation)
{
    TpdiCapturedRawFrame sample {SAMPLE};

    ASSERT_EQ (0x109, interpreter.getFrameNumber (sample));
    ASSERT_EQ (0x103, interpreter.getSequenceIndex (sample));
    ASSERT_EQ (224, interpreter.getHorizontalSize (sample));
    ASSERT_EQ (172, interpreter.getVerticalSize (sample));
    ASSERT_EQ (1, interpreter.getBinning (sample));
}

TEST_F (TestPseudoDataInterpreterM2452_AIO, VerifyTemperatureRawData)
{
    royale::imager::M2452::PseudoDataInterpreter_AIO interpreter;
    TpdiCapturedRawFrame sample {SAMPLE};
    uint16_t vRef1 = 0, vRef2 = 0, vNtc1 = 0, vNtc2 = 0, offset = 0;

    interpreter.getTemperatureRawValues (sample, vRef1, vNtc1, vRef2, vNtc2, offset);

    ASSERT_THAT (vRef1, Eq (SAMPLE[vRef1I]));
    ASSERT_THAT (vRef2, Eq (SAMPLE[vRef2I]));
    ASSERT_THAT (vNtc1, Eq (SAMPLE[vNtc1I]));
    ASSERT_THAT (vNtc2, Eq (SAMPLE[vNtc2I]));
}

TEST_F (TestPseudoDataInterpreterM2452_AIO, Verify12Bit)
{
    uint16_t ref1Gold = 0x0fff;
    uint16_t ref2Gold = 0x0eff;
    uint16_t ntc1Gold = 0x00ff;
    uint16_t ntc2Gold = 0x00ef;
    uint16_t highBits = 0xf000;
    std::vector<uint16_t> pseudoData = SAMPLE;
    pseudoData[vRef1I] = highBits | ref1Gold;
    pseudoData[vRef2I] = highBits | ref2Gold;
    pseudoData[vNtc1I] = highBits | ntc1Gold;
    pseudoData[vNtc2I] = highBits | ntc2Gold;
    TpdiCapturedRawFrame sample {pseudoData};
    uint16_t vRef1 = 0, vRef2 = 0, vNtc1 = 0, vNtc2 = 0, offset = 0;

    interpreter.getTemperatureRawValues (sample, vRef1, vNtc1, vRef2, vNtc2, offset);

    ASSERT_THAT (vRef1, Eq (ref1Gold));
    ASSERT_THAT (vRef2, Eq (ref2Gold));
    ASSERT_THAT (vNtc1, Eq (ntc1Gold));
    ASSERT_THAT (vNtc2, Eq (ntc2Gold));
}

TEST_F (TestPseudoDataInterpreterM2452_AIO, ThrowsOnInvalidRefData)
{
    std::vector<uint16_t> pseudoData = SAMPLE;
    pseudoData[vRef1I] = 9;
    pseudoData[vRef2I] = 10;
    pseudoData[vNtc1I] = 5;
    pseudoData[vNtc2I] = 4;
    TpdiCapturedRawFrame sample {pseudoData};
    uint16_t vRef1 = 0, vRef2 = 0, vNtc1 = 0, vNtc2 = 0, offset = 0;

    ASSERT_THROW (interpreter.getTemperatureRawValues (sample, vRef1, vNtc1, vRef2, vNtc2, offset), royale::common::InvalidValue);
}

TEST_F (TestPseudoDataInterpreterM2452_AIO, ThrowsOnInvalidNtcData)
{
    std::vector<uint16_t> pseudoData = SAMPLE;
    pseudoData[vRef1I] = 10;
    pseudoData[vRef2I] = 9;
    pseudoData[vNtc1I] = 4;
    pseudoData[vNtc2I] = 5;
    TpdiCapturedRawFrame sample {pseudoData};
    uint16_t vRef1 = 0, vRef2 = 0, vNtc1 = 0, vNtc2 = 0, offset = 0;

    ASSERT_THROW (interpreter.getTemperatureRawValues (sample, vRef1, vNtc1, vRef2, vNtc2, offset), royale::common::InvalidValue);
}
