#include <royale/ProcessingFlag.hpp>
#include <gtest/gtest.h>

using namespace royale;

TEST (TestProcessingFlag, testGetFlagName)
{
    String testString = getProcessingFlagName (ProcessingFlag::MPIAmpThreshold_Float);

    EXPECT_EQ (testString, "MPIAmpThreshold_Float");

    testString = getProcessingFlagName (ProcessingFlag::NUM_FLAGS);
    EXPECT_EQ (testString, "");
}

TEST (TestProcessingFlag, testMapOperator)
{
    ProcessingParameterMap map1 (
    {
        { ProcessingFlag::ConsistencyTolerance_Float, Variant (1.2f, 0.2f, 1.5f) },
        { ProcessingFlag::AdaptiveNoiseFilterType_Int, Variant (1, 1, 2) }
    });

    ProcessingParameterMap map2 (
    {
        { ProcessingFlag::FlyingPixelsF0_Float, Variant (0.018f, 0.01f, 0.04f) },
        { ProcessingFlag::FlyingPixelsF1_Float, Variant (0.14f, 0.01f, 0.5f) }

    });

    ProcessingParameterMap map3 (
    {
        { ProcessingFlag::ConsistencyTolerance_Float, Variant (2.2f, 0.2f, 2.5f) },
        { ProcessingFlag::FlyingPixelsF0_Float, Variant (0.018f, 0.01f, 0.04f) }
    });

    ProcessingParameterMap map1plus2Expected (
    {
        { ProcessingFlag::ConsistencyTolerance_Float, Variant (1.2f, 0.2f, 1.5f) },
        { ProcessingFlag::AdaptiveNoiseFilterType_Int, Variant (1, 1, 2) },
        { ProcessingFlag::FlyingPixelsF0_Float, Variant (0.018f, 0.01f, 0.04f) },
        { ProcessingFlag::FlyingPixelsF1_Float, Variant (0.14f, 0.01f, 0.5f) }
    });

    ProcessingParameterMap map1plus3Expected (
    {
        { ProcessingFlag::ConsistencyTolerance_Float, Variant (2.2f, 0.2f, 2.5f) },
        { ProcessingFlag::AdaptiveNoiseFilterType_Int, Variant (1, 1, 2) },
        { ProcessingFlag::FlyingPixelsF0_Float, Variant (0.018f, 0.01f, 0.04f) }
    });

    ProcessingParameterMap map1plus2 = combineProcessingMaps (map1, map2);
    ProcessingParameterMap map1plus3 = combineProcessingMaps (map1, map3);

    EXPECT_EQ (map1plus2.size(), 4u);
    EXPECT_EQ (map1plus3.size(), 3u);

    for (auto param : map1plus2Expected)
    {
        EXPECT_EQ (param.second, map1plus2[param.first]);
    }

    for (auto param : map1plus3Expected)
    {
        EXPECT_EQ (param.second, map1plus3[param.first]);
    }
}
