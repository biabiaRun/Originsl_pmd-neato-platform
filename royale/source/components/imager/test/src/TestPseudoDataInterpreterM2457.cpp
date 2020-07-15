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
#include <imager/M2457/PseudoDataInterpreter.hpp>

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
 * Test the interpreter with a PseudoData line
 */
TEST (TestPseudoDataInterpreterM2457, InterpretPseudoData)
{
    const std::vector<uint16_t> sampleData =
    {
        //Not tested data (starting at pixel 0)
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

        //raw temperature values (starting at pixel 47)
        0x0024, 0x0021, 0x0048, 0x0042
    };

    royale::imager::M2457::PseudoDataInterpreter interpreter {true};
    TpdiCapturedRawFrame sample {sampleData};

    //Test raw temperature values reading
    auto values = interpreter.getTemperatureRawValues (sample);
    ASSERT_THAT (static_cast<int> (values.size()), Eq(4));
    uint16_t vRef1 = values[0];
    uint16_t vRef2 = values[1];
    uint16_t vNtc1 = values[2];
    uint16_t vNtc2 = values[3];
    EXPECT_THAT (vRef1, Eq (0x0021));
    EXPECT_THAT (vNtc1, Eq (0x0042));
    EXPECT_THAT (vRef2, Eq (0x0024));
    EXPECT_THAT (vNtc2, Eq (0x0048));
}

