/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <gmock/gmock.h>

#include <common/ICapturedRawFrame.hpp>
#include <imager/M2455/PseudoDataInterpreter.hpp>

#include <vector>

using testing::Eq;
using testing::Ge;

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

}

/**
 * Test the interpreter with a PseudoData line for the MiraDonna in RAW mode.
 * These are theoretical values, as we're not using RAW mode for the bring-up.
 */
TEST (TestPseudoDataInterpreterM2455, DonnaRawMode)
{
    const std::vector<uint16_t> sampleData =
    {
        0x0001, 0x0000, 0x0001, 0x0001, 0x0109, 0x0103, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x01bf, 0x0000, 0x00a7, 0x0240, 0x0014, 0x0009,
        0x0014, 0x0009, 0x0000, 0x0000, 0x0000, 0x0000, 0x0040, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0100, 0x0016, 0x0811, 0x07c3, 0x07ff, 0x07e3, 0x07da, 0x07e0,
        0x0815, 0x0813, 0x07ce, 0x07dc, 0x0815, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff,
        0x00ff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x001e, 0x0023,
        0x0081, 0x000a, 0x0011, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
    };

    royale::imager::M2455::PseudoDataInterpreter interpreter {true};
    ASSERT_THAT (sampleData.size(), Ge (interpreter.getRequiredImageWidth()));
    TpdiCapturedRawFrame sample {sampleData};

    EXPECT_THAT (interpreter.getFrameNumber (sample), Eq (0x0109));
    EXPECT_THAT (interpreter.getSequenceIndex (sample), Eq (0x0003))
            << "Sequence counter (only 7-bit)";
    EXPECT_THAT (interpreter.getHorizontalSize (sample), Eq (448));
    EXPECT_THAT (interpreter.getVerticalSize (sample), Eq (168));
    EXPECT_THAT (interpreter.getBinning (sample), Eq (1));
    uint32_t eyeSafetyError = 0;
    ASSERT_NO_THROW (interpreter.getEyeSafetyError (sample, eyeSafetyError));
    EXPECT_TRUE (eyeSafetyError);
}

/**
 * Test the interpreter with a PseudoData line for the MiraDonna in MBSEQ1
 * mode, captured from a real device.
 */
TEST (TestPseudoDataInterpreterM2455, DonnaMbseq1Mode)
{
    const std::vector<uint16_t> sampleData =
    {
        0x0002, 0x0000, 0x0000, 0x0037, 0x01f2, 0x0003, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x01bf, 0x0000, 0x00a7, 0x0240, 0x0014, 0x0009,
        0x0014, 0x0009, 0x0000, 0x0000, 0x0000, 0x0000, 0x0040, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0100, 0x0016, 0x0811, 0x07c3, 0x07ff, 0x07e3, 0x07da, 0x07e0,
        0x0815, 0x0813, 0x07ce, 0x07dc, 0x0815, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff,
        0x00ff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x001e, 0x0023,
        0x0081, 0x000a, 0x0011, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
    };

    royale::imager::M2455::PseudoDataInterpreter interpreter {true};
    ASSERT_THAT (sampleData.size(), Ge (interpreter.getRequiredImageWidth()));
    TpdiCapturedRawFrame sample {sampleData};

    EXPECT_THAT (interpreter.getFrameNumber (sample), Eq (0x01f2));
    EXPECT_THAT (interpreter.getSequenceIndex (sample), Eq (0x0003))
            << "Sequence counter (only 7-bit)";
    EXPECT_THAT (interpreter.getHorizontalSize (sample), Eq (448));
    EXPECT_THAT (interpreter.getVerticalSize (sample), Eq (168));
    EXPECT_THAT (interpreter.getBinning (sample), Eq (1));
    uint32_t eyeSafetyError = 0;
    ASSERT_NO_THROW (interpreter.getEyeSafetyError (sample, eyeSafetyError));
    EXPECT_TRUE (eyeSafetyError);
}
