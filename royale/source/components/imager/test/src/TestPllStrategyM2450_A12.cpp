/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <gtest/gtest.h>
#include <cmath>
#include <common/exceptions/LogicError.hpp>
#include <imager/ModPllStrategyM2450_A12.hpp>
#include <imager/DphyPllStrategyM2450_A12.hpp>

//use the define to enable fast testing
#undef ENABLE_FASTPLLTESTING

/*
* Tests require external test data files, by default they are not used and the test
* is reduced to one vector entry only due to timing contraints for unit tests (compilation and runtime).
* Use the preprocessor flag to enable full testing.
*/
class TestPllStrategyM2450_A12 : public ::testing::Test
{
protected:
    TestPllStrategyM2450_A12()
    {

    }

    virtual ~TestPllStrategyM2450_A12()
    {

    }

    virtual void SetUp()
    {

    }

    std::vector<std::vector<double>> loadTestVectors (const std::string &fileName)
    {
        std::vector<std::vector<double>> testVectors;

#ifdef ENABLE_FASTPLLTESTING
        return testVectors;
#endif

        if (fileName.length() &&
                std::ifstream (fileName).good())
        {
            std::ifstream stream (fileName);
            std::string line;

            while (std::getline (stream, line))
            {
                std::vector<double> testVector;

                std::istringstream s (line);
                std::string field;

                if (line.length() == 0 || line.find ("//") < line.length())
                {
                    continue;
                }

                while (getline (s, field, ','))
                {
                    testVector.push_back (std::stod (field));
                }

                testVectors.push_back (testVector);
            }
        }
        else
        {
            LOG (INFO) << "Warning: Fast test has been enabled because no test data file found.";
        }

        return testVectors;
    }

    virtual void TearDown()
    {
        m_dphyPllCalc.reset();
        m_modPllCalc.reset();
    }

protected:
    std::unique_ptr <royale::imager::IPllStrategy> m_modPllCalc;
    std::unique_ptr <royale::imager::IPllStrategy> m_dphyPllCalc;
};

TEST_F (TestPllStrategyM2450_A12, ModPllCalcNegative)
{
    m_modPllCalc.reset (new royale::imager::ModPllStrategyM2450_A12 (26000000));
    ASSERT_NE (m_modPllCalc, nullptr) << "pll calc instance is null.";

    std::vector<uint16_t> pllcfg (8);
    ASSERT_FALSE (m_modPllCalc->pllSettings (12600000 - 1, false, pllcfg));
    ASSERT_FALSE (m_modPllCalc->pllSettings (400000000 + 1, false, pllcfg));
    ASSERT_THROW (m_modPllCalc->pllSettings (12600000, true, pllcfg, 0.0), royale::common::LogicError);

    // test vector contains fssc > 1e5, which has to result in a return value of false
    ASSERT_FALSE (m_modPllCalc->pllSettings (12600000, true, pllcfg, 100001, 0.5000, 0.0010));

    // no ssc possible frequency to high/low, which has to result in a return value of false
    ASSERT_FALSE (m_modPllCalc->pllSettings (13340000, true, pllcfg, 10000, 0.5000, 0.0125));

    // no ssc possible at this frequency with the given parameters
    ASSERT_FALSE (m_modPllCalc->pllSettings (17430000, true, pllcfg, 10000, 0.5, 0.0125));
}

TEST_F (TestPllStrategyM2450_A12, ModPllCalc_26MHz)
{
    /*
    * Perform a calculation of the modpll registers.
    * Results are compared with the original
    * results from the MATLAB single source.
    */
    const uint32_t fsys = 26000000;
    m_modPllCalc.reset (new royale::imager::ModPllStrategyM2450_A12 (fsys));
    ASSERT_NE (m_modPllCalc, nullptr) << "pll calc instance is null.";

    std::vector<uint16_t> pllcfg (8);

    uint16_t maxDiff = 0;

    std::vector<std::vector<double>> testVectors = loadTestVectors ("modpll_m2450_A12_26MHz_checker.csv");

    if (!testVectors.size())
    {
        testVectors.push_back ({ 26000000, 12600000, 0, 20000, 0.0000, 0.0025, 18485, 64528, 960, 21801, 1452, 59836, 17411, 1, 1, 1, 1, 0 });
        testVectors.push_back ({ 26000000, 13000000, 1, 10000, 0.5000, 0.0125, 34869, 0, 1536, 21801, 13107, 5619, 35082, 2, 1, 0, 0, 0 });
    };

    auto vectorCount = 0u;

    for (auto testVector : testVectors)
    {
        auto frequency = testVector.at (1);

        //test only for the current config's system frequency, ignore other test vectors
        if (testVector.at (0) == static_cast<double> (fsys))
        {
            vectorCount++;

            bool validFreq = m_modPllCalc->pllSettings (frequency, testVector.at (2) == 0. ? false : true, pllcfg, testVector.at (3), testVector.at (4), testVector.at (5));

            LOG (INFO) << "PLLCFG1: 0x" << std::hex << pllcfg.at (0) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (6)) <<

                       "   PLLCFG2: 0x" << std::hex << pllcfg.at (1) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (7)) <<

                       "   PLLCFG3: 0x" << std::hex << pllcfg.at (2) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (8)) <<

                       "   PLLCFG4: 0x" << std::hex << pllcfg.at (3) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (9)) <<

                       "   PLLCFG5: 0x" << std::hex << pllcfg.at (4) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (10)) <<

                       "   PLLCFG6: 0x" << std::hex << pllcfg.at (5) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (11)) <<

                       "   PLLCFG7: 0x" << std::hex << pllcfg.at (6) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (12)) <<

                       "   PLLCFG8: 0x" << std::hex << pllcfg.at (7) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (13));

            uint16_t diff = static_cast<uint16_t> (::fabs (testVector.at (7) - pllcfg.at (1)));
            if (diff > maxDiff)
            {
                maxDiff = diff;
            }

            //if the testVector marks a frequency as invalid, the modpll calc also must detect this
            if ( (testVector.at (14) + testVector.at (15) > 0) && validFreq == true)
            {
                LOG (ERROR) << "Forbidden frequency check compliance failed at frequency: " << frequency;
                ASSERT_TRUE (false);
            }

            //compare the three PLL register settings
            ASSERT_EQ (pllcfg.at (0), testVector.at (6)) << "@" << frequency << "Hz";
            ASSERT_NEAR (pllcfg.at (1), testVector.at (7), 2) << "@" << frequency << "Hz"; //allow 1d LSB deviance
            ASSERT_EQ (pllcfg.at (2), testVector.at (8)) << "@" << frequency << "Hz";
            ASSERT_EQ (pllcfg.at (3), testVector.at (9)) << "@" << frequency << "Hz";
            if (testVector.at (2) > 0)
            {
                ASSERT_EQ (pllcfg.at (4), testVector.at (10)) << "@" << frequency << "Hz";
                ASSERT_EQ (pllcfg.at (5), testVector.at (11)) << "@" << frequency << "Hz";
                ASSERT_EQ (pllcfg.at (6), testVector.at (12)) << "@" << frequency << "Hz";
                ASSERT_EQ (pllcfg.at (7), testVector.at (13)) << "@" << frequency << "Hz";
            }

            // backward calculation from register data to frequency and SSC

            royale::imager::IPllStrategy::ReversePllSettings settings;
            const std::vector<uint16_t> registers = { static_cast<uint16_t> (testVector.at (6)), static_cast<uint16_t> (testVector.at (7)),
                                                      static_cast<uint16_t> (testVector.at (8)), static_cast<uint16_t> (testVector.at (9)),
                                                      static_cast<uint16_t> (testVector.at (10)), static_cast<uint16_t> (testVector.at (11)),
                                                      static_cast<uint16_t> (testVector.at (12)), static_cast<uint16_t> (testVector.at (13))
                                                    };
            m_modPllCalc->reversePllSettings (registers, settings);

            if (validFreq && testVector.at (2) > 0)
            {
                ASSERT_NEAR (settings.outFreq, frequency, 2) << "@" << frequency << "Hz";
                ASSERT_NEAR (settings.sscFreq, testVector.at (3), 150) << "@" << frequency << "Hz";
                ASSERT_NEAR (settings.sscKspread, testVector.at (4), 0.01) << "@" << frequency << "Hz";
                ASSERT_NEAR (settings.sscDelta, testVector.at (5), 0.001) << "@" << frequency << "Hz";
            }
        }
    }

    // Check that we tested any vectors at all
    EXPECT_GT (vectorCount, 0u);

    LOG (INFO) << "Max diff: " << maxDiff;
}

TEST_F (TestPllStrategyM2450_A12, ModPllCalc_24MHz)
{
    /*
    * Perform a calculation of the modpll registers.
    * Results are compared with the original
    * results from the MATLAB single source.
    */
    const uint32_t fsys = 24000000;
    m_modPllCalc.reset (new royale::imager::ModPllStrategyM2450_A12 (fsys));
    ASSERT_NE (m_modPllCalc, nullptr) << "pll calc instance is null.";

    std::vector<uint16_t> pllcfg (8);

    uint16_t maxDiff = 0;

    std::vector<std::vector<double>> testVectors = loadTestVectors ("modpll_m2450_A12_24MHz_checker.csv");

    if (!testVectors.size())
    {
        testVectors.push_back ({ 24000000, 12600000, 0, 20000, 0.0000, 0.0025, 26681, 26214, 998, 21801, 1573, 38625, 11012, 1, 0, 1, 1, 0 });
        testVectors.push_back ({ 24000000, 12600000, 1, 10000, 0.5000, 0.0125, 26681, 26214, 1766, 21801, 62915, 30936, 22283, 2, 0, 1, 1, 0 });
    };

    auto vectorCount = 0u;

    for (auto testVector : testVectors)
    {
        auto frequency = testVector.at (1);
        //test only for the current config's system frequency, ignore other test vectors
        if (testVector.at (0) == static_cast<double> (fsys))
        {
            vectorCount++;

            bool validFreq = m_modPllCalc->pllSettings (frequency, testVector.at (2) == 0. ? false : true, pllcfg, testVector.at (3), testVector.at (4), testVector.at (5));

            LOG (INFO) << "PLLCFG1: 0x" << std::hex << pllcfg.at (0) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (6)) <<

                       "   PLLCFG2: 0x" << std::hex << pllcfg.at (1) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (7)) <<

                       "   PLLCFG3: 0x" << std::hex << pllcfg.at (2) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (8)) <<

                       "   PLLCFG4: 0x" << std::hex << pllcfg.at (3) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (9)) <<

                       "   PLLCFG5: 0x" << std::hex << pllcfg.at (4) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (10)) <<

                       "   PLLCFG6: 0x" << std::hex << pllcfg.at (5) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (11)) <<

                       "   PLLCFG7: 0x" << std::hex << pllcfg.at (6) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (12)) <<

                       "   PLLCFG8: 0x" << std::hex << pllcfg.at (7) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (13));


            uint16_t diff = static_cast<uint16_t> (::fabs (testVector.at (7) - pllcfg.at (1)));
            if (diff > maxDiff)
            {
                maxDiff = diff;
            }

            //if the testVector marks a frequency as invalid, the modpll calc also must detect this
            if ( (testVector.at (14) + testVector.at (15) > 0) && validFreq == true)
            {
                LOG (ERROR) << "Forbidden frequency check compliance failed at frequency: " << frequency;
                ASSERT_TRUE (false);
            }

            //compare the three PLL register settings
            ASSERT_EQ (pllcfg.at (0), testVector.at (6)) << "@" << frequency << "Hz";
            ASSERT_NEAR (pllcfg.at (1), testVector.at (7), 2) << "@" << frequency << "Hz"; //allow 1d LSB deviance
            ASSERT_EQ (pllcfg.at (2), testVector.at (8)) << "@" << frequency << "Hz";
            ASSERT_EQ (pllcfg.at (3), testVector.at (9)) << "@" << frequency << "Hz";
            if (testVector.at (2) > 0)
            {
                ASSERT_EQ (pllcfg.at (4), testVector.at (10)) << "@" << frequency << "Hz";
                ASSERT_EQ (pllcfg.at (5), testVector.at (11)) << "@" << frequency << "Hz";
                ASSERT_EQ (pllcfg.at (6), testVector.at (12)) << "@" << frequency << "Hz";
                ASSERT_EQ (pllcfg.at (7), testVector.at (13)) << "@" << frequency << "Hz";
            }

            // backward calculation from register data to frequency and SSC

            royale::imager::IPllStrategy::ReversePllSettings settings;
            const std::vector<uint16_t> registers = { static_cast<uint16_t> (testVector.at (6)), static_cast<uint16_t> (testVector.at (7)),
                                                      static_cast<uint16_t> (testVector.at (8)), static_cast<uint16_t> (testVector.at (9)),
                                                      static_cast<uint16_t> (testVector.at (10)), static_cast<uint16_t> (testVector.at (11)),
                                                      static_cast<uint16_t> (testVector.at (12)), static_cast<uint16_t> (testVector.at (13))
                                                    };
            m_modPllCalc->reversePllSettings (registers, settings);

            if (validFreq && testVector.at (2) > 0)
            {
                ASSERT_NEAR (settings.outFreq, frequency, 2) << "@" << frequency << "Hz";
                ASSERT_NEAR (settings.sscFreq, testVector.at (3), 150) << "@" << frequency << "Hz";
                ASSERT_NEAR (settings.sscKspread, testVector.at (4), 0.01) << "@" << frequency << "Hz";
                ASSERT_NEAR (settings.sscDelta, testVector.at (5), 0.001) << "@" << frequency << "Hz";
            }
        }
    }

    // Check that we tested any vectors at all
    EXPECT_GT (vectorCount, 0u);

    LOG (INFO) << "Max diff: " << maxDiff;
}

TEST_F (TestPllStrategyM2450_A12, ModPllCalc_19MHz2)
{
    /*
    * Perform a calculation of the modpll registers.
    * Results are compared with the original
    * results from the MATLAB single source.
    */
    const uint32_t fsys = 19200000;
    m_modPllCalc.reset (new royale::imager::ModPllStrategyM2450_A12 (fsys));
    ASSERT_NE (m_modPllCalc, nullptr) << "pll calc instance is null.";

    std::vector<uint16_t> pllcfg (8);

    uint16_t maxDiff = 0;

    std::vector<std::vector<double>> testVectors = loadTestVectors ("modpll_m2450_A12_19MHz2_checker.csv");

    if (!testVectors.size())
    {
        testVectors.push_back ({ 19200000, 12700000, 0, 20000, 0.0000, 0.0025, 18464, 43691, 970, 21801, 18350, 52935, 57089, 1, 0, 1, 1, 0 });
        testVectors.push_back ({ 19200000, 12710000, 0, 20000, 0.0000, 0.0025, 18464, 48060, 971, 21801, 22544, 53192, 57089, 1, 0, 0, 0, 0 });
        testVectors.push_back ({ 19200000, 209020000, 0, 20000, 0.0000, 0.0025, 24608, 30583, 1009, 21801, 64422, 56557, 57089, 1, 0, 0, 0, 0 });
        testVectors.push_back ({ 19200000, 400000000, 0, 20000, 0.0000, 0.0025, 24648, 43691, 3050, 21801, 0, 36580, 57091, 1, 0, 1, 1, 0 });

        testVectors.push_back ({ 19200000, 12800000, 1, 10000, 0.5000, 0.0125, 18464, 21845, 1749, 21801, 52429, 36300, 48900, 3, 0, 1, 1, 0 });
        testVectors.push_back ({ 19200000, 12810000, 1, 10000, 0.5000, 0.0125, 18464, 26214, 1750, 21801, 56361, 36557, 48900, 3, 0, 0, 0, 0 });
        testVectors.push_back ({ 19200000, 205300000, 1, 10000, 0.5000, 0.0125, 16416, 43691, 1752, 21801, 7373, 37072, 48900, 3, 0, 0, 0, 0 });
        testVectors.push_back ({ 19200000, 400000000, 1, 10000, 0.5000, 0.0125, 24648, 43691, 3818, 21801, 0, 58586, 48904, 3, 0, 1, 1, 0 });
    };

    auto vectorCount = 0u;

    for (auto testVector : testVectors)
    {
        auto frequency = testVector.at (1);

        //test only for the current config's system frequency, ignore other test vectors
        if (testVector.at (0) == static_cast<double> (fsys))
        {
            vectorCount++;

            bool validFreq = m_modPllCalc->pllSettings (frequency, testVector.at (2) == 0. ? false : true, pllcfg, testVector.at (3), testVector.at (4), testVector.at (5));

            LOG (INFO) << "PLLCFG1: 0x" << std::hex << pllcfg.at (0) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (6)) <<

                       "   PLLCFG2: 0x" << std::hex << pllcfg.at (1) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (7)) <<

                       "   PLLCFG3: 0x" << std::hex << pllcfg.at (2) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (8)) <<

                       "   PLLCFG4: 0x" << std::hex << pllcfg.at (3) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (9)) <<

                       "   PLLCFG5: 0x" << std::hex << pllcfg.at (4) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (10)) <<

                       "   PLLCFG6: 0x" << std::hex << pllcfg.at (5) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (11)) <<

                       "   PLLCFG7: 0x" << std::hex << pllcfg.at (6) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (12)) <<

                       "   PLLCFG8: 0x" << std::hex << pllcfg.at (7) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (13));

            uint16_t diff = static_cast<uint16_t> (::fabs (testVector.at (7) - pllcfg.at (1)));
            if (diff > maxDiff)
            {
                maxDiff = diff;
            }

            //if the testVector marks a frequency as invalid, the modpll calc also must detect this
            if ( (testVector.at (14) + testVector.at (15) > 0) && validFreq == true)
            {
                LOG (ERROR) << "Forbidden frequency check compliance failed at frequency: " << frequency;
                ASSERT_TRUE (false);
            }

            //compare the three PLL register settings
            ASSERT_EQ (pllcfg.at (0), testVector.at (6)) << "@" << frequency << "Hz";
            ASSERT_NEAR (pllcfg.at (1), testVector.at (7), 2) << "@" << frequency << "Hz"; //allow 1d LSB deviance
            ASSERT_EQ (pllcfg.at (2), testVector.at (8)) << "@" << frequency << "Hz";
            ASSERT_EQ (pllcfg.at (3), testVector.at (9)) << "@" << frequency << "Hz";
            if (testVector.at (2) > 0)
            {
                ASSERT_EQ (pllcfg.at (4), testVector.at (10)) << "@" << frequency << "Hz";
                ASSERT_EQ (pllcfg.at (5), testVector.at (11)) << "@" << frequency << "Hz";
                ASSERT_EQ (pllcfg.at (6), testVector.at (12)) << "@" << frequency << "Hz";
                ASSERT_EQ (pllcfg.at (7), testVector.at (13)) << "@" << frequency << "Hz";
            }

            // backward calculation from register data to frequency and SSC

            royale::imager::IPllStrategy::ReversePllSettings settings;
            const std::vector<uint16_t> registers = { static_cast<uint16_t> (testVector.at (6)), static_cast<uint16_t> (testVector.at (7)),
                                                      static_cast<uint16_t> (testVector.at (8)), static_cast<uint16_t> (testVector.at (9)),
                                                      static_cast<uint16_t> (testVector.at (10)), static_cast<uint16_t> (testVector.at (11)),
                                                      static_cast<uint16_t> (testVector.at (12)), static_cast<uint16_t> (testVector.at (13))
                                                    };
            m_modPllCalc->reversePllSettings (registers, settings);

            if (validFreq && testVector.at (2) > 0)
            {
                ASSERT_NEAR (settings.outFreq, frequency, 2) << "@" << frequency << "Hz";
                ASSERT_NEAR (settings.sscFreq, testVector.at (3), 150) << "@" << frequency << "Hz";
                ASSERT_NEAR (settings.sscKspread, testVector.at (4), 0.01) << "@" << frequency << "Hz";
                ASSERT_NEAR (settings.sscDelta, testVector.at (5), 0.001) << "@" << frequency << "Hz";
            }
        }
    }

    // Check that we tested any vectors at all
    EXPECT_GT (vectorCount, 0u);

    LOG (INFO) << "Max diff: " << maxDiff;
}

TEST_F (TestPllStrategyM2450_A12, DPhyPllCalc)
{
    /*
    * Perform a calculation of the dphypll registers.
    * Results are compared with the original
    * results from the MATLAB single source.
    */
    std::vector<uint16_t> pllcfg (8);

    std::vector<std::vector<double>> testVectors = loadTestVectors ("dphypll_m2450_A12_checker.csv");

    if (!testVectors.size())
    {
        testVectors.push_back ({ 10000000, 400000000, 0, 30000, 0.5000, 0.0150, 38049, 21928, 39322, 63193, 42357, 0, 0, 0, 1, 0 });
        testVectors.push_back ({ 22000000, 400000000, 1, 30000, 0, 0.0050, 42019, 21928, 17873, 32535, 46624, 0, 35746, 1582, 0, 0 });
    };

    for (auto testVector : testVectors)
    {
        auto frequency = testVector.at (0);
        m_dphyPllCalc.reset (new royale::imager::DphyPllStrategyM2450_A12 ( (uint32_t) frequency));
        ASSERT_NE (m_dphyPllCalc, nullptr) << "pll calc instance is null.";

        bool validFreq = m_dphyPllCalc->pllSettings (testVector.at (1), testVector.at (2) == 0. ? false : true, pllcfg, testVector.at (3), testVector.at (4), testVector.at (5));

        LOG (INFO) << "PLLCFG1: 0x" << std::hex << pllcfg.at (0) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (6)) <<

                   "   PLLCFG2: 0x" << std::hex << pllcfg.at (1) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (7)) <<

                   "   PLLCFG3: 0x" << std::hex << pllcfg.at (2) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (8)) <<

                   "   PLLCFG4: 0x" << std::hex << pllcfg.at (3) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (9)) <<

                   "   PLLCFG5: 0x" << std::hex << pllcfg.at (4) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (10)) <<

                   "   PLLCFG6: 0x" << std::hex << pllcfg.at (5) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (11)) <<

                   "   PLLCFG7: 0x" << std::hex << pllcfg.at (6) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (12)) <<

                   "   PLLCFG8: 0x" << std::hex << pllcfg.at (7) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (13));


        //if the testVector marks a frequency as invalid, the dphy pll calc also must detect this
        if ( (testVector.at (14) + testVector.at (15) > 0) && validFreq == true)
        {
            LOG (ERROR) << "Forbidden frequency check compliance failed at frequency: " << testVector.at (1);
            ASSERT_TRUE (false) << "@fsys:" << frequency << "Hz";
        }

        //compare the PLL register settings (ssc is not used therefore only 4 comparisions)
        ASSERT_EQ (pllcfg.at (0), testVector.at (6)) << "@fsys:" << frequency << "Hz";
        ASSERT_EQ (pllcfg.at (1), testVector.at (7)) << "@fsys:" << frequency << "Hz";
        if (testVector.at (2) > 0)
        {
            ASSERT_EQ (pllcfg.at (2), testVector.at (8)) << "@fsys:" << frequency << "Hz";
            ASSERT_EQ (pllcfg.at (3), testVector.at (9)) << "@fsys:" << frequency << "Hz";
            ASSERT_EQ (pllcfg.at (4), testVector.at (10)) << "@fsys:" << frequency << "Hz";
            ASSERT_EQ (pllcfg.at (5), testVector.at (11)) << "@fsys:" << frequency << "Hz";
        }
        ASSERT_NEAR (pllcfg.at (6), testVector.at (12), 2) << "@fsys:" << frequency << "Hz"; //allow 1d LSB deviance
        ASSERT_EQ (pllcfg.at (7), testVector.at (13)) << "@fsys:" << frequency << "Hz";

        // backward calculation from register data to frequency and SSC

        royale::imager::IPllStrategy::ReversePllSettings settings;
        const std::vector<uint16_t> registers = { static_cast<uint16_t> (testVector.at (6)), static_cast<uint16_t> (testVector.at (7)),
                                                  static_cast<uint16_t> (testVector.at (8)), static_cast<uint16_t> (testVector.at (9)),
                                                  static_cast<uint16_t> (testVector.at (10)), static_cast<uint16_t> (testVector.at (11)),
                                                  static_cast<uint16_t> (testVector.at (12)), static_cast<uint16_t> (testVector.at (13))
                                                };
        m_dphyPllCalc->reversePllSettings (registers, settings);

        if (validFreq && testVector.at (2) > 0)
        {
            ASSERT_NEAR (settings.outFreq, testVector.at (1), 3) << "@" << frequency << "Hz";
            ASSERT_NEAR (settings.sscFreq, testVector.at (3), 150) << "@" << frequency << "Hz";
            ASSERT_NEAR (settings.sscKspread, testVector.at (4), 0.01) << "@" << frequency << "Hz";
            ASSERT_NEAR (settings.sscDelta, testVector.at (5), 0.001) << "@" << frequency << "Hz";
        }
    }
}
