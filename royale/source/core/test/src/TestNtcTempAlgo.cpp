/****************************************************************************\
* Copyright (C) 2016 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#include <gtest/gtest.h>
#include "common/NtcTempAlgo.hpp"
#include <algorithm>
#include <limits>
#include <cmath>
#include <vector>
#include "common/exceptions/InvalidValue.hpp"

using royale::common::NtcTempAlgo;
using royale::common::InvalidValue;

class ANtcTempAlgo: public ::testing::Test
{
public:
    const float resistorR1    =  10000;
    const float resistorRntc0 = 100000;
    const float resitorRatio = 0.1f;
    const float referenceTemperature = 25.0f;
    const float thermistorBeta = 4200.0f;
    const NtcTempAlgo algo = NtcTempAlgo (resistorR1, resistorRntc0, referenceTemperature, thermistorBeta);
    uint16_t offset = 0;
};

TEST_F (ANtcTempAlgo, calculatesDifferentialTemperatures)
{
    uint16_t vRef1 = 3920;
    uint16_t vRef2 = 2925;
    const unsigned int ntcsSize = 4;
    uint16_t vNtc1s[ntcsSize] = { 3729,   3271,   2284,    544 };
    uint16_t vNtc2s[ntcsSize] = { 2761,   2367,   1517,     20 };
    float expTemps[ntcsSize]  = { 0.0f,  25.0f,  50.0f,  80.0f };

    for (unsigned int i = 0; i < ntcsSize; i++)
    {
        EXPECT_NEAR (expTemps[i], algo.calcTemperature (vRef1, vNtc1s[i], vRef2, vNtc2s[i], offset), 0.5)
                << "vDiffRatio = " << static_cast<float> (vRef1 - vRef2) / static_cast<float> (vNtc1s[i] - vNtc2s[i])
                << "\nvRef1 = " << vRef1
                << "\nvRef2 = " << vRef2
                << "\nvNtc1 = " << vNtc1s[i]
                << "\nvNtc2 = " << vNtc2s[i];
    }
}

TEST_F (ANtcTempAlgo, calculatesTemperatures)
{
    uint16_t vRef1 = 3920;
    uint16_t vRef2 = 2925;
    const unsigned int ntcsSize = 4;
    uint16_t vNtc1s[ntcsSize] = { 3729,   3271,   2284,    544 };
    uint16_t vNtc2s[ntcsSize] = { 2761,   2367,   1517,     20 };
    float expTemps[ntcsSize]  = { 0.0f,  25.0f,  50.0f,  80.0f };
    float refVoltage = static_cast<float> (vRef1 - vRef2);

    for (unsigned int i = 0; i < ntcsSize; i++)
    {
        const auto ntcVoltage = static_cast<float> (vNtc1s[i] - vNtc2s[i]);
        EXPECT_NEAR (expTemps[i], algo.calcTemperature (refVoltage, ntcVoltage), 0.5)
                << "refVoltage = " << static_cast<float> (vRef1 - vRef2)
                << "\nntcVoltage = " << static_cast<float> (vNtc1s[i] - vNtc2s[i]);
    }
}

TEST_F (ANtcTempAlgo, throwsExceptionForInvalidValues)
{
    {
        uint16_t vRef1 = 10u;
        uint16_t vRef2 = 5u;
        uint16_t vNtc1 = 9u;
        uint16_t vNtc2 = 3u;

        EXPECT_THROW (algo.calcTemperature (vRef1, vNtc1, vRef2, vNtc2, offset), InvalidValue)
                << " vRef1 = " << vRef1
                << " vRef2 = " << vRef2
                << " vNtc1 = " << vNtc1
                << " vNtc2 = " << vNtc2;
    }

    {
        uint16_t vRef1 = 10u;
        uint16_t vRef2 = 5u;
        uint16_t vNtc1 = 10u;
        uint16_t vNtc2 = 10u;

        EXPECT_THROW (algo.calcTemperature (vRef1, vNtc1, vRef2, vNtc2, offset), InvalidValue)
                << " vRef1 = " << vRef1
                << " vRef2 = " << vRef2
                << " vNtc1 = " << vNtc1
                << " vNtc2 = " << vNtc2;
    }

    {
        uint16_t vRef1 = 5u;
        uint16_t vRef2 = 10u;
        uint16_t vNtc1 = 10u;
        uint16_t vNtc2 = 10u;

        EXPECT_THROW (algo.calcTemperature (vRef1, vNtc1, vRef2, vNtc2, offset), InvalidValue)
                << " vRef1 = " << vRef1
                << " vRef2 = " << vRef2
                << " vNtc1 = " << vNtc1
                << " vNtc2 = " << vNtc2;
    }

    {
        uint16_t vRef1 = 10u;
        uint16_t vRef2 = 0u;
        uint16_t vNtc1 = 10u;
        uint16_t vNtc2 = 9u;

        EXPECT_NO_THROW (algo.calcTemperature (vRef1, vNtc1, vRef2, vNtc2, offset))
                << " vRef1 = " << vRef1
                << " vRef2 = " << vRef2
                << " vNtc1 = " << vNtc1
                << " vNtc2 = " << vNtc2;
    }
}

TEST_F (ANtcTempAlgo, checkA1013Settings)
{
    float a1013R1 = 100000.0f;
    float a1013Rntc0 = 100000.0f;
    float a1013Beta = 4250.0f;
    float refVoltage = 3.3f;
    float ntcVoltage = 1.65f;
    float expTemperature = 25.0f;
    auto ntcAlgo = royale::common::NtcTempAlgo (a1013R1, a1013Rntc0, referenceTemperature, a1013Beta);
    ASSERT_FLOAT_EQ (ntcAlgo.calcTemperature (refVoltage, ntcVoltage), expTemperature);
}

