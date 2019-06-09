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
#include <imager/ModPllStrategyM2452.hpp>
#include <imager/DphyPllStrategyM2452.hpp>

//use the define to enable fast testing
#undef ENABLE_FASTPLLTESTING

/*
* Tests require external test data files, by default they are not used and the test
* is reduced to one vector entry only due to timing contraints for unit tests (compilation and runtime).
* Use the preprocessor flag to enable full testing.
*/
class TestPllStrategyM2452 : public ::testing::Test
{
protected:
    TestPllStrategyM2452()
    {
    }

    virtual ~TestPllStrategyM2452()
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
        m_pllCalc.reset();
    }

protected:
    std::unique_ptr <royale::imager::IPllStrategy> m_pllCalc;
    std::unique_ptr <royale::imager::IPllStrategy> m_dphyPllCalc;
};

TEST_F (TestPllStrategyM2452, ModPllCalcNegative)
{
    m_pllCalc.reset (new royale::imager::ModPllStrategyM2452 (24000000));
    ASSERT_NE (m_pllCalc, nullptr) << "PLL calculation instance is null.";

    std::vector<uint16_t> pllcfg (8);
    ASSERT_FALSE (m_pllCalc->pllSettings (12600000 - 1, false, pllcfg));
    ASSERT_FALSE (m_pllCalc->pllSettings (400000000 + 1, false, pllcfg));
    ASSERT_THROW (m_pllCalc->pllSettings (12600000, true, pllcfg, 0.0), royale::common::LogicError);

    // test vector contains fssc > 1e5, which has to result in a return value of false
    ASSERT_FALSE (m_pllCalc->pllSettings (18400000, true, pllcfg, 100001, 0.5000, 0.0010));

    // no ssc possible frequency to high/low, which has to result in a return value of false
    ASSERT_FALSE (m_pllCalc->pllSettings (12600000, true, pllcfg, 10000, 0.0, 0.1));

    // no ssc possible at this frequency with the given parameters
    ASSERT_FALSE (m_pllCalc->pllSettings (85500000, true, pllcfg, 10000, 0.0, 0.001));
}

TEST_F (TestPllStrategyM2452, ModPllCalc)
{
    /*
    * Create an instance of a ImagerM2450_A12 subclass with direct
    * access to the modpll calculation and perform a calculation
    * of the modpll registers. Results are compared with the original
    * results from the MATLAB single source.
    */
    m_pllCalc.reset (new royale::imager::ModPllStrategyM2452 (24000000));
    ASSERT_NE (m_pllCalc, nullptr) << "PLL calculation instance is null.";

    std::vector<uint16_t> pllcfg (8);

    uint16_t maxDiff = 0;

    std::vector<std::vector<double>> testVectors = loadTestVectors ("modpll_m2452_A11_24MHz_checker.csv");

    if (!testVectors.size())
    {
        testVectors.push_back ({ 24000000, 12600000, 0, 10000, 0.0000, 0.0010, 27193, 26214, 998, 21801, 16358, 60388, 22272, 2, });
        testVectors.push_back ({ 24000000, 12600000, 1, 10000, 0.0000, 0.0010, 27193, 26214, 1766, 21801, 16358, 60388, 22272, 2, });
    };

    for (auto testVector : testVectors)
    {
        auto frequency = testVector.at (1);
        bool validFreq = false;
        //test only for the current config's system frequency, ignore other test vectors
        if (testVector.at (0) == static_cast<double> (24000000))
        {
            validFreq = m_pllCalc->pllSettings (frequency, testVector.at (2) == 0. ? false : true, pllcfg, testVector.at (3), testVector.at (4), testVector.at (5));
            ASSERT_TRUE (validFreq);

            LOG (INFO) << "PLLCFG1: 0x" << std::hex << pllcfg.at (0) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (6)) <<

                       "   PLLCFG2: 0x" << std::hex << pllcfg.at (1) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (7)) <<

                       "   PLLCFG3: 0x" << std::hex << pllcfg.at (2) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (8)) <<

                       "   PLLCFG4: 0x" << std::hex << pllcfg.at (3) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (9)) <<

                       "   PLLCFG5: 0x" << std::hex << pllcfg.at (4) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (10)) <<

                       "   PLLCFG6: 0x" << std::hex << pllcfg.at (5) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (11)) <<

                       "   PLLCFG7: 0x" << std::hex << pllcfg.at (6) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (12)) <<

                       "   PLLCFG8: 0x" << std::hex << pllcfg.at (7) << " <=> 0x" << std::hex << static_cast<uint16_t> (testVector.at (13));

            uint16_t diff;
            diff = static_cast<uint16_t> (::fabs (testVector.at (7) - pllcfg.at (1)));
            if (diff > maxDiff)
            {
                maxDiff = diff;
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
        }

        // backward calculation from register data to frequency and SSC

        royale::imager::IPllStrategy::ReversePllSettings settings;
        const std::vector<uint16_t> registers = { static_cast<uint16_t> (testVector.at (6)), static_cast<uint16_t> (testVector.at (7)),
                                                  static_cast<uint16_t> (testVector.at (8)), static_cast<uint16_t> (testVector.at (9)),
                                                  static_cast<uint16_t> (testVector.at (10)), static_cast<uint16_t> (testVector.at (11)),
                                                  static_cast<uint16_t> (testVector.at (12)), static_cast<uint16_t> (testVector.at (13))
                                                };
        m_pllCalc->reversePllSettings (registers, settings);

        if (validFreq && testVector.at (2) > 0)
        {
            ASSERT_NEAR (settings.outFreq, frequency, 0.1) << "@" << frequency << "Hz";
            ASSERT_NEAR (settings.sscFreq, testVector.at (3), 0.01) << "@" << frequency << "Hz";
            ASSERT_NEAR (settings.sscKspread, testVector.at (4), 0.01) << "@" << frequency << "Hz";
            ASSERT_NEAR (settings.sscDelta, testVector.at (5), 0.001) << "@" << frequency << "Hz";
        }
    }

    LOG (INFO) << "Max diff: " << maxDiff;
}

TEST_F (TestPllStrategyM2452, DPhyPllCalc)
{
    /*
    * Perform a calculation of the dphypll registers.
    * Results are compared with the original
    * results from the MATLAB single source.
    */
    std::vector<uint16_t> pllcfg (8);

    std::vector<std::vector<double>> testVectors = loadTestVectors ("dphypll_m2452_A11_checker.csv");

    if (!testVectors.size())
    {
        testVectors.push_back ({ 10000000, 400000000, 0, 30000, 0.0000, 0.0050, 38049, 21928, 26214, 21222, 42535, 0, 0, 2048, 1, 0, });
        testVectors.push_back ({ 10000000, 400000000, 1, 30000, 0.0000, 0.0050, 33953, 21928, 26214, 21222, 42535, 0, 0, 3584, 1, 0, });
    };

    for (auto testVector : testVectors)
    {
        auto frequency = testVector.at (0);
        m_dphyPllCalc.reset (new royale::imager::DphyPllStrategyM2452 ( (uint32_t) frequency));
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

        //compare the PLL register settings
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
            ASSERT_NEAR (settings.outFreq, testVector.at (1), 2.) << "@" << frequency << "Hz";
            ASSERT_NEAR (settings.sscFreq, testVector.at (3), 150.) << "@" << frequency << "Hz";
            ASSERT_NEAR (settings.sscKspread, testVector.at (4), 0.01) << "@" << frequency << "Hz";
            ASSERT_NEAR (settings.sscDelta, testVector.at (5), 0.001) << "@" << frequency << "Hz";
        }
    }
}
