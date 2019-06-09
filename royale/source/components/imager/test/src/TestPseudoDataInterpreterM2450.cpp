#include <gtest/gtest.h>

#include <common/ICapturedRawFrame.hpp>
#include <imager/M2450_A12/PseudoDataInterpreter.hpp>

#include <memory>
#include <vector>

using std::vector;
using std::unique_ptr;

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
     * The start of a PseudoData line captured from an EvalBoard, after
     * normalisation by the Bridge, for the first image of the first
     * capture (frame 4, image 0).
     */
    const vector<uint16_t> SAMPLE_4 =
    {
        0x0004, 0x000b, 0x0078, 0x0000, 0x0000, 0x0cfe, 0x05d9, 0x0041
    };

    /**
     * Start of a PseudoData line for the second image of the first capture
     * (frame 5, image 1).
     */
    const vector<uint16_t> SAMPLE_5 =
    {
        0x0005, 0x008b, 0x0078, 0x0040, 0x0000, 0x0cfe, 0x05d9, 0x0041
    };

    /**
     * Start of a PseudoData line for the first image of the second capture
     * (frame 8, image 0 of the next capture).
     */
    const vector<uint16_t> SAMPLE_8 =
    {
        0x0008, 0x000b, 0x0078, 0x0000, 0x0000, 0x0cfe, 0x05d9, 0x0041
    };
}

TEST (TestPseudoDataInterpreterM2450, MiraCE)
{
    royale::imager::M2450_A12::PseudoDataInterpreter interpreter;

    TpdiCapturedRawFrame sample4 {SAMPLE_4};
    ASSERT_EQ (4, interpreter.getFrameNumber (sample4));
    ASSERT_EQ (0, interpreter.getSequenceIndex (sample4));
    ASSERT_EQ (176, interpreter.getHorizontalSize (sample4));
    ASSERT_EQ (120, interpreter.getVerticalSize (sample4));
    ASSERT_EQ (0, interpreter.getBinning (sample4));

    //check for valid range (-40 < temp < 125)
    float tempCaptured = interpreter.getImagerTemperature (sample4);
    ASSERT_GT (tempCaptured, -40.f);
    ASSERT_LT (tempCaptured, 125.f);

    TpdiCapturedRawFrame sample5 {SAMPLE_5};
    ASSERT_EQ (5, interpreter.getFrameNumber (sample5));
    ASSERT_EQ (1, interpreter.getSequenceIndex (sample5));

    TpdiCapturedRawFrame sample8 {SAMPLE_8};
    ASSERT_EQ (8, interpreter.getFrameNumber (sample8));
    ASSERT_EQ (0, interpreter.getSequenceIndex (sample8));
}
