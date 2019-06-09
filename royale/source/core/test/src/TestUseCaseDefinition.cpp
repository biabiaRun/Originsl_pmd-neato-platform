#include <gtest/gtest.h>
#include <royale/Status.hpp>
#include <royale/Vector.hpp>
#include <usecase/UseCaseDefinition.hpp>
#include <usecase/UseCaseEightPhase.hpp>
#include <usecase/UseCaseFourPhase.hpp>
#include <usecase/UseCaseInterleavedXHt.hpp>
#include <usecase/UseCaseMixedXHt.hpp>
#include <usecase/UseCaseMixedIrregularXHt.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/OutOfBounds.hpp>

using namespace royale::usecase;
using royale::StreamId;

namespace
{
    void testDutyCycle (UseCaseDefinition &ucd, double dutyCycleSet, RawFrameSet::DutyCycle dutyCycleExpected)
    {
        ucd.setDutyCycle (dutyCycleSet);

        for (size_t idx = 0; idx < ucd.getRawFrameSets().size(); idx++)
        {
            ASSERT_EQ (dutyCycleExpected, ucd.getRawFrameSets().at (idx).dutyCycle);
        }
    }
}

TEST (TestUseCaseDefinition, operatorEqual)
{
    using UcdPtr = std::unique_ptr<UseCaseDefinition>;
    UcdPtr test4 (new UseCaseFourPhase (45u, 30000000, {50u, 1000u}, 1000u, 1000u));
    UcdPtr testSSCOn (new UseCaseFourPhase (45u, 30000000, { 50u, 1000u }, 1000u, 1000u, ExposureGray::Off, IntensityPhaseOrder::IntensityLastPhase, true));
    UcdPtr testSSCOff (new UseCaseFourPhase (45u, 30000000, { 50u, 1000u }, 1000u, 1000u, ExposureGray::Off, IntensityPhaseOrder::IntensityLastPhase, false));
    UcdPtr test8 (new UseCaseEightPhase (10u, 30000000, 20200000, { 200u, 1000u }, 1000u, 1000u, 1000u));
    UcdPtr test4_mod (new UseCaseFourPhase (45u, 20000000, {50u, 1000u}, 1000u, 1000u));
    UcdPtr test4_same (new UseCaseFourPhase (45u, 30000000, {50u, 1000u}, 1000u, 1000u));
    UcdPtr testM (new UseCaseMixedXHt (5u, 5u, 30000000, 30000000, 30000000,
    {50u, 1000u}, {50u, 1000u}, 1000u, 1000u, 1000u, 1000u, 1000u));
    UcdPtr testM_ratio (new UseCaseMixedXHt (5u, 9u, 30000000, 30000000, 30000000,
    { 50u, 1000u }, { 50u, 1000u }, 1000u, 1000u, 1000u, 1000u, 1000u));
    UcdPtr testM_rate (new UseCaseMixedXHt (1u, 5u, 30000000, 30000000, 30000000,
    { 50u, 1000u }, { 50u, 1000u }, 1000u, 1000u, 1000u, 1000u, 1000u));
    UcdPtr testM_limit (new UseCaseMixedXHt (5u, 5u, 30000000, 30000000, 30000000,
    { 111u, 1000u }, { 50u, 1000u }, 1000u, 1000u, 1000u, 1000u, 1000u));
    UcdPtr testMI (new UseCaseMixedIrregularXHt (5u, 5u, 30000000, 30000000, 30000000,
    { 50u, 1000u }, { 50u, 1000u }, 1000u, 1000u, 1000u, 1000u, 1000u));
    UcdPtr testMI_ratio (new UseCaseMixedIrregularXHt (5u, 9u, 30000000, 30000000, 30000000,
    { 50u, 1000u }, { 50u, 1000u }, 1000u, 1000u, 1000u, 1000u, 1000u));
    UcdPtr testMI_rate (new UseCaseMixedIrregularXHt (1u, 5u, 30000000, 30000000, 30000000,
    { 50u, 1000u }, { 50u, 1000u }, 1000u, 1000u, 1000u, 1000u, 1000u));
    UcdPtr testMI_limit (new UseCaseMixedIrregularXHt (5u, 5u, 30000000, 30000000, 30000000,
    { 111u, 1000u }, { 50u, 1000u }, 1000u, 1000u, 1000u, 1000u, 1000u));
    UcdPtr testI (new UseCaseInterleavedXHt (5u, 5u, 30000000, 30000000, 30000000,
    { 50u, 1000u }, { 50u, 1000u }, 1000u, 1000u, 1000u, 1000u, 1000u));

    EXPECT_FALSE (*testSSCOn == *testSSCOff);
    EXPECT_FALSE (*test4 == *test8);
    EXPECT_FALSE (*test4 == *test4_mod);
    EXPECT_TRUE (*test4 == *test4_same);
    EXPECT_FALSE (*test4 == *testM);
    EXPECT_FALSE (*test8 == *testM);
    EXPECT_FALSE (*testM == *testM_ratio);
    EXPECT_FALSE (*testM == *testM_ratio);
    EXPECT_FALSE (*testM == *testM_rate);
    EXPECT_FALSE (*testM_ratio == *testM_rate);
    EXPECT_FALSE (*testM == *testM_limit);
    EXPECT_FALSE (*testM == *testI);

    EXPECT_FALSE (*test4 == *testMI);
    EXPECT_FALSE (*test8 == *testMI);
    EXPECT_FALSE (*testMI == *testMI_ratio);
    EXPECT_FALSE (*testMI == *testMI_ratio);
    EXPECT_FALSE (*testMI == *testMI_rate);
    EXPECT_FALSE (*testMI_ratio == *testMI_rate);
    EXPECT_FALSE (*testMI == *testMI_limit);
    EXPECT_FALSE (*testMI == *testI);
}

namespace
{
    /**
     * Immediately after construction, this class will pass the verifyClassInvariants() test.  Once
     * any of the other methods are called, it will fail.
     *
     * Each instance of this class only supports calling one method, once, and then calling
     * verifyClassInvariants().  Unless otherwise documented, calling two methods before
     * verifyClassInvariants leads to undefined behaviour.
     *
     * Based on UseCaseFourPhase because UseCaseFourPhase is approximately the minimum needed to
     * pass the tests in verifyClassInvariants(); less would fail for multiple reasons.
     */
    class BreakableFourPhaseUseCase : public UseCaseFourPhase
    {
    public:
        BreakableFourPhaseUseCase() :
            UseCaseFourPhase (45u, 30000000,
        {
            50u, 1000u
        }, 1000u, 1000u, ExposureGray::Off, IntensityPhaseOrder::IntensityLastPhase)
        {
            // should not throw
            verifyClassInvariants();
        }

        void addAnEmptyStream()
        {
            createStream();
        }

        /**
         * To the stream that UseCaseFourPhase created, add a new FrameGroup.
         *
         * This method itself should not throw, UCD::constructFrameGroup supports making an
         * interleaved use case with this call sequence.
         */
        void addMismatchedFrameGroup()
        {
            // Using the UseCaseFourPhase's sets as a template, create a group with no gray
            royale::Vector<RawFrameSet> frameGroup = getRawFrameSets();
            ASSERT_EQ (2u, frameGroup.size());
            frameGroup.resize (1u);

            auto stream = getStream (getStreamIds() [0]);
            constructFrameGroup (stream, frameGroup, RawFrameSet::Alignment::CLOCK_ALIGNED);
            // Adding an ES group and then the gray from the UCFourPhase would make a valid UCD
        }

        void addEmptyFrameGroup()
        {
            auto stream = getStream (getStreamIds() [0]);
            constructFrameGroup (stream, {}, RawFrameSet::Alignment::CLOCK_ALIGNED);
        }

        void exceedMaxRate()
        {
            m_targetRate = static_cast<uint16_t> (getMaxRate() + 1);
        }

        /**
         * Change one of the raw frame sets to an exposure index that's out-of-bounds.
         */
        void exceedExposureGroupIndices ()
        {
            m_rawFrameSet.at (1).exposureGroupIdx = 2;
        }

        void useOnlyExposureGroup (ExposureGroupIdx index)
        {
            for (auto &rawFrameSet : m_rawFrameSet)
            {
                rawFrameSet.exposureGroupIdx = index;
            }
        }
    };
}

TEST (TestUseCaseDefinition, verifyClassInvariants)
{
    using LogicError = royale::common::LogicError;

    // As UCD::verifyClassInvariants()'s documentation explains, the base class itself isn't a
    // complete use case.
    {
        UseCaseDefinition ucd;
        ASSERT_THROW (ucd.verifyClassInvariants(), LogicError);
    }


    /* For reference, a proper set of arguments (this would not throw)
    ASSERT_THROW (UseCaseFourPhase (45u, 30000000, {50u, 1000u}, 1000u, 1000u));
    */

    // Max frame rate less than min frame rate (which is 1, from UCD::UCD (maxRate))
    ASSERT_THROW (UseCaseFourPhase (0u, 30000000, {50u, 1000u}, 1000u, 1000u), LogicError);
    // Exposure time outside the limits
    ASSERT_THROW (UseCaseFourPhase (45u, 30000000, {50u, 1000u}, 49u, 1000u), LogicError);
    ASSERT_THROW (UseCaseFourPhase (45u, 30000000, {50u, 1000u}, 1000u, 49u), LogicError);
    ASSERT_THROW (UseCaseFourPhase (45u, 30000000, {50u, 1000u}, 1001u, 1000u), LogicError);
    ASSERT_THROW (UseCaseFourPhase (45u, 30000000, {50u, 1000u}, 1000u, 1001u), LogicError);

    ASSERT_NO_THROW (BreakableFourPhaseUseCase()) << "BreakableFourPhaseUseCase's constructor shouldn't fail";
    {
        BreakableFourPhaseUseCase ucd;
        ASSERT_NO_THROW (ucd.addAnEmptyStream());
        ASSERT_THROW (ucd.verifyClassInvariants(), LogicError);
    }
    {
        BreakableFourPhaseUseCase ucd;
        ASSERT_NO_THROW (ucd.addMismatchedFrameGroup());
        ASSERT_THROW (ucd.verifyClassInvariants(), LogicError);
    }
    {
        BreakableFourPhaseUseCase ucd;
        ASSERT_NO_THROW (ucd.addEmptyFrameGroup());
        ASSERT_THROW (ucd.verifyClassInvariants(), LogicError);
    }
    {
        BreakableFourPhaseUseCase ucd;
        ASSERT_NO_THROW (ucd.exceedMaxRate());
        ASSERT_THROW (ucd.verifyClassInvariants(), LogicError);
    }
    {
        BreakableFourPhaseUseCase ucd;
        ASSERT_NO_THROW (ucd.exceedExposureGroupIndices ());
        ASSERT_THROW (ucd.verifyClassInvariants(), LogicError);
    }
    {
        BreakableFourPhaseUseCase ucd;
        ASSERT_NO_THROW (ucd.useOnlyExposureGroup (0));
        ASSERT_THROW (ucd.verifyClassInvariants(), LogicError);
    }
    {
        BreakableFourPhaseUseCase ucd;
        ASSERT_NO_THROW (ucd.useOnlyExposureGroup (1));
        ASSERT_THROW (ucd.verifyClassInvariants(), LogicError);
    }
    {
        BreakableFourPhaseUseCase ucd;
        // This should fail for two reasons (unused groups and out-of-bounds index).
        ASSERT_NO_THROW (ucd.useOnlyExposureGroup (2));
        ASSERT_THROW (ucd.verifyClassInvariants(), LogicError);
    }

    // Assuming a 12-bit wrap-round frame counter, having more than 2047 frames means the last frame
    // may have a number "less" than the first one.
    ASSERT_THROW (UseCaseMixedXHt (5u, 410u, 30000000, 20200000, 20600000, { 200u, 1000u }, { 200u, 1000u },
                                   1000u, 1000u, 1000u, 1000u, 1000u), LogicError);
}

/**
 * As an implementer of the FrameCollector, I want to convert the raw frame set IDs to individual
 * frames.
 */
TEST (TestUseCaseDefinition, getSequenceIndicesForRawFrameSet)
{
    {
        UseCaseEightPhase ucd (5u, 30000000, 20200000, { 200u, 1000u }, 1000u, 1000u, 1000u, ExposureGray::Off, IntensityPhaseOrder::IntensityFirstPhase);
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (0), (royale::Vector<uint16_t> {0}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (1), (royale::Vector<uint16_t> {1, 2, 3, 4}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (2), (royale::Vector<uint16_t> {5, 6, 7, 8}));
    }
    {
        UseCaseEightPhase ucd (5u, 30000000, 20200000, { 200u, 1000u }, 1000u, 1000u, 1000u, ExposureGray::Off, IntensityPhaseOrder::IntensityLastPhase);
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (0), (royale::Vector<uint16_t> {0, 1, 2, 3}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (1), (royale::Vector<uint16_t> {4, 5, 6, 7}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (2), (royale::Vector<uint16_t> {8}));
    }
    {
        UseCaseMixedXHt ucd (5u, 1u, 30000000, 20200000, 20600000, { 200u, 1000u }, { 200u, 1000u },
                             1000u, 1000u, 1000u, 1000u, 1000u, ExposureGray::Off, ExposureGray::Off, IntensityPhaseOrder::IntensityFirstPhase);
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (0), (royale::Vector<uint16_t> {0}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (1), (royale::Vector<uint16_t> {1, 2, 3, 4}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (2), (royale::Vector<uint16_t> {5}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (3), (royale::Vector<uint16_t> {6, 7, 8, 9}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (4), (royale::Vector<uint16_t> {10, 11, 12, 13}));
    }
    {
        UseCaseMixedXHt ucd (5u, 1u, 30000000, 20200000, 20600000, { 200u, 1000u }, { 200u, 1000u },
                             1000u, 1000u, 1000u, 1000u, 1000u, ExposureGray::Off, ExposureGray::Off, IntensityPhaseOrder::IntensityLastPhase);
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (0), (royale::Vector<uint16_t> {0, 1, 2, 3}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (1), (royale::Vector<uint16_t> {4}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (2), (royale::Vector<uint16_t> {5, 6, 7, 8}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (3), (royale::Vector<uint16_t> {9, 10, 11, 12}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (4), (royale::Vector<uint16_t> {13}));
    }
    {
        UseCaseMixedXHt ucd (5u, 5u, 30000000, 20200000, 20600000, { 200u, 1000u }, { 200u, 1000u },
                             1000u, 1000u, 1000u, 1000u, 1000u, ExposureGray::Off, ExposureGray::Off, IntensityPhaseOrder::IntensityFirstPhase);
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (0), (royale::Vector<uint16_t> {0}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (1), (royale::Vector<uint16_t> {1, 2, 3, 4}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (2), (royale::Vector<uint16_t> {5}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (3), (royale::Vector<uint16_t> {6, 7, 8, 9}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (4), (royale::Vector<uint16_t> {10}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (5), (royale::Vector<uint16_t> {11, 12, 13, 14}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (6), (royale::Vector<uint16_t> {15}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (7), (royale::Vector<uint16_t> {16, 17, 18, 19}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (8), (royale::Vector<uint16_t> {20}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (9), (royale::Vector<uint16_t> {21, 22, 23, 24}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (10), (royale::Vector<uint16_t> {25}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (11), (royale::Vector<uint16_t> {26, 27, 28, 29}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (12), (royale::Vector<uint16_t> {30, 31, 32, 33}));
    }
    {
        UseCaseMixedXHt ucd (5u, 5u, 30000000, 20200000, 20600000, { 200u, 1000u }, { 200u, 1000u },
                             1000u, 1000u, 1000u, 1000u, 1000u, ExposureGray::Off, ExposureGray::Off, IntensityPhaseOrder::IntensityLastPhase);
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (0), (royale::Vector<uint16_t> {0, 1, 2, 3}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (1), (royale::Vector<uint16_t> {4}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (2), (royale::Vector<uint16_t> {5, 6, 7, 8}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (3), (royale::Vector<uint16_t> {9}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (4), (royale::Vector<uint16_t> {10, 11, 12, 13}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (5), (royale::Vector<uint16_t> {14}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (6), (royale::Vector<uint16_t> {15, 16, 17, 18}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (7), (royale::Vector<uint16_t> {19}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (8), (royale::Vector<uint16_t> {20, 21, 22, 23}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (9), (royale::Vector<uint16_t> {24}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (10), (royale::Vector<uint16_t> {25, 26, 27, 28}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (11), (royale::Vector<uint16_t> {29, 30, 31, 32}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (12), (royale::Vector<uint16_t> {33}));
    }
    {
        UseCaseMixedIrregularXHt ucd (5u, 5u, 30000000, 20200000, 20600000, { 200u, 1000u }, { 200u, 1000u },
                                      1000u, 1000u, 1000u, 1000u, 1000u, ExposureGray::Off, ExposureGray::Off, IntensityPhaseOrder::IntensityLastPhase);
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (0), (royale::Vector<uint16_t> {0, 1, 2, 3}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (1), (royale::Vector<uint16_t> {4}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (2), (royale::Vector<uint16_t> {5, 6, 7, 8}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (3), (royale::Vector<uint16_t> {9}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (4), (royale::Vector<uint16_t> {10, 11, 12, 13}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (5), (royale::Vector<uint16_t> {14}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (6), (royale::Vector<uint16_t> {15, 16, 17, 18}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (7), (royale::Vector<uint16_t> {19}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (8), (royale::Vector<uint16_t> {20, 21, 22, 23}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (9), (royale::Vector<uint16_t> {24}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (10), (royale::Vector<uint16_t> {25, 26, 27, 28}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (11), (royale::Vector<uint16_t> {29, 30, 31, 32}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (12), (royale::Vector<uint16_t> {33}));
    }
    {
        UseCaseInterleavedXHt ucd (5u, 5u, 30000000, 20200000, 20600000, { 200u, 1000u }, { 200u, 1000u },
                                   1000u, 1000u, 1000u, 1000u, 1000u, ExposureGray::Off, ExposureGray::Off, IntensityPhaseOrder::IntensityFirstPhase);
        // HT1
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (0), (royale::Vector<uint16_t> {0}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (1), (royale::Vector<uint16_t> {1, 2, 3, 4}));
        // ES1
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (2), (royale::Vector<uint16_t> {5}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (3), (royale::Vector<uint16_t> {6, 7, 8, 9}));
        // HT2
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (4), (royale::Vector<uint16_t> {10}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (5), (royale::Vector<uint16_t> {11, 12, 13, 14}));
        // ES2
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (6), (royale::Vector<uint16_t> {15, 16, 17, 18}));
        // HT3
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (7), (royale::Vector<uint16_t> {19}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (8), (royale::Vector<uint16_t> {20, 21, 22, 23}));
        // HT4
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (9), (royale::Vector<uint16_t> {24}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (10), (royale::Vector<uint16_t> {25, 26, 27, 28}));
        // HT5
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (11), (royale::Vector<uint16_t> {29}));
        ASSERT_EQ (ucd.getSequenceIndicesForRawFrameSet (12), (royale::Vector<uint16_t> {30, 31, 32, 33}));
    }
}

TEST (TestUseCaseDefinition, dynamicNames)
{
    {
        UseCaseMixedXHt ucd (5u, 5u, 30000000, 20200000, 20600000, { 200u, 1000u }, { 200u, 1000u },
                             1000u, 1000u, 1000u, 1000u, 1000u, ExposureGray::Off, ExposureGray::Off, IntensityPhaseOrder::IntensityLastPhase);
        ASSERT_EQ (royale::String ("MixedXHT_5"), ucd.getTypeName());
    }
    {
        UseCaseMixedXHt ucd (5u, 10u, 30000000, 20200000, 20600000, { 200u, 1000u }, { 200u, 1000u },
                             1000u, 1000u, 1000u, 1000u, 1000u, ExposureGray::Off, ExposureGray::Off, IntensityPhaseOrder::IntensityLastPhase);
        ASSERT_EQ (royale::String ("MixedXHT_10"), ucd.getTypeName());
    }
    {
        UseCaseMixedIrregularXHt ucd (5u, 5u, 30000000, 20200000, 20600000, { 200u, 1000u }, { 200u, 1000u },
                                      1000u, 1000u, 1000u, 1000u, 1000u, ExposureGray::Off, ExposureGray::Off, IntensityPhaseOrder::IntensityLastPhase);
        ASSERT_EQ (royale::String ("MixedIrregularXHT_5_1"), ucd.getTypeName());
    }
    {
        UseCaseMixedIrregularXHt ucd (15u, 3u, 30000000, 20200000, 20600000, { 200u, 1000u }, { 200u, 1000u },
                                      1000u, 1000u, 1000u, 1000u, 1000u, ExposureGray::Off, ExposureGray::Off, IntensityPhaseOrder::IntensityLastPhase);
        ASSERT_EQ (royale::String ("MixedIrregularXHT_15_5"), ucd.getTypeName());
    }
    {
        UseCaseInterleavedXHt ucd (5u, 5u, 30000000, 20200000, 20600000, { 200u, 1000u }, { 200u, 1000u },
                                   1000u, 1000u, 1000u, 1000u, 1000u, ExposureGray::Off, ExposureGray::Off, IntensityPhaseOrder::IntensityLastPhase);
        ASSERT_EQ (royale::String ("InterleavedXHT_5"), ucd.getTypeName());
    }
    {
        UseCaseInterleavedXHt ucd (5u, 10u, 30000000, 20200000, 20600000, { 200u, 1000u }, { 200u, 1000u },
                                   1000u, 1000u, 1000u, 1000u, 1000u, ExposureGray::Off, ExposureGray::Off, IntensityPhaseOrder::IntensityLastPhase);
        ASSERT_EQ (royale::String ("InterleavedXHT_10"), ucd.getTypeName());
    }
}

/**
 * Check that the stream number allocation works.
 */
TEST (TestUseCaseDefinition, allocatedStreamIds)
{
    const UseCaseMixedXHt ucd
    {
        5u, 5u, 30000000, 30000000, 20200000, { 200u, 1000u }, { 200u, 1000u },
        1000u, 1000u, 1000u, 1000u, 1000u, ExposureGray::Off, ExposureGray::Off, IntensityPhaseOrder::IntensityFirstPhase
    };

    const auto streamIds = ucd.getStreamIds();
    ASSERT_EQ (streamIds.size(), 2u);
    ASSERT_NE (streamIds[0], streamIds[1]);
    ASSERT_EQ (DEFAULT_STREAM_ID, streamIds[0]);
    ASSERT_EQ (static_cast<StreamId> (DEFAULT_STREAM_ID + 1), streamIds[1]);
}

/**
 * As an implementer of the FrameCollector, I want to navigate from the UCD to the set of frame
 * groups and know which frames are in each frame group.
 */
TEST (TestUseCaseDefinition, getStreamCallbackFrames)
{
    // Using a ratio of 2, the framesets are in the order:
    // HT1gray HT1mod ES1gray ES1mod HT2gray HT2mod ES2mod
    const UseCaseInterleavedXHt ucd
    {
        5u, 2u, 30000000, 30000000, 20200000, { 200u, 1000u }, { 200u, 1000u },
        1000u, 1000u, 1000u, 1000u, 1000u, ExposureGray::Off, ExposureGray::Off, IntensityPhaseOrder::IntensityFirstPhase
    };

    const royale::Vector<RawFrameSetId> expectedEsUcFrames {5, 6, 7, 8, 9, 15, 16, 17, 18};
    const royale::Vector<RawFrameSetId> expectedHt1UcFrames {0, 1, 2, 3, 4};
    const royale::Vector<RawFrameSetId> expectedHt2UcFrames {10, 11, 12, 13, 14};

    const auto streamIds = ucd.getStreamIds();
    ASSERT_EQ (streamIds.size(), 2u);
    StreamId esId, htId;
    if (ucd.getRawFrameSetIndices (streamIds[0], 0).size() == 3u)
    {
        esId = streamIds[0];
        htId = streamIds[1];
    }
    else
    {
        esId = streamIds[1];
        htId = streamIds[0];
    }

    ASSERT_EQ (1u, ucd.getFrameGroupCount (esId));
    ASSERT_EQ (2u, ucd.getFrameGroupCount (htId));

    const auto esFrameSets = ucd.getRawFrameSetIndices (esId, 0);
    const auto ht1FrameSets = ucd.getRawFrameSetIndices (htId, 0);
    const auto ht2FrameSets = ucd.getRawFrameSetIndices (htId, 1);

    ASSERT_EQ ( (royale::Vector<std::size_t> {2, 3, 6}), esFrameSets);
    ASSERT_EQ ( (royale::Vector<std::size_t> {0, 1}), ht1FrameSets);
    ASSERT_EQ ( (royale::Vector<std::size_t> {4, 5}), ht2FrameSets);

    royale::Vector<RawFrameSetId> frames;
    for (auto setId : esFrameSets)
    {
        for (auto frameId : ucd.getSequenceIndicesForRawFrameSet (setId))
        {
            frames.push_back (frameId);
        }
    }
    ASSERT_EQ (expectedEsUcFrames, frames);
    frames.clear();
    for (auto setId : ht1FrameSets)
    {
        for (auto frameId : ucd.getSequenceIndicesForRawFrameSet (setId))
        {
            frames.push_back (frameId);
        }
    }
    ASSERT_EQ (expectedHt1UcFrames, frames);
    frames.clear();
    for (auto setId : ht2FrameSets)
    {
        for (auto frameId : ucd.getSequenceIndicesForRawFrameSet (setId))
        {
            frames.push_back (frameId);
        }
    }
    ASSERT_EQ (expectedHt2UcFrames, frames);
}

/**
 * As an implementor of the Processing, I want to know which exposure times, as returned by
 * CapturedUseCase, correspond to which RawFrameSets in a stream.
 *
 * For example, in an HT+ES use case, when I ask about the HT stream, I want to get the information
 * that I would get for an HT non-mixed UseCase, no matter how many HT FrameGroups are in the mixed
 * stream.
 */
TEST (TestUseCaseDefinition, getCapturedExposuresByStream)
{
    const UseCaseMixedXHt ucd
    {
        5u, 5u, 30000000, 20200000, 30000000, { 200u, 1000u }, { 200u, 1000u },
        501u, 502u, 503u, 504u, 505u, ExposureGray::Off, ExposureGray::Off, IntensityPhaseOrder::IntensityFirstPhase
    };
    // Mock for a CapturedUseCase that has older exposures, which are 100 less than the current values
    std::vector<uint16_t> capturedUseCaseExposures;
    for (const auto exposure : ucd.getExposureTimes())
    {
        capturedUseCaseExposures.push_back (static_cast<uint16_t> (exposure - 100));
    }

    // These expectations are in the order that the received frames are in the captureCallback,
    // which is different to the order of UCD.getExposureTimes() and CUC.getExposureTimes().
    const royale::Vector<uint16_t> expectedEsExposures {405u, 402u, 403u}; // gray, mod1, mod2
    const royale::Vector<uint16_t> expectedHtExposures {404u, 401u}; // gray, ht

    // Match expectations to streams
    const auto streamIds = ucd.getStreamIds();
    ASSERT_EQ (streamIds.size(), 2u);
    royale::Vector<royale::Vector<uint16_t>> expected;
    if (ucd.getRawFrameSetIndices (streamIds[0], 0).size() == 3u)
    {
        expected.push_back (expectedEsExposures);
        expected.push_back (expectedHtExposures);
    }
    else
    {
        expected.push_back (expectedHtExposures);
        expected.push_back (expectedEsExposures);
    }

    // Sanity check for the test setup
    for (std::size_t i = 0; i < streamIds.size(); i++)
    {
        const auto streamId = streamIds.at (i);
        ASSERT_GE (ucd.getFrameGroupCount (streamId), 1u);
    }

    // Method 1: without a convenience function
    for (std::size_t i = 0; i < streamIds.size(); i++)
    {
        const auto streamId = streamIds.at (i);
        royale::Vector<uint16_t> exposures;
        for (const auto setIdx : ucd.getRawFrameSetIndices (streamId, 0))
        {
            const auto groupIdx = ucd.getRawFrameSets().at (setIdx).exposureGroupIdx;
            exposures.push_back (capturedUseCaseExposures.at (groupIdx));
        }
        ASSERT_EQ (expected[i], exposures);
    }

    // Method 2: with a convenience function
    for (std::size_t i = 0; i < streamIds.size(); i++)
    {
        const auto streamId = streamIds.at (i);
        royale::Vector<uint16_t> exposures;
        for (const auto groupIdx : ucd.getExposureIndicesForStream (streamId))
        {
            exposures.push_back (capturedUseCaseExposures.at (groupIdx));
        }
        ASSERT_EQ (expected[i], exposures);
    }
}

TEST (TestUseCaseDefinition, setDutyCycles)
{
    UseCaseEightPhase ucd (5u, 30000000, 20200000, { 200u, 1000u }, 1000u, 1000u, 1000u);

    testDutyCycle (ucd, 0.0, RawFrameSet::DutyCycle::DC_0);
    testDutyCycle (ucd, 25.0, RawFrameSet::DutyCycle::DC_25);
    testDutyCycle (ucd, 37.5, RawFrameSet::DutyCycle::DC_37_5);
    testDutyCycle (ucd, 50.0, RawFrameSet::DutyCycle::DC_50);
    testDutyCycle (ucd, 75.0, RawFrameSet::DutyCycle::DC_75);
    testDutyCycle (ucd, 100.0, RawFrameSet::DutyCycle::DC_100);

    ASSERT_THROW (testDutyCycle (ucd, 24.999, RawFrameSet::DutyCycle::DC_25), royale::common::OutOfBounds);
}

TEST (TestUseCaseDefinition, rawFrameSetOperatorEqual)
{
    using PhaseDefinition = royale::usecase::RawFrameSet::PhaseDefinition;
    using DutyCycle = royale::usecase::RawFrameSet::DutyCycle;
    using Alignment = royale::usecase::RawFrameSet::Alignment;

    const RawFrameSet rfs1 { 30000000, PhaseDefinition::MODULATED_4PH_CW, DutyCycle::DC_25, 0, Alignment::CLOCK_ALIGNED, 0.};
    const RawFrameSet rfs2 { 20200000, PhaseDefinition::MODULATED_4PH_CW, DutyCycle::DC_25, 0, Alignment::CLOCK_ALIGNED, 0.};
    const RawFrameSet rfs3 { 30000000, PhaseDefinition::GRAYSCALE, DutyCycle::DC_25, 0, Alignment::CLOCK_ALIGNED, 0.};
    const RawFrameSet rfs4 { 30000000, PhaseDefinition::MODULATED_4PH_CW, DutyCycle::DC_50, 0, Alignment::CLOCK_ALIGNED, 0.};
    const RawFrameSet rfs5 { 30000000, PhaseDefinition::MODULATED_4PH_CW, DutyCycle::DC_25, 1, Alignment::CLOCK_ALIGNED, 0.};
    const RawFrameSet rfs6 { 30000000, PhaseDefinition::MODULATED_4PH_CW, DutyCycle::DC_25, 0, Alignment::START_ALIGNED, 0.};
    const RawFrameSet rfs7 { 30000000, PhaseDefinition::MODULATED_4PH_CW, DutyCycle::DC_25, 0, Alignment::CLOCK_ALIGNED, 1.};

    ASSERT_NE (rfs1, rfs2);
    ASSERT_NE (rfs1, rfs3);
    ASSERT_NE (rfs1, rfs4);
    ASSERT_NE (rfs1, rfs5);
    ASSERT_NE (rfs1, rfs6);
    ASSERT_NE (rfs1, rfs7);

    const RawFrameSet rfsSSC1{ 30000000, PhaseDefinition::MODULATED_4PH_CW, DutyCycle::DC_25, 0, Alignment::CLOCK_ALIGNED, 0., 10000.0, 0.5, 0.166 };
    const RawFrameSet rfsSSC2{ 30000000, PhaseDefinition::MODULATED_4PH_CW, DutyCycle::DC_25, 0, Alignment::CLOCK_ALIGNED, 0., 10000.0, 0.5, 0.125 };

    ASSERT_NE (rfsSSC1, rfsSSC2);

    const RawFrameSet identicalToRfs1 { 30000000, PhaseDefinition::MODULATED_4PH_CW, DutyCycle::DC_25, 0, Alignment::CLOCK_ALIGNED, 0.};
    const RawFrameSet copyOfRfs1 {rfs1};

    ASSERT_EQ (rfs1, identicalToRfs1);
    ASSERT_EQ (rfs1, copyOfRfs1);
}
