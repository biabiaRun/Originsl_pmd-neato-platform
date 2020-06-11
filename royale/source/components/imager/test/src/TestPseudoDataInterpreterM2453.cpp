/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <gmock/gmock.h>

#include <common/ICapturedRawFrame.hpp>
#include <imager/M2453/PseudoDataInterpreter.hpp>
#include <imager/M2453/PseudoDataInterpreter_B11.hpp>

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
     * The start of a PseudoData line as defined by the ifx jama item GID-1771802.
     * Content is filled with dummy values as no real-life example was available.
     */
    const std::vector<uint16_t> SAMPLE_A11 =
    {
        0x0000, 0x0001, 0x0001, 0x0109, 0x0103, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        //roi (starting at pixel 22)
        0x0000, 0x00DF, 0x0000, 0x00AB,
        //any data (starting at pixel 26)
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000,
        //raw temperature values (starting at pixel 46)
        0x0024, 0x0021, 0x0048, 0x0042
    };
    const std::vector<uint16_t> SAMPLE_B11 =
    {
        0x0000, 0x0001, 0x0001, 0x0109, 0x0000, 0x0103, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        //roi (starting at pixel 23)
        0x0000, 0x00DF, 0x0000, 0x00AB,
        //any data (starting at pixel 27)
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000,
        //raw temperature values (starting at pixel 47)
        0x0024, 0x0021, 0x0048, 0x0042
    };
}

TEST (TestPseudoDataInterpreterM2453, M2453_A11_PD)
{
    royale::imager::M2453_A11::PseudoDataInterpreter interpreter;

    TpdiCapturedRawFrame sample {SAMPLE_A11};
    EXPECT_THAT (interpreter.getFrameNumber (sample), Eq (0x0109));
    EXPECT_THAT (interpreter.getSequenceIndex (sample), Eq (0x0103));
    EXPECT_THAT (interpreter.getHorizontalSize (sample), Eq (224));
    EXPECT_THAT (interpreter.getVerticalSize (sample), Eq (172));
    EXPECT_THAT (interpreter.getBinning (sample), Eq (1));

    //Test raw temperature values reading
    auto values = interpreter.getTemperatureRawValues (sample);
    uint16_t &vRef1 = values[0];
    uint16_t &vRef2 = values[1];
    uint16_t &vNtc1 = values[2];
    uint16_t &vNtc2 = values[3];
    EXPECT_THAT (vRef1, Eq (0x0021));
    EXPECT_THAT (vNtc1, Eq (0x0042));
    EXPECT_THAT (vRef2, Eq (0x0024));
    EXPECT_THAT (vNtc2, Eq (0x0048));
}

TEST (TestPseudoDataInterpreterM2453, M2453_B11_PD)
{
    royale::imager::M2453_B11::PseudoDataInterpreterB11 interpreter;

    TpdiCapturedRawFrame sample {SAMPLE_B11};
    EXPECT_THAT (interpreter.getFrameNumber (sample), Eq (0x0109));
    EXPECT_THAT (interpreter.getSequenceIndex (sample), Eq (0x0103));
    EXPECT_THAT (interpreter.getHorizontalSize (sample), Eq (224));
    EXPECT_THAT (interpreter.getVerticalSize (sample), Eq (172));
    EXPECT_THAT (interpreter.getBinning (sample), Eq (1));

    //Test raw temperature values reading
    auto values = interpreter.getTemperatureRawValues (sample);
    ASSERT_THAT (static_cast<int> (values.size()), Eq(4));
    uint16_t &vRef1 = values[0];
    uint16_t &vRef2 = values[1];
    uint16_t &vNtc1 = values[2];
    uint16_t &vNtc2 = values[3];
    EXPECT_THAT (vRef1, Eq (0x0021));
    EXPECT_THAT (vNtc1, Eq (0x0042));
    EXPECT_THAT (vRef2, Eq (0x0024));
    EXPECT_THAT (vNtc2, Eq (0x0048));
}
