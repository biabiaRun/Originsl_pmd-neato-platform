#ifdef _WIN32
#pragma warning( disable : 4996 )
#endif
#include <royale/Vector.hpp>
#include <royale/Pair.hpp>
#include <royale/String.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <algorithm>

using namespace royale;
using namespace royale::iterator;

namespace
{
    class DoubleDestructionCanary;


    /**
    * This class counts the destructor calls of DoubleDestructionCanary.
    */
    class DestructorCounter
    {
    public:
        DestructorCounter();
        virtual ~DestructorCounter();
        MOCK_METHOD1 (destructorCalled, void (DoubleDestructionCanary *));
    };
    /**
     * This class will crash if its destructor is called a second time
     */
    class DoubleDestructionCanary
    {
        /**
         * Memory-checking tools may be able to report the site of the first dealloc, so this
         * pointer is used to trigger any tool if it's running.
         */
        char *m_allocated;

        /** If there's no tool analysing the code, crash on double destruction. */
        bool m_destroyed;

    public:
        DoubleDestructionCanary()
            : m_allocated (new char),
              m_destroyed (false)
        {
            assertObject();
        }

        /**
         * Copy constructor - just create a separate instance, because each instance keeps track of
         * its own destruction.
         */
        DoubleDestructionCanary (const DoubleDestructionCanary &)
            : m_allocated (new char),
              m_destroyed (false)
        {
            assertObject();
        }

        /**
        * Tell DestructorCounter to expect one destructor call
        * for this object.
        */
        void assertObject()
        {
            if (counter)
            {
                EXPECT_CALL (*counter, destructorCalled (this))
                .Times (1);
            }
        }

        /**
         * Copy allocator - do nothing, each instance keeps track of its own destruction.
         */
        DoubleDestructionCanary &operator= (const DoubleDestructionCanary &)
        {
            return *this;
        }



        /**
        * Destructor- informs DestructorCounter about all calls
        * Trigger double delete and subsequent termination.
        */
        ~DoubleDestructionCanary()
        {
            if (counter)
            {
                counter->destructorCalled (this);
            }
            EXPECT_THAT (m_destroyed, testing::Eq (false));
            delete m_allocated;
            if (m_destroyed)
            {
                std::terminate();
            }
            m_destroyed = true;
        }

        static DestructorCounter *counter;
    };

    DestructorCounter *DoubleDestructionCanary::counter = nullptr;

    DestructorCounter::DestructorCounter()
    {
        DoubleDestructionCanary::counter = this;
    }
    DestructorCounter::~DestructorCounter()
    {
        DoubleDestructionCanary::counter = nullptr;
    }
}

TEST (TestVectorPair, TestVectorInt)
{
    // Test ints
    size_t elements = 0;
    royale::Vector<uint32_t> v;
    ASSERT_EQ (v.size(), 0u);
    ASSERT_EQ (v.empty(), true);

    v.push_back (30);
    ASSERT_EQ (v.empty(), false);
    ASSERT_EQ (v.back(), 30u);
    ASSERT_EQ (v.capacity(), 1u);
    ASSERT_EQ (v.size(), 1u);

    v.push_back (10);
    ASSERT_EQ (v.at (1), 10u);
    ASSERT_EQ (v.capacity(), 2u);
    ASSERT_EQ (v.size(), 2u);

    v.push_back (20);
    ASSERT_EQ (v.back(), 20u);
    ASSERT_EQ (v.capacity(), 4u);
    ASSERT_EQ (v.size(), 3u);

    royale::Vector<uint32_t> v2 (v);
    elements = v2.size() >= v.size() ? v2.size() : v.size();
    for (size_t i = 0; i < elements; ++i)
    {
        ASSERT_EQ (v.at (i), v2.at (i));
    }

    v.clear();
    for (size_t i = 0; i < v.size(); ++i)
    {
        ASSERT_NE (v.at (i), v2.at (i));
    }
    ASSERT_EQ (v.capacity(), 0u);

    ASSERT_EQ (v2.capacity(), 4u);
    ASSERT_EQ (v2.size(), 3u);

    v2.pop_back();

    ASSERT_EQ (v2.capacity(), 4u);
    ASSERT_EQ (v2.size(), 2u);

    royale::Vector<uint32_t> v3;
    v3 = v2;

    ASSERT_EQ (v3.capacity(), 4u);
    ASSERT_EQ (v3.size(), 2u);
    elements = v3.size() >= v2.size() ? v3.size() : v2.size();
    for (size_t i = 0; i < elements; ++i)
    {
        ASSERT_EQ (v3.at (i), v2.at (i));
    }

    v3[1] = 100;
    v3.push_back (75);
    ASSERT_EQ (v3.capacity(), 4u);
    ASSERT_EQ (v2[0], v3.at (0));
    ASSERT_EQ (v3[1], 100u);
    ASSERT_EQ (v2[1], 10u);
    ASSERT_EQ (v2.size(), 2u);
    ASSERT_EQ (v3.size(), 3u);
    ASSERT_EQ (v3.back(), v3.at (2));
    ASSERT_EQ (v3[v3.size() - 1], 75u);
}

TEST (TestVectorPair, TestVectorString)
{
    // Test Strings
    royale::Vector< std::string > vs;

    vs.push_back ("Hello");
    ASSERT_EQ (vs.back(), "Hello");
    vs.push_back ("World");
    ASSERT_EQ (vs.back(), "World");
    vs.push_back ("Infineon");
    ASSERT_EQ (vs.back(), "Infineon");
    ASSERT_EQ (vs.size(), 3u);

    vs.pop_back();

    ASSERT_EQ (vs[0], "Hello");
    ASSERT_EQ (vs.at (1), "World");
    ASSERT_EQ (vs.size(), 2u);
    ASSERT_THROW (vs.at (2), std::out_of_range);

    royale::Vector< std::string > vs2 (vs);
    vs.clear();

    ASSERT_EQ (vs2[0], "Hello");
    ASSERT_EQ (vs2.at (1), "World");
    ASSERT_EQ (vs2.size(), 2u);
    ASSERT_THROW (vs2.at (2), std::out_of_range);
}

TEST (TestVectorPair, TestVectorIntReserveAndResize)
{
    Vector<uint32_t> royaleIntVector;

    royaleIntVector.reserve (1);
    ASSERT_EQ (royaleIntVector.size(), 0u);

    royaleIntVector.resize (1);
    ASSERT_EQ (royaleIntVector.size(), 1u);
}

TEST (TestVectorPair, TestVectorResize)
{
    // char
    {
        Vector<char> royaleCharVector ({ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i' });
        royaleCharVector.resize (20, 'X');

        ASSERT_EQ (royaleCharVector.size(), 20u);

        ASSERT_EQ (royaleCharVector.at (0), 'a');
        ASSERT_EQ (royaleCharVector.at (1), 'b');
        ASSERT_EQ (royaleCharVector.at (2), 'c');
        ASSERT_EQ (royaleCharVector.at (3), 'd');
        ASSERT_EQ (royaleCharVector.at (4), 'e');
        ASSERT_EQ (royaleCharVector.at (5), 'f');
        ASSERT_EQ (royaleCharVector.at (6), 'g');
        ASSERT_EQ (royaleCharVector.at (7), 'h');
        ASSERT_EQ (royaleCharVector.at (8), 'i');

        size_t i;
        for (i = 9; i < royaleCharVector.size(); ++i)
        {
            ASSERT_EQ (royaleCharVector.at (i), 'X');
        }
        royaleCharVector.emplace_back ('s');
        royaleCharVector.push_back ('t');
        ASSERT_EQ (royaleCharVector.at (i++), 's');
        ASSERT_EQ (royaleCharVector.at (i++), 't');

        ASSERT_EQ (royaleCharVector.size(), 22u);
        ASSERT_GE (royaleCharVector.capacity(), 22u);
    }

    // more complex data type (String)
    {
        Vector<String> royaleStringVector ({ "a#1", "b#2", "c#3", "d#4", "e#5", "f#6", "g#7", "h#8", "i#9" });
        royaleStringVector.resize (20, "X##");

        ASSERT_EQ (royaleStringVector.size(), 20u);

        ASSERT_EQ (royaleStringVector.at (0), "a#1");
        ASSERT_EQ (royaleStringVector.at (1), "b#2");
        ASSERT_EQ (royaleStringVector.at (2), "c#3");
        ASSERT_EQ (royaleStringVector.at (3), "d#4");
        ASSERT_EQ (royaleStringVector.at (4), "e#5");
        ASSERT_EQ (royaleStringVector.at (5), "f#6");
        ASSERT_EQ (royaleStringVector.at (6), "g#7");
        ASSERT_EQ (royaleStringVector.at (7), "h#8");
        ASSERT_EQ (royaleStringVector.at (8), "i#9");

        size_t i;
        for (i = 9; i < royaleStringVector.size(); ++i)
        {
            ASSERT_EQ (royaleStringVector.at (i), "X##");
        }
        royaleStringVector.emplace_back ("s#21");
        royaleStringVector.push_back ("t#22");
        ASSERT_EQ (royaleStringVector.at (i++), "s#21");
        ASSERT_EQ (royaleStringVector.at (i++), "t#22");

        ASSERT_EQ (royaleStringVector.size(), 22u);
        ASSERT_GE (royaleStringVector.capacity(), 22u);
    }
}

TEST (TestVectorPair, TestVectorStringResize)
{
    Vector<std::string> royaleStringVector;
    royaleStringVector.resize (3);

    ASSERT_EQ (royaleStringVector.size(), 3u);
    ASSERT_EQ (royaleStringVector.at (0), std::string());
    ASSERT_EQ (royaleStringVector.at (1), std::string());
    ASSERT_EQ (royaleStringVector.at (2), std::string());
    ASSERT_THROW (royaleStringVector.at (3), std::out_of_range);

    royaleStringVector[1] = "Hallo Welt";
    ASSERT_EQ (royaleStringVector.at (0), std::string());
    ASSERT_EQ (royaleStringVector.at (1), std::string ("Hallo Welt"));
    ASSERT_EQ (royaleStringVector.at (2), std::string());
    ASSERT_THROW (royaleStringVector.at (3), std::out_of_range);

    Vector<std::string> royaleStringVector2;
    royaleStringVector2.resize (3, std::string ("Hallo neue Welt"));
    ASSERT_EQ (royaleStringVector2.at (0), std::string ("Hallo neue Welt"));
    ASSERT_EQ (royaleStringVector2.at (1), std::string ("Hallo neue Welt"));
    ASSERT_EQ (royaleStringVector2.at (2), std::string ("Hallo neue Welt"));
    ASSERT_THROW (royaleStringVector2.at (3), std::out_of_range);

    Vector<std::string> royaleStringVector3;
    royaleStringVector3.push_back (std::string ("Hallo neue Welt"));
    royaleStringVector3.push_back (std::string ("Hallo neue Welt"));
    royaleStringVector3.push_back (std::string ("Hallo neue Welt"));
    royaleStringVector3.push_back (std::string ("Hallo neue Welt"));

    royaleStringVector3.resize (100);
    ASSERT_EQ (royaleStringVector3.size(), 100u);
    ASSERT_GE (royaleStringVector3.capacity(), 100u);
    ASSERT_NO_THROW (royaleStringVector3.at (99));
    ASSERT_EQ (royaleStringVector3.at (99), std::string());

    ASSERT_EQ (royaleStringVector3.at (0), std::string ("Hallo neue Welt"));
    ASSERT_EQ (royaleStringVector3.at (1), std::string ("Hallo neue Welt"));
    ASSERT_EQ (royaleStringVector3.at (2), std::string ("Hallo neue Welt"));
    ASSERT_EQ (royaleStringVector3.at (3), std::string ("Hallo neue Welt"));
    ASSERT_EQ (royaleStringVector3.at (4), std::string());
    ASSERT_EQ (royaleStringVector3.at (5), std::string());
    royaleStringVector3.push_back (std::string ("Hallo neue Welt"));
    royaleStringVector3.push_back (std::string ("Hallo neue Welt"));

    ASSERT_EQ (royaleStringVector3.at (0), std::string ("Hallo neue Welt"));
    ASSERT_EQ (royaleStringVector3.at (1), std::string ("Hallo neue Welt"));
    ASSERT_EQ (royaleStringVector3.at (2), std::string ("Hallo neue Welt"));
    ASSERT_EQ (royaleStringVector3.at (3), std::string ("Hallo neue Welt"));

    ASSERT_EQ (royaleStringVector3.at (100), std::string ("Hallo neue Welt"));
    ASSERT_EQ (royaleStringVector3.at (101), std::string ("Hallo neue Welt"));

    ASSERT_THROW (royaleStringVector3.at (102), std::out_of_range);
    ASSERT_EQ (royaleStringVector3.size(), 102u);
    ASSERT_GE (royaleStringVector3.capacity(), 102u);

    for (size_t i = 0; i < 80; i++)
    {
        royaleStringVector3.pop_back();
    }

    ASSERT_EQ (royaleStringVector3.size(), 22u);
    ASSERT_GE (royaleStringVector3.capacity(), 32u);
    ASSERT_NE (royaleStringVector3.front(), std::string ("Hallo neue Welt!"));
    ASSERT_NE (royaleStringVector3.back(), std::string ("!"));
}

TEST (TestVectorPair, TestVectorIntPairs)
{
    // Test Pair of int's
    royale::Vector< royale::Pair<int, int> > vpi;
    vpi.push_back (royale::royale_pair (1, 10));
    vpi.push_back (royale::royale_pair (2, 20));
    vpi.push_back (royale::royale_pair (3, 30));
    vpi.push_back (royale::royale_pair (4, 40));

    ASSERT_EQ (vpi[0].first, 1);
    ASSERT_EQ (vpi[0].second, 10);

    ASSERT_EQ (vpi.at (3).first, 4);
    ASSERT_EQ (vpi.at (3).second, 40);

    ASSERT_EQ (vpi.back().first, 4);
    ASSERT_EQ (vpi.back().second, 40);

    ASSERT_EQ (vpi.at (vpi.size() - 1).first, 4);
    ASSERT_EQ (vpi[vpi.size() - 1].second, 40);

    ASSERT_EQ (vpi.at (vpi.size() - 1).first, 4);
    ASSERT_EQ (vpi[vpi.size() - 1].second, 40);

    vpi.pop_back();

    ASSERT_EQ (vpi.at (2).first, 3);
    ASSERT_EQ (vpi.at (2).second, 30);

    ASSERT_EQ (vpi.back().first, 3);
    ASSERT_EQ (vpi.back().second, 30);

    ASSERT_EQ (vpi.at (vpi.size() - 1).first, 3);
    ASSERT_EQ (vpi[vpi.size() - 1].second, 30);

    ASSERT_EQ (vpi.at (vpi.size() - 1).first, 3);
    ASSERT_EQ (vpi[vpi.size() - 1].second, 30);

    vpi.pop_back();

    ASSERT_EQ (vpi.at (1).first, 2);
    ASSERT_EQ (vpi.at (1).second, 20);

    ASSERT_EQ (vpi.back().first, 2);
    ASSERT_EQ (vpi.back().second, 20);

    ASSERT_EQ (vpi.at (vpi.size() - 1).first, 2);
    ASSERT_EQ (vpi[vpi.size() - 1].second, 20);

    ASSERT_EQ (vpi.at (vpi.size() - 1).first, 2);
    ASSERT_EQ (vpi[vpi.size() - 1].second, 20);

    vpi.push_back (royale::royale_pair (4, 40));

    ASSERT_EQ (vpi.size(), 3u);
    ASSERT_EQ (vpi.at (2).first, 4);
    ASSERT_EQ (vpi.at (2).second, 40);

    ASSERT_EQ (vpi.back().first, 4);
    ASSERT_EQ (vpi.back().second, 40);

    ASSERT_EQ (vpi.at (vpi.size() - 1).first, 4);
    ASSERT_EQ (vpi[vpi.size() - 1].second, 40);

    ASSERT_EQ (vpi.at (vpi.size() - 1).first, 4);
    ASSERT_EQ (vpi[vpi.size() - 1].second, 40);

    ASSERT_THROW (vpi.at (3), std::out_of_range);

    royale::Vector< royale::Pair<int, int> > vpi2 (vpi);
    vpi.clear();

    ASSERT_EQ (vpi2.size(), 3u);
    ASSERT_EQ (vpi2.at (2).first, 4);
    ASSERT_EQ (vpi2.at (2).second, 40);

    ASSERT_EQ (vpi2.back().first, 4);
    ASSERT_EQ (vpi2.back().second, 40);

    ASSERT_EQ (vpi2.at (vpi2.size() - 1).first, 4);
    ASSERT_EQ (vpi2[vpi2.size() - 1].second, 40);

    ASSERT_EQ (vpi2.at (vpi2.size() - 1).first, 4);
    ASSERT_EQ (vpi2[vpi2.size() - 1].second, 40);

    ASSERT_THROW (vpi2.at (3), std::out_of_range);

    royale::Vector< royale::Pair<int, int> > vpi3;
    vpi3 = vpi2;
    vpi2.clear();

    ASSERT_EQ (vpi3.size(), 3u);
    ASSERT_EQ (vpi3.at (2).first, 4);
    ASSERT_EQ (vpi3.at (2).second, 40);

    ASSERT_EQ (vpi3.back().first, 4);
    ASSERT_EQ (vpi3.back().second, 40);

    ASSERT_EQ (vpi3.at (vpi3.size() - 1).first, 4);
    ASSERT_EQ (vpi3[vpi3.size() - 1].second, 40);

    ASSERT_EQ (vpi3.at (vpi3.size() - 1).first, 4);
    ASSERT_EQ (vpi3[vpi3.size() - 1].second, 40);

    ASSERT_THROW (vpi3.at (3), std::out_of_range);
}

TEST (TestVectorPair, TestVectorStringPairs)
{
    // Test Pair of string's
    royale::Vector< royale::Pair<std::string, std::string> > vps;
    vps.push_back (royale_pair (std::string ("Bart"), std::string ("Simpson")));
    vps.push_back (royale_pair (std::string ("Lisa"), std::string ("Simpson")));
    vps.push_back (royale_pair (std::string ("Marge"), std::string ("Simpson")));
    vps.push_back (royale_pair (std::string ("Homer"), std::string ("Simpson")));

    ASSERT_EQ (vps[0].first, "Bart");
    ASSERT_EQ (vps[0].second, "Simpson");

    ASSERT_EQ (vps.at (0).first, "Bart");
    ASSERT_EQ (vps.at (0).second, "Simpson");

    ASSERT_EQ (vps.front().first, "Bart");
    ASSERT_EQ (vps.front().second, "Simpson");

    ASSERT_EQ (vps.startsWith (royale::royale_pair (std::string ("Bart"), std::string ("Simpson"))), true);
    ASSERT_EQ (vps.endsWith (royale::royale_pair (std::string ("Homer"), std::string ("Simpson"))), true);

    ASSERT_EQ (vps.first().first, "Bart");
    ASSERT_EQ (vps.first().second, "Simpson");

    ASSERT_EQ (vps.at (3).first, "Homer");
    ASSERT_EQ (vps.at (3).second, "Simpson");

    ASSERT_EQ (vps.back().first, "Homer");
    ASSERT_EQ (vps.last().second, "Simpson");

    ASSERT_EQ (vps.at (vps.size() - 1).first, "Homer");
    ASSERT_EQ (vps[vps.size() - 1].second, "Simpson");

    ASSERT_EQ (vps.at (vps.size() - 1).first, "Homer");
    ASSERT_EQ (vps[vps.size() - 1].second, "Simpson");

    vps.pop_back();

    ASSERT_EQ (vps.at (2).first, "Marge");
    ASSERT_EQ (vps.at (2).second, "Simpson");

    ASSERT_EQ (vps.back().first, "Marge");
    ASSERT_EQ (vps.last().second, "Simpson");

    ASSERT_EQ (vps.at (vps.size() - 1).first, "Marge");
    ASSERT_EQ (vps[vps.size() - 1].second, "Simpson");

    ASSERT_EQ (vps.at (vps.size() - 1).first, "Marge");
    ASSERT_EQ (vps[vps.size() - 1].second, "Simpson");

    vps.pop_back();

    ASSERT_EQ (vps.at (1).first, "Lisa");
    ASSERT_EQ (vps.at (1).second, "Simpson");

    ASSERT_EQ (vps.back().first, "Lisa");
    ASSERT_EQ (vps.last().second, "Simpson");

    ASSERT_EQ (vps.at (vps.size() - 1).first, "Lisa");
    ASSERT_EQ (vps[vps.size() - 1].second, "Simpson");

    ASSERT_EQ (vps.at (vps.size() - 1).first, "Lisa");
    ASSERT_EQ (vps[vps.size() - 1].second, "Simpson");

    vps.push_back (royale_pair (std::string ("Maggie"), std::string ("Simpson")));

    ASSERT_EQ (vps.size(), 3u);
    ASSERT_EQ (vps.at (1).first, "Lisa");
    ASSERT_EQ (vps.at (1).second, "Simpson");

    ASSERT_EQ (vps.back().first, "Maggie");
    ASSERT_EQ (vps.last().second, "Simpson");

    ASSERT_EQ (vps.at (vps.size() - 1).first, "Maggie");
    ASSERT_EQ (vps[vps.size() - 1].second, "Simpson");

    ASSERT_EQ (vps.at (vps.size() - 1).first, "Maggie");
    ASSERT_EQ (vps[vps.size() - 1].second, "Simpson");

    ASSERT_THROW (vps.at (3), std::out_of_range);

    royale::Vector< royale::Pair<std::string, std::string> > vps2 (vps);
    vps.clear();
    ASSERT_EQ (vps2.size(), 3u);
    ASSERT_EQ (vps2.at (1).first, "Lisa");
    ASSERT_EQ (vps2.at (1).second, "Simpson");

    ASSERT_EQ (vps2.last().first, "Maggie");
    ASSERT_EQ (vps2.back().second, "Simpson");

    ASSERT_EQ (vps2.at (vps2.size() - 1).first, "Maggie");
    ASSERT_EQ (vps2[vps2.size() - 1].second, "Simpson");

    ASSERT_EQ (vps2.at (vps2.size() - 1).first, "Maggie");
    ASSERT_EQ (vps2[vps2.size() - 1].second, "Simpson");

    ASSERT_THROW (vps2.at (3), std::out_of_range);

    /* Test const access */
    {
        const royale::Vector< royale::Pair<std::string, std::string> > &vpsRef = vps2;
        ASSERT_EQ (vpsRef.size(), 3u);
        ASSERT_EQ (vpsRef.at (1).first, "Lisa");
        ASSERT_EQ (vpsRef.at (1).second, "Simpson");

        ASSERT_EQ (vpsRef.last().first, "Maggie");
        ASSERT_EQ (vpsRef.last().second, "Simpson");

        ASSERT_EQ (vpsRef.at (vpsRef.size() - 1).first, "Maggie");
        ASSERT_EQ (vpsRef[vpsRef.size() - 1].second, "Simpson");

        ASSERT_EQ (vpsRef.at (vpsRef.size() - 1).first, "Maggie");
        ASSERT_EQ (vpsRef[vpsRef.size() - 1].second, "Simpson");

        ASSERT_THROW (vpsRef.at (3), std::out_of_range);

        ASSERT_EQ (vpsRef.front().first, "Bart");
        ASSERT_EQ (vpsRef.front().second, "Simpson");

        ASSERT_EQ (vpsRef.first().first, "Bart");
        ASSERT_EQ (vpsRef.first().second, "Simpson");

        ASSERT_EQ (vpsRef.startsWith (royale::royale_pair (std::string ("Bart"), std::string ("Simpson"))), true);
        ASSERT_EQ (vpsRef.endsWith (royale::royale_pair (std::string ("Maggie"), std::string ("Simpson"))), true);

        ASSERT_EQ (vpsRef[0].first, "Bart");
        ASSERT_EQ (vpsRef[0].second, "Simpson");
    }
    /* ----------------- */

    royale::Vector< royale::Pair<std::string, std::string> > vps3;
    vps3 = vps2;
    vps2.clear();

    ASSERT_EQ (vps3.size(), 3u);
    ASSERT_EQ (vps3.at (1).first, "Lisa");
    ASSERT_EQ (vps3.at (1).second, "Simpson");

    ASSERT_EQ (vps3.last().first, "Maggie");
    ASSERT_EQ (vps3.back().second, "Simpson");

    ASSERT_EQ (vps3.at (vps3.size() - 1).first, "Maggie");
    ASSERT_EQ (vps3[vps3.size() - 1].second, "Simpson");

    ASSERT_EQ (vps3.at (vps3.size() - 1).first, "Maggie");
    ASSERT_EQ (vps3[vps3.size() - 1].second, "Simpson");

    ASSERT_THROW (vps3.at (3), std::out_of_range);

    size_t capacity_old = vps3.capacity();

    vps3.pop_back();

    ASSERT_EQ (vps3.size(), 2u);
    ASSERT_EQ (vps3.at (0).first, "Bart");
    ASSERT_EQ (vps3.at (0).second, "Simpson");

    ASSERT_EQ (vps3.last().first, "Lisa");
    ASSERT_EQ (vps3.back().second, "Simpson");

    ASSERT_THROW (vps3.at (2), std::out_of_range);
    ASSERT_THROW (vps3.at (3), std::out_of_range);

    ASSERT_EQ (capacity_old, vps3.capacity());

    vps3.shrink_to_fit();

    ASSERT_EQ (vps3.size(), 2u);
    ASSERT_EQ (vps3.at (0).first, "Bart");
    ASSERT_EQ (vps3.at (0).second, "Simpson");

    ASSERT_EQ (vps3.back().first, "Lisa");
    ASSERT_EQ (vps3.last().second, "Simpson");

    ASSERT_THROW (vps3.at (2), std::out_of_range);
    ASSERT_THROW (vps3.at (3), std::out_of_range);
    ASSERT_NE (capacity_old, vps3.capacity());

    std::stringstream s;
    s << vps3;
}

namespace
{
    /**
    * A simple wrapper for a resource.  The lifecycle is known, it will always
    * hold exactly one of the resource.
    */
    class ResourceHolderInt
    {
        /** Imagine a heavyweight object, for example a USB connection's resource handle */
        int m_resource;

    public:
        ResourceHolderInt()
        {
            m_resource = 1;
        }

        ResourceHolderInt (const ResourceHolderInt &other) :
            m_resource (other.m_resource)
        {
        }

        ResourceHolderInt &operator= (const ResourceHolderInt &rhs)
        {
            if (this != &rhs)
            {
                m_resource = rhs.m_resource;
            }
            return *this;
        }

        int value()
        {
            return m_resource;
        }

        ~ResourceHolderInt()
        {
            // call a cleanup function on the resource
            m_resource = 0;
        }
    };

    /**
    * A simple wrapper for a resource.  The lifecycle is known, it will always
    * hold exactly one of the resource.
    */
    class ResourceHolderPInt
    {
        /** Imagine a heavyweight object, for example a USB connection's resource handle */
        int *m_resource;

    public:
        ResourceHolderPInt()
        {
            m_resource = new int;
            *m_resource = 0;
        }

        ResourceHolderPInt (const ResourceHolderPInt &rhs)
        {
            m_resource = new int;
            *m_resource = *rhs.m_resource;
        }

        ResourceHolderPInt &operator= (const ResourceHolderPInt &rhs)
        {
            if (this != &rhs)
            {
                *m_resource = *rhs.m_resource;
            }
            return *this;
        }

        int value()
        {
            return *m_resource;
        }

        ~ResourceHolderPInt()
        {
            // call a cleanup function on the resource
            *m_resource = 0;

            // delete and clean up pointers
            delete m_resource;
            m_resource = nullptr;
        }
    };
}

TEST (TestVectorPair, TestVectorResourceHolderInt)
{
    Vector< ResourceHolderInt > royaleVector;
    royaleVector.resize (1);

    ASSERT_NO_THROW (royaleVector.pop_back());
    ASSERT_NO_THROW (royaleVector.clear());
}

TEST (TestVectorPair, TestVectorResourceHolderPInt)
{
    Vector< ResourceHolderPInt > royaleVector;
    royaleVector.resize (1);
    ASSERT_NO_THROW (royaleVector.clear());
}

TEST (TestVectorPair, TestStdVectorResourceHolder)
{
    std::vector< ResourceHolderPInt > royaleVector;
    royaleVector.resize (1);

    ASSERT_NO_THROW (royaleVector.pop_back());
    ASSERT_NO_THROW (royaleVector.clear());
}

TEST (TestVectorPair, TestVectorStringDestructor)
{
    Vector< std::string > royaleVector;
    royaleVector.resize (1);

    ASSERT_NO_THROW (royaleVector.pop_back());
    ASSERT_NO_THROW (royaleVector.clear());
}

TEST (TestVectorPair, TestVectorAllocation)
{
    Vector< std::string > royaleVector;
    ASSERT_NO_THROW (royaleVector.resize (1));
    ASSERT_NO_THROW (royaleVector.resize (5));

    ASSERT_NO_THROW (royaleVector.reserve (10));

    ASSERT_NO_THROW (royaleVector.pop_back());

    ASSERT_NO_THROW (royaleVector.clear());
    ASSERT_EQ (royaleVector.size(), 0u);
    ASSERT_EQ (royaleVector.capacity(), 0u);

    royaleVector.push_back ("Hello World");
    ASSERT_EQ (royaleVector.size(), 1u);
    ASSERT_EQ (royaleVector.capacity(), 1u);

    royaleVector.pop_back();
    ASSERT_EQ (royaleVector.capacity(), 0u);

    ASSERT_NO_THROW (royaleVector.reserve (10));
    ASSERT_EQ (royaleVector.capacity(), 10u);
    royaleVector.push_back ("Hello World #2");

    ASSERT_EQ (royaleVector.size(), 1u);
    ASSERT_EQ (royaleVector.capacity(), 10u);

    royaleVector.shrink_to_fit();
    ASSERT_EQ (royaleVector.size(), 1u);
    ASSERT_EQ (royaleVector.capacity(), 1u);

    royaleVector.pop_back();
    royaleVector.shrink_to_fit();
    ASSERT_EQ (royaleVector.size(), 0u);
    ASSERT_EQ (royaleVector.capacity(), 0u);

    royaleVector.resize (5, "Hello World");
    ASSERT_EQ (royaleVector.size(), 5u);
    ASSERT_EQ (royaleVector.capacity(), 5u);
    ASSERT_EQ (royaleVector.at (0), "Hello World");
    ASSERT_EQ (royaleVector.at (1), "Hello World");
    ASSERT_EQ (royaleVector.at (2), "Hello World");
    ASSERT_EQ (royaleVector.at (3), "Hello World");
    ASSERT_EQ (royaleVector.at (4), "Hello World");
    ASSERT_THROW (royaleVector.at (5), std::out_of_range);

    royaleVector.pop_back();
    ASSERT_EQ (royaleVector.size(), 4u);
    ASSERT_EQ (royaleVector.capacity(), 5u);
    royaleVector.pop_back();
    ASSERT_EQ (royaleVector.size(), 3u);
    ASSERT_EQ (royaleVector.capacity(), 5u);
    royaleVector.pop_back();
    ASSERT_EQ (royaleVector.size(), 2u);
    ASSERT_EQ (royaleVector.capacity(), 5u);
    royaleVector.pop_back();
    ASSERT_EQ (royaleVector.size(), 1u);
    ASSERT_EQ (royaleVector.capacity(), 5u);
    royaleVector.pop_back();
    ASSERT_EQ (royaleVector.size(), 0u);
    ASSERT_EQ (royaleVector.capacity(), 0u);
}

TEST (TestVectorPair, IntConversionFromStdVector)
{
    std::vector<uint32_t> stdIntVector;
    stdIntVector.push_back (1u);
    stdIntVector.push_back (2u);
    stdIntVector.push_back (3u);
    stdIntVector.push_back (4u);

    Vector<uint32_t> royaleIntVector (stdIntVector);
    ASSERT_EQ (royaleIntVector.at (0), 1u);
    ASSERT_EQ (royaleIntVector.at (1), 2u);
    ASSERT_EQ (royaleIntVector.at (2), 3u);
    ASSERT_EQ (royaleIntVector.at (3), 4u);

    stdIntVector.pop_back();
    stdIntVector.pop_back();
    ASSERT_THROW (stdIntVector.at (2), std::out_of_range);
    ASSERT_EQ (royaleIntVector.at (0), 1u);
    ASSERT_EQ (royaleIntVector.at (1), 2u);
    ASSERT_EQ (royaleIntVector.at (2), 3u);
    ASSERT_EQ (royaleIntVector.at (3), 4u);

    stdIntVector.clear();
    ASSERT_EQ (stdIntVector.size(), 0u);
    ASSERT_THROW (stdIntVector.at (0), std::out_of_range);
    ASSERT_EQ (royaleIntVector.at (0), 1u);
    ASSERT_EQ (royaleIntVector.at (1), 2u);
    ASSERT_EQ (royaleIntVector.at (2), 3u);
    ASSERT_EQ (royaleIntVector.at (3), 4u);

    stdIntVector = royaleIntVector.toStdVector();
    ASSERT_EQ (stdIntVector.at (0), 1u);
    ASSERT_EQ (stdIntVector.at (1), 2u);
    ASSERT_EQ (stdIntVector.at (2), 3u);
    ASSERT_EQ (stdIntVector.at (3), 4u);

    royaleIntVector.clear();
    ASSERT_THROW (royaleIntVector.at (0), std::out_of_range);
    ASSERT_EQ (royaleIntVector.size(), 0u);

    ASSERT_EQ (stdIntVector.at (0), 1u);
    ASSERT_EQ (stdIntVector.at (1), 2u);
    ASSERT_EQ (stdIntVector.at (2), 3u);
    ASSERT_EQ (stdIntVector.at (3), 4u);

    royaleIntVector = Vector<uint32_t>::fromStdVector (stdIntVector);
    Vector<uint32_t> copyRoyaleVector (stdIntVector);
    stdIntVector.clear();
    ASSERT_EQ (stdIntVector.size(), 0u);

    ASSERT_THROW (stdIntVector.at (0), std::out_of_range);
    ASSERT_EQ (royaleIntVector.at (0), 1u);
    ASSERT_EQ (royaleIntVector.at (1), 2u);
    ASSERT_EQ (royaleIntVector.at (2), 3u);
    ASSERT_EQ (royaleIntVector.at (3), 4u);

    ASSERT_EQ (royaleIntVector.at (0), copyRoyaleVector.at (0));
    ASSERT_EQ (royaleIntVector.at (1), copyRoyaleVector.at (1));
    ASSERT_EQ (royaleIntVector.at (2), copyRoyaleVector.at (2));
    ASSERT_EQ (royaleIntVector.at (3), copyRoyaleVector.at (3));
}

TEST (TestVectorPair, StringConversionFromStdVector)
{
    std::vector<std::string> stdStringVector;
    stdStringVector.push_back (std::string ("Hello World #1"));
    stdStringVector.push_back (std::string ("Hello World #2"));
    stdStringVector.push_back (std::string ("Hello World #3"));
    stdStringVector.push_back (std::string ("Hello World #4"));

    Vector<std::string> royaleStringVector (stdStringVector);
    ASSERT_EQ (royaleStringVector.at (0), std::string ("Hello World #1"));
    ASSERT_EQ (royaleStringVector.at (1), std::string ("Hello World #2"));
    ASSERT_EQ (royaleStringVector.at (2), std::string ("Hello World #3"));
    ASSERT_EQ (royaleStringVector.at (3), std::string ("Hello World #4"));

    stdStringVector.pop_back();
    stdStringVector.pop_back();
    ASSERT_THROW (stdStringVector.at (2), std::out_of_range);
    ASSERT_EQ (royaleStringVector.at (0), std::string ("Hello World #1"));
    ASSERT_EQ (royaleStringVector.at (1), std::string ("Hello World #2"));
    ASSERT_EQ (royaleStringVector.at (2), std::string ("Hello World #3"));
    ASSERT_EQ (royaleStringVector.at (3), std::string ("Hello World #4"));

    stdStringVector.clear();
    ASSERT_EQ (stdStringVector.size(), 0u);
    ASSERT_THROW (stdStringVector.at (0), std::out_of_range);
    ASSERT_EQ (royaleStringVector.at (0), std::string ("Hello World #1"));
    ASSERT_EQ (royaleStringVector.at (1), std::string ("Hello World #2"));
    ASSERT_EQ (royaleStringVector.at (2), std::string ("Hello World #3"));
    ASSERT_EQ (royaleStringVector.at (3), std::string ("Hello World #4"));

    stdStringVector = royaleStringVector.toStdVector();
    ASSERT_EQ (royaleStringVector.at (0), std::string ("Hello World #1"));
    ASSERT_EQ (royaleStringVector.at (1), std::string ("Hello World #2"));
    ASSERT_EQ (royaleStringVector.at (2), std::string ("Hello World #3"));
    ASSERT_EQ (royaleStringVector.at (3), std::string ("Hello World #4"));

    royaleStringVector.clear();
    ASSERT_THROW (royaleStringVector.at (0), std::out_of_range);
    ASSERT_EQ (royaleStringVector.size(), 0u);
    ASSERT_EQ (stdStringVector.at (0), std::string ("Hello World #1"));
    ASSERT_EQ (stdStringVector.at (1), std::string ("Hello World #2"));
    ASSERT_EQ (stdStringVector.at (2), std::string ("Hello World #3"));
    ASSERT_EQ (stdStringVector.at (3), std::string ("Hello World #4"));

    royaleStringVector = Vector<std::string>::fromStdVector (stdStringVector);
    Vector<std::string> copyRoyaleVector (stdStringVector);
    stdStringVector.clear();
    ASSERT_EQ (stdStringVector.size(), 0u);

    ASSERT_THROW (stdStringVector.at (0), std::out_of_range);
    ASSERT_EQ (royaleStringVector.at (0), std::string ("Hello World #1"));
    ASSERT_EQ (royaleStringVector.at (1), std::string ("Hello World #2"));
    ASSERT_EQ (royaleStringVector.at (2), std::string ("Hello World #3"));
    ASSERT_EQ (royaleStringVector.at (3), std::string ("Hello World #4"));

    ASSERT_EQ (royaleStringVector.at (0), copyRoyaleVector.at (0));
    ASSERT_EQ (royaleStringVector.at (1), copyRoyaleVector.at (1));
    ASSERT_EQ (royaleStringVector.at (2), copyRoyaleVector.at (2));
    ASSERT_EQ (royaleStringVector.at (3), copyRoyaleVector.at (3));
}

TEST (TestVectorPair, PairConversionFromStdVector)
{
    std::vector< std::pair<uint32_t, std::string> > stdPairVector;
    stdPairVector.push_back (std::make_pair (1u, std::string ("Hello World #1")));
    stdPairVector.push_back (std::make_pair (2u, std::string ("Hello World #2")));
    stdPairVector.push_back (std::make_pair (3u, std::string ("Hello World #3")));
    stdPairVector.push_back (std::make_pair (4u, std::string ("Hello World #4")));

    Vector< std::pair<uint32_t, std::string> > royalePairVector (stdPairVector);
    ASSERT_EQ (royalePairVector.at (0).first, 1u);
    ASSERT_EQ (royalePairVector.at (1).first, 2u);
    ASSERT_EQ (royalePairVector.at (2).first, 3u);
    ASSERT_EQ (royalePairVector.at (3).first, 4u);

    stdPairVector.pop_back();
    stdPairVector.pop_back();
    ASSERT_EQ (royalePairVector.at (0).second, std::string ("Hello World #1"));
    ASSERT_EQ (royalePairVector.at (1).second, std::string ("Hello World #2"));
    ASSERT_EQ (royalePairVector.at (2).second, std::string ("Hello World #3"));
    ASSERT_EQ (royalePairVector.at (3).second, std::string ("Hello World #4"));

    stdPairVector.clear();
    ASSERT_EQ (stdPairVector.size(), 0u);
    ASSERT_EQ (royalePairVector.at (0).second, std::string ("Hello World #1"));
    ASSERT_EQ (royalePairVector.at (1).second, std::string ("Hello World #2"));
    ASSERT_EQ (royalePairVector.at (2).second, std::string ("Hello World #3"));
    ASSERT_EQ (royalePairVector.at (3).second, std::string ("Hello World #4"));

    stdPairVector = royalePairVector.toStdVector();
    ASSERT_EQ (royalePairVector.at (0).second, std::string ("Hello World #1"));
    ASSERT_EQ (royalePairVector.at (1).second, std::string ("Hello World #2"));
    ASSERT_EQ (royalePairVector.at (2).second, std::string ("Hello World #3"));
    ASSERT_EQ (royalePairVector.at (3).second, std::string ("Hello World #4"));

    royalePairVector.clear();
    ASSERT_EQ (royalePairVector.size(), 0u);
    ASSERT_EQ (stdPairVector.at (0).second, std::string ("Hello World #1"));
    ASSERT_EQ (stdPairVector.at (1).second, std::string ("Hello World #2"));
    ASSERT_EQ (stdPairVector.at (2).second, std::string ("Hello World #3"));
    ASSERT_EQ (stdPairVector.at (3).second, std::string ("Hello World #4"));

    royalePairVector = Vector< std::pair<uint32_t, std::string> >::fromStdVector (stdPairVector);
    Vector< std::pair<uint32_t, std::string> > copyRoyaleVector (stdPairVector);
    stdPairVector.clear();
    ASSERT_EQ (stdPairVector.size(), 0u);

    ASSERT_EQ (royalePairVector.at (0).second, std::string ("Hello World #1"));
    ASSERT_EQ (royalePairVector.at (1).second, std::string ("Hello World #2"));
    ASSERT_EQ (royalePairVector.at (2).second, std::string ("Hello World #3"));
    ASSERT_EQ (royalePairVector.at (3).second, std::string ("Hello World #4"));

    ASSERT_EQ (royalePairVector.at (0), copyRoyaleVector.at (0));
    ASSERT_EQ (royalePairVector.at (1), copyRoyaleVector.at (1));
    ASSERT_EQ (royalePairVector.at (2), copyRoyaleVector.at (2));
    ASSERT_EQ (royalePairVector.at (3), copyRoyaleVector.at (3));
}

TEST (TestVectorPair, ConvertFromStdMap)
{
    std::map< uint32_t, std::string > stdMap;
    stdMap.emplace (1, std::string ("Hello World #1"));
    stdMap.emplace (2, std::string ("Hello World #2"));
    stdMap.emplace (3, std::string ("Hello World #3"));
    stdMap.emplace (4, std::string ("Hello World #4"));

    Vector< Pair<uint32_t, std::string> > royaleCopyVector (Vector< Pair<uint32_t, std::string> >::fromStdMap (stdMap));
    Vector< Pair<uint32_t, std::string> > royaleAssignmentVector = Vector< Pair<uint32_t, std::string> >::fromStdMap (stdMap);

    ASSERT_EQ (stdMap.size(), royaleCopyVector.size());
    ASSERT_EQ (stdMap.size(), royaleAssignmentVector.size());

    ASSERT_EQ (royaleCopyVector.at (0).second, stdMap.at (1));
    ASSERT_EQ (royaleCopyVector.at (1).second, stdMap.at (2));
    ASSERT_EQ (royaleCopyVector.at (2).second, stdMap.at (3));
    ASSERT_EQ (royaleCopyVector.at (3).second, stdMap.at (4));

    ASSERT_EQ (royaleAssignmentVector.at (0).second, stdMap.at (1));
    ASSERT_EQ (royaleAssignmentVector.at (1).second, stdMap.at (2));
    ASSERT_EQ (royaleAssignmentVector.at (2).second, stdMap.at (3));
    ASSERT_EQ (royaleAssignmentVector.at (3).second, stdMap.at (4));

    ASSERT_EQ (royaleAssignmentVector.at (0).second, royaleCopyVector.at (0).second);
    ASSERT_EQ (royaleAssignmentVector.at (1).second, royaleCopyVector.at (1).second);
    ASSERT_EQ (royaleAssignmentVector.at (2).second, royaleCopyVector.at (2).second);
    ASSERT_EQ (royaleAssignmentVector.at (3).second, royaleCopyVector.at (3).second);
}

TEST (TestVectorPair, ConvertToStdMap)
{
    Vector< Pair<uint32_t, std::string> > royaleVector;

    royaleVector.push_back (royale_pair (uint32_t (1), std::string ("Hello World #1")));
    royaleVector.push_back (royale_pair (uint32_t (2), std::string ("Hello World #2")));
    royaleVector.push_back (royale_pair (uint32_t (3), std::string ("Hello World #3")));
    royaleVector.push_back (royale_pair (uint32_t (4), std::string ("Hello World #4")));

    std::map< uint32_t, std::string > stdMap (royaleVector.toStdMap< uint32_t, std::string >());

    ASSERT_EQ (royaleVector.at (0).second, stdMap.at (1));
    ASSERT_EQ (royaleVector.at (1).second, stdMap.at (2));
    ASSERT_EQ (royaleVector.at (2).second, stdMap.at (3));
    ASSERT_EQ (royaleVector.at (3).second, stdMap.at (4));
}

TEST (TestVectorPair, ConvertFromStdPair)
{
    {
        std::pair<uint32_t, std::string> stdPair;
        stdPair.first = 30;
        stdPair.second = std::string ("Hallo Welt");

        Pair<uint32_t, std::string> royalePair (Pair<uint32_t, std::string>::fromStdPair (stdPair));
        ASSERT_EQ (royalePair.first, stdPair.first);
        ASSERT_EQ (royalePair.second, stdPair.second);

        ASSERT_EQ (stdPair, royalePair.toStdPair());
    }

    {
        Pair<uint32_t, std::string> royalePair;
        royalePair.first = 30;
        royalePair.second = std::string ("Hallo Welt");

        std::pair<uint32_t, std::string> stdPair (royalePair.toStdPair());
        ASSERT_EQ (royalePair.first, stdPair.first);
        ASSERT_EQ (royalePair.second, stdPair.second);

        Pair<uint32_t, std::string> royalePair2 = stdPair;
        ASSERT_EQ (royalePair2, stdPair);
        ASSERT_EQ (royalePair, royalePair2);

        Pair<uint32_t, std::string> royalePair3;
        royalePair3 = std::move (royalePair);
        ASSERT_EQ (royalePair2, royalePair3);
    }
}

TEST (TestVectorPair, EqualityCheckVectors)
{
    std::vector<std::string> stdStringVector (4);
    stdStringVector.push_back (std::string ("Hello World #1"));
    stdStringVector.push_back (std::string ("Hello World #2"));
    stdStringVector.push_back (std::string ("Hello World #3"));
    stdStringVector.push_back (std::string ("Hello World #4"));

    Vector<std::string> royaleStringVector (stdStringVector);
    ASSERT_EQ (royaleStringVector.at (4), std::string ("Hello World #1"));
    ASSERT_EQ (royaleStringVector.at (5), std::string ("Hello World #2"));
    ASSERT_EQ (royaleStringVector.at (6), std::string ("Hello World #3"));
    ASSERT_EQ (royaleStringVector.at (7), std::string ("Hello World #4"));

    Vector<std::string> royaleStringVector2 (stdStringVector);

    ASSERT_EQ (royaleStringVector, stdStringVector);
    ASSERT_EQ (royaleStringVector, royaleStringVector2);

    royaleStringVector2.pop_back();
    ASSERT_NE (royaleStringVector, royaleStringVector2);

    royaleStringVector2.push_back (std::string ("Hello World #4"));
    ASSERT_EQ (royaleStringVector, royaleStringVector2);

    royaleStringVector2.pop_back();
    ASSERT_NE (royaleStringVector, royaleStringVector2);

    royaleStringVector2.push_back (std::string ("Hello World #5"));
    ASSERT_NE (royaleStringVector, royaleStringVector2);

    royaleStringVector2.clear();
    ASSERT_NE (royaleStringVector, royaleStringVector2);

    ASSERT_EQ (royaleStringVector, stdStringVector);

    royaleStringVector.pop_back();
    ASSERT_NE (royaleStringVector, stdStringVector);

    royaleStringVector.push_back (std::string ("Hello World #4"));
    ASSERT_EQ (royaleStringVector, stdStringVector);

    royaleStringVector.pop_back();
    ASSERT_NE (royaleStringVector, stdStringVector);

    royaleStringVector.push_back (std::string ("Hello World #5"));
    ASSERT_NE (royaleStringVector, stdStringVector);

    royaleStringVector.clear();
    ASSERT_NE (royaleStringVector, stdStringVector);
}

TEST (TestVectorPair, EqualityCheckPairs)
{
    std::pair<uint32_t, std::string> stdPair (1, std::string ("Hello World"));
    Pair<uint32_t, std::string> royalePair (stdPair);
    Pair<uint32_t, std::string> royalePair2 (stdPair);
    ASSERT_EQ (royalePair, stdPair);
    ASSERT_EQ (royalePair, royalePair2);

    stdPair.first = 2;
    ASSERT_NE (royalePair, stdPair);
    ASSERT_EQ (royalePair, royalePair2);

    royalePair2.first = 2;
    ASSERT_EQ (royalePair2, stdPair);
    ASSERT_NE (royalePair, royalePair2);

    royalePair.first = 2;
    ASSERT_EQ (royalePair, royalePair2);
}

TEST (TestVectorPair, TestAssignmentFromConstStdVector)
{
    std::vector<std::string> stdStringVector;
    stdStringVector.push_back (std::string ("Hello World #1"));
    stdStringVector.push_back (std::string ("Hello World #2"));

    const std::vector<std::string> src (std::move (stdStringVector));

    Vector<std::string> dest;
    dest = src;

    ASSERT_EQ (src.size(), 2u);
    ASSERT_EQ (src.at (0), std::string ("Hello World #1"));
    ASSERT_EQ (src.at (1), std::string ("Hello World #2"));

    ASSERT_EQ (dest.size(), 2u);
    ASSERT_EQ (dest.at (0), std::string ("Hello World #1"));
    ASSERT_EQ (dest.at (1), std::string ("Hello World #2"));
}

TEST (TestVectorPair, TestAssignmentFromConst)
{
    std::vector<std::string> stdStringVector;
    stdStringVector.push_back (std::string ("Hello World #1"));
    stdStringVector.push_back (std::string ("Hello World #2"));

    const Vector<std::string> src (stdStringVector);

    ASSERT_EQ (src.at (0), std::string ("Hello World #1"));
    ASSERT_EQ (src.at (1), std::string ("Hello World #2"));

    Vector<std::string> dest;
    dest = src;

    ASSERT_EQ (src.size(), 2u);
    ASSERT_EQ (src.at (0), std::string ("Hello World #1"));
    ASSERT_EQ (src.at (1), std::string ("Hello World #2"));

    ASSERT_EQ (dest.size(), 2u);
    ASSERT_EQ (dest.at (0), std::string ("Hello World #1"));
    ASSERT_EQ (dest.at (1), std::string ("Hello World #2"));
}

TEST (TestVectorPair, TestMoveSemantics)
{
    std::vector<std::string> stdStringVector (4);
    stdStringVector.push_back (std::string ("Hello World #1"));
    stdStringVector.push_back (std::string ("Hello World #2"));
    stdStringVector.push_back (std::string ("Hello World #3"));
    stdStringVector.push_back (std::string ("Hello World #4"));

    Vector<std::string> royaleStringVector (stdStringVector);
    ASSERT_EQ (royaleStringVector.at (4), std::string ("Hello World #1"));
    ASSERT_EQ (royaleStringVector.at (5), std::string ("Hello World #2"));
    ASSERT_EQ (royaleStringVector.at (6), std::string ("Hello World #3"));
    ASSERT_EQ (royaleStringVector.at (7), std::string ("Hello World #4"));

    Vector<std::string> royaleStringVector2 (std::move (royaleStringVector));

    ASSERT_EQ (royaleStringVector2.at (4), std::string ("Hello World #1"));
    ASSERT_EQ (royaleStringVector2.at (5), std::string ("Hello World #2"));
    ASSERT_EQ (royaleStringVector2.at (6), std::string ("Hello World #3"));
    ASSERT_EQ (royaleStringVector2.at (7), std::string ("Hello World #4"));

    ASSERT_ANY_THROW (royaleStringVector.at (4));
    ASSERT_ANY_THROW (royaleStringVector.at (5));
    ASSERT_ANY_THROW (royaleStringVector.at (6));
    ASSERT_ANY_THROW (royaleStringVector.at (7));

    ASSERT_NO_THROW (royaleStringVector.clear());
    ASSERT_NO_THROW (royaleStringVector.clear());
    ASSERT_NO_THROW (royaleStringVector.clear());

    royaleStringVector = std::move (royaleStringVector2);

    ASSERT_ANY_THROW (royaleStringVector2.at (4));
    ASSERT_ANY_THROW (royaleStringVector2.at (5));
    ASSERT_ANY_THROW (royaleStringVector2.at (6));
    ASSERT_ANY_THROW (royaleStringVector2.at (7));

    ASSERT_NO_THROW (royaleStringVector2.clear());
    ASSERT_NO_THROW (royaleStringVector2.clear());
    ASSERT_NO_THROW (royaleStringVector2.clear());

    ASSERT_EQ (royaleStringVector.at (4), std::string ("Hello World #1"));
    ASSERT_EQ (royaleStringVector.at (5), std::string ("Hello World #2"));
    ASSERT_EQ (royaleStringVector.at (6), std::string ("Hello World #3"));
    ASSERT_EQ (royaleStringVector.at (7), std::string ("Hello World #4"));
}

TEST (TestVectorPair, TestIterators)
{
    Vector<std::string> royaleStringVector;

    ASSERT_NO_THROW (royaleStringVector.push_back ("Hello World #1"));
    ASSERT_NO_THROW (royaleStringVector.push_back ("Hello World #2"));
    ASSERT_NO_THROW (royaleStringVector.push_back ("Hello World #3"));
    ASSERT_NO_THROW (royaleStringVector.push_back ("Hello World #4"));

    {
        int count = 0;
        for (auto &i : royaleStringVector)
        {
            ASSERT_EQ (i, royaleStringVector.at (count));
            count++;
        }
    }

    for (auto royIter = royaleStringVector.begin(); royIter != royaleStringVector.end(); ++royIter)
    {
        auto diff = royIter - royaleStringVector.begin();
        String checkString = String ("Hello World #");
        checkString += String::fromAny<Vector<std::string>::iterator::difference_type> (diff + 1);
        ASSERT_EQ (checkString, *royIter);
    }

    {
        Vector<char> test = { 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
        royale_iterator<std::bidirectional_iterator_tag, char> myIter (test.data());
        myIter++;
        ++myIter;
        myIter--;
        --myIter;
        //std::cout << "myIter[2]: " << myIter[2] << std::endl;
    }

    {
        Vector<std::string> test = { "a", "b", "c", "d", "e", "f", "g" };
        royale_iterator<std::random_access_iterator_tag, std::string> myIter (test.data());
        myIter++->append ("x");
        for (Vector<std::string>::iterator spinningIter = myIter; spinningIter != test.end(); spinningIter++)
        {
            spinningIter->append ("x");
        }

        for (Vector<std::string>::iterator spinningIter = test.begin(); spinningIter != test.end(); spinningIter++)
        {
            ASSERT_EQ (spinningIter->at (spinningIter->size() - 1), 'x');
            ASSERT_EQ (spinningIter->back(), 'x');
        }
    }

    {
        // compile test
        //Vector<std::string>::iterator it0 (royaleStringVector.rbegin());                  // forward iterator may take reverse iterator
        //(void) it0;
        //Vector<std::string>::const_iterator it1 (royaleStringVector.rbegin());            // const forward iterator may take const reverse iterator
        //(void) it1;
        Vector<std::string>::const_iterator it2 (royaleStringVector.begin());             // const forward iterator may take const forward iterator
        (void) it2;
        Vector<std::string>::iterator it3 (royaleStringVector.begin());                   // forward iterator may take forward iterator
        (void) it3;
        Vector<std::string>::const_iterator it4 (royaleStringVector.cbegin());            // const forward iterator may take strict const forward iterator
        (void) it4;

        Vector<std::string>::reverse_iterator it5 (royaleStringVector.begin());           // reverse iterator may take forward one
        (void) it5;
        Vector<std::string>::const_reverse_iterator it6 (royaleStringVector.begin());     // const reverse iterator may take const forward one
        (void) it6;
        Vector<std::string>::const_reverse_iterator it7 (royaleStringVector.rbegin());    // const reverse iterator may take const reverse one
        (void) it7;
        Vector<std::string>::reverse_iterator it8 (royaleStringVector.rbegin());          // reverse iterator may take reverse iterator
        (void) it8;
        Vector<std::string>::const_reverse_iterator it9 (royaleStringVector.crbegin());   // const reverse iterator may take const reverse iterator
        (void) it9;

        ASSERT_EQ (it3 == it4, true);
        ASSERT_EQ (it5 != it8, true);

        ASSERT_EQ (it2 == it4, true);
        ASSERT_EQ (it8 == it9, true);

        ASSERT_EQ (it3 != it4, false);
        ASSERT_EQ (it5 == it8, false);

        ASSERT_EQ (it2 != it4, false);
        ASSERT_EQ (it8 != it9, false);
    }

    {
        Vector<std::string>::const_iterator it0 (royaleStringVector.cbegin());
        (void) it0;
        Vector<std::string>::const_iterator it1 (royaleStringVector.cend());
        (void) it1;
        Vector<std::string>::const_reverse_iterator it2 (royaleStringVector.crbegin());
        (void) it2;
        Vector<std::string>::const_reverse_iterator it3 (royaleStringVector.crend());
        (void) it3;
    }

    {
        // compile test
        //Vector<std::string>::iterator it0 = royaleStringVector.rbegin();                  // forward iterator may take reverse iterator
        //(void)it0;
        //Vector<std::string>::const_iterator it1 = royaleStringVector.rbegin();            // const forward iterator may take const reverse iterator
        //(void)it1;
        Vector<std::string>::const_iterator it2 = royaleStringVector.begin();             // const forward iterator may take const forward iterator
        (void) it2;
        Vector<std::string>::iterator it3 = royaleStringVector.begin();                   // forward iterator may take forward iterator
        (void) it3;
        Vector<std::string>::const_iterator it4 = royaleStringVector.cbegin();            // const forward iterator may take strict const forward iterator
        (void) it4;

        Vector<std::string>::reverse_iterator it5 = royaleStringVector.begin();           // reverse iterator may take forward one
        (void) it5;
        Vector<std::string>::const_reverse_iterator it6 = royaleStringVector.begin();     // const reverse iterator may take const forward one
        (void) it6;
        Vector<std::string>::const_reverse_iterator it7 = royaleStringVector.rbegin();    // const reverse iterator may take const reverse one
        (void) it7;
        Vector<std::string>::reverse_iterator it8 = royaleStringVector.rbegin();          // reverse iterator may take reverse iterator
        (void) it8;
        Vector<std::string>::const_reverse_iterator it9 = royaleStringVector.crbegin();   // const reverse iterator may take const reverse iterator
        (void) it9;
    }

    {
        Vector<std::string>::const_iterator it0 = royaleStringVector.cbegin();
        (void) it0;
        Vector<std::string>::const_iterator it1 = royaleStringVector.cend();
        (void) it1;
        Vector<std::string>::const_reverse_iterator it2 = royaleStringVector.crbegin();
        (void) it2;
        Vector<std::string>::const_reverse_iterator it3 = royaleStringVector.crend();
        (void) it3;
    }

    for (Vector<std::string>::iterator i = royaleStringVector.begin(); i != royaleStringVector.end(); i++)
    {
        auto diff = i - royaleStringVector.begin();
        String checkString = String ("Hello World #");
        checkString += String::fromAny<Vector<std::string>::iterator::difference_type> (diff + 1);
        ASSERT_EQ (checkString, *i);
    }

    int count = 0;
    for (Vector<std::string>::iterator i = royaleStringVector.begin(); i != royaleStringVector.end(); ++i)
    {
        *i = "The World is out #" + String::fromAny<Vector<std::string>::iterator::difference_type> (count).toStdString();
        count++;
    }

    for (Vector<std::string>::reverse_iterator i = royaleStringVector.rbegin(); i != royaleStringVector.rend(); ++i)
    {
        auto diff = i - royaleStringVector.rend();
        String checkString = String ("The World is out #");
        checkString += String::fromAny<Vector<std::string>::iterator::difference_type> (diff - 1);
        ASSERT_EQ (checkString, *i);
    }

    ASSERT_EQ (royaleStringVector.at (0), std::string ("The World is out #0"));
    ASSERT_EQ (royaleStringVector.at (1), std::string ("The World is out #1"));
    ASSERT_EQ (royaleStringVector.at (2), std::string ("The World is out #2"));
    ASSERT_EQ (royaleStringVector.at (3), std::string ("The World is out #3"));

    {
        Vector<String> royaleStringVectorWRString;
        for (auto &i : royaleStringVector)
        {
            royaleStringVectorWRString.push_back (i);
        }

        for (auto &i : royaleStringVectorWRString)
        {
            String::const_reverse_iterator it (i.rbegin());
            int steps = 0;
            for (; it != i.rend(); ++it)
            {
                steps++;
                if (steps == 7)
                {
                    break;
                }
            }

            String::iterator itInserter (i.begin());
            for (; itInserter != it; ++itInserter)
            {
                // No op here
            }

            String insertString (" mine");
            std::copy (insertString.begin(), insertString.end(), String::iterator (i.end() - 7));

        }

        ASSERT_EQ (royaleStringVectorWRString.at (0), std::string ("The World is mine#0"));
        ASSERT_EQ (royaleStringVectorWRString.at (1), std::string ("The World is mine#1"));
        ASSERT_EQ (royaleStringVectorWRString.at (2), std::string ("The World is mine#2"));
        ASSERT_EQ (royaleStringVectorWRString.at (3), std::string ("The World is mine#3"));

        Vector<String>::iterator changeIt = royaleStringVectorWRString.begin() + 2;
        *changeIt++ = "Test";
        *changeIt = "Test2";

        ASSERT_EQ (royaleStringVectorWRString.at (0), std::string ("The World is mine#0"));
        ASSERT_EQ (royaleStringVectorWRString.at (1), std::string ("The World is mine#1"));
        ASSERT_EQ (royaleStringVectorWRString.at (2), std::string ("Test"));
        ASSERT_EQ (royaleStringVectorWRString.at (3), std::string ("Test2"));
    }

    {
        Vector<std::string>::reverse_iterator it0 (royaleStringVector.rbegin());
        Vector<std::string>::const_reverse_iterator it1 (royaleStringVector.rbegin());
        ASSERT_LE (it0, it1);
        ASSERT_GE (it0, it1);
        ASSERT_EQ (it0, it1);
        it1++;
        ASSERT_LE (it1, it0);
        ASSERT_LT (it1, it0);
        ASSERT_GT (it0, it1);
        ASSERT_GE (it0, it1);
        ASSERT_NE (it0, it1);


        Vector<std::string>::const_iterator it2 (royaleStringVector.begin());             // const forward iterator may take const forward iterator
        Vector<std::string>::iterator it3 (royaleStringVector.begin());                   // forward iterator may take forward iterator
        ASSERT_LE (it2, it3);
        ASSERT_GE (it2, it3);
        ASSERT_EQ (it2, it3);
        it3--;
        ASSERT_LE (it3, it2);
        ASSERT_LT (it3, it2);
        ASSERT_GT (it2, it3);
        ASSERT_GE (it2, it3);
        ASSERT_NE (it2, it3);
        it3++;
        ASSERT_LE (it2, it3);
        ASSERT_GE (it2, it3);
        ASSERT_EQ (it2, it3);
        it2++;
        ASSERT_LE (it3, it2);
        ASSERT_LT (it3, it2);
        ASSERT_GT (it2, it3);
        ASSERT_GE (it2, it3);
        ASSERT_NE (it2, it3);
    }

    {
        Vector<int> myvector;
        for (int i = 0; i < 10; i++)
        {
            myvector.push_back (i);
        }

        typedef Vector<int>::iterator iter_type;

        royale_reverse_iterator<iter_type> rev_end (myvector.begin());
        royale_reverse_iterator<iter_type> rev_begin (myvector.end());

        int i = 0;
        for (iter_type it = rev_end.base(); it != rev_begin.base(); ++it, i++)
        {
            ASSERT_EQ (*it, i);
        }
    }

    {
        Vector<int> myvector;
        for (int i = 0; i < 10; i++)
        {
            myvector.push_back (i);
        }

        typedef Vector<int>::iterator iter_type;

        iter_type end (myvector.end());
        iter_type begin (myvector.begin());

        int i = 0;
        for (iter_type it = begin; it != end; ++it, i++)
        {
            ASSERT_EQ (i, *it);
        }
    }

    {
        Vector<int> myvector;
        for (int i = 0; i < 10; i++)
        {
            myvector.push_back (i);
        }

        typedef Vector<int>::iterator iter_type;

        royale_reverse_iterator<iter_type> rbegin (myvector.rbegin());
        royale_reverse_iterator<iter_type> rend (myvector.rend());

        int i = 0;

        for (royale_reverse_iterator<iter_type> it = rbegin; it != rend; ++it, ++i)
        {
            ASSERT_EQ (*it, * (it.base()));
        }
    }
}

TEST (TestVectorPair, TestConversions)
{
    Vector<uint8_t> data;
    data.push_back (1);
    data.push_back (2);
    data.push_back (3);
    data.push_back (4);

    auto func = [] (const Vector<uint8_t> &data)
    {
        std::vector<uint8_t> stdData = data.toStdVector (data);

        String fromVector;
        for (auto &i : stdData)
        {
            fromVector += String::fromUInt (i);
        }
        ASSERT_EQ (fromVector, "1234");
    };
    func (data);

    const Vector<uint8_t> dataNew ({ 1, 2, 3, 4 });
    ASSERT_EQ (dataNew.toStdVector().at (0), 1u);
    ASSERT_EQ (dataNew.toStdVector().at (1), 2u);
    ASSERT_EQ (dataNew.toStdVector().at (2), 3u);
    ASSERT_EQ (dataNew.toStdVector().at (3), 4u);
}

TEST (TestVectorPair, TestAssignmentCapacity)
{
    std::vector<uint8_t> stdVec1 (123);
    ASSERT_EQ (stdVec1.capacity(), 123u);
    ASSERT_EQ (stdVec1.size(), 123u);
    std::vector<uint8_t> stdVec2;
    ASSERT_EQ (stdVec2.capacity(), 0u);
    ASSERT_EQ (stdVec2.size(), 0u);
    stdVec2 = stdVec1;
    ASSERT_EQ (stdVec1.capacity(), stdVec2.capacity());
    ASSERT_EQ (stdVec1.size(), stdVec2.size());

    Vector<uint8_t> royVec1 (123);
    ASSERT_EQ (royVec1.capacity(), 123u);
    ASSERT_EQ (royVec1.size(), 123u);
    Vector<uint8_t> royVec2;
    ASSERT_EQ (royVec2.capacity(), 0u);
    ASSERT_EQ (royVec2.size(), 0u);
    royVec2 = royVec1;
    ASSERT_EQ (royVec1.capacity(), royVec2.capacity());
    ASSERT_EQ (royVec1.size(), royVec2.size());

    {
        Vector<uint8_t> royVec1 (123);
        ASSERT_EQ (royVec1.capacity(), 123u);
        ASSERT_EQ (royVec1.size(), 123u);
        std::vector<uint8_t> stdVec1;
        ASSERT_EQ (stdVec1.capacity(), 0u);
        ASSERT_EQ (stdVec1.size(), 0u);
        stdVec1 = royVec1.toStdVector();
        ASSERT_EQ (royVec1.capacity(), stdVec1.capacity());
        ASSERT_EQ (royVec1.size(), stdVec1.size());
    }

    {
        uint8_t array[] = { 1, 2, 3 };
        Vector<uint8_t> data (123);
        memcpy (&data[0], array, sizeof (array));
        std::vector<uint8_t> tmp = data.toStdVector();

        ASSERT_EQ (tmp.at (0), 1u);
        ASSERT_EQ (tmp.at (1), 2u);
        ASSERT_EQ (tmp.at (2), 3u);
    }
}

TEST (TestVectorPair, TestVectorInsert)
{
    {
        // no realloc
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("9"));
        ASSERT_NO_THROW (royVec.push_back ("10"));

        Vector<std::string> royVec2;
        ASSERT_NO_THROW (royVec2.push_back ("5"));
        ASSERT_NO_THROW (royVec2.push_back ("6"));
        ASSERT_NO_THROW (royVec2.push_back ("7"));
        ASSERT_NO_THROW (royVec2.push_back ("8"));

        royVec.insert (royVec.end() - 2, royVec2.begin(), royVec2.end());

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");

        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");
        ASSERT_EQ (royVec.at (6), "7");
        ASSERT_EQ (royVec.at (7), "8");

        ASSERT_EQ (royVec.at (8), "9");
        ASSERT_EQ (royVec.at (9), "10");
    }

    {
        Vector<std::string> royVec;
        royVec.reserve (6);
        ASSERT_NO_THROW (royVec.push_back ("Hello World #1"));
        ASSERT_NO_THROW (royVec.push_back ("Hello World #2"));
        ASSERT_NO_THROW (royVec.push_back ("Hello World #3"));
        ASSERT_NO_THROW (royVec.push_back ("Hello World #4"));
        ASSERT_NO_THROW (royVec.push_back ("Hello World #1"));
        ASSERT_NO_THROW (royVec.push_back ("Hello World #4"));

        Vector<std::string> royVec2;
        ASSERT_NO_THROW (royVec2.push_back ("Hello World #2"));
        ASSERT_NO_THROW (royVec2.push_back ("Hello World #3"));

        Vector<std::string> secondRoyVec (royVec2);

        royVec.insert (royVec.end() - 1, secondRoyVec.begin(), secondRoyVec.end());

        ASSERT_EQ (royVec.at (0), "Hello World #1");
        ASSERT_EQ (royVec.at (1), "Hello World #2");
        ASSERT_EQ (royVec.at (2), "Hello World #3");
        ASSERT_EQ (royVec.at (3), "Hello World #4");

        ASSERT_EQ (royVec.at (4), "Hello World #1");
        ASSERT_EQ (royVec.at (5), "Hello World #2");
        ASSERT_EQ (royVec.at (6), "Hello World #3");
        ASSERT_EQ (royVec.at (7), "Hello World #4");
    }

    // insert with new allocation
    // replace with buffer reallocation
    {
        Vector<std::string> royVec;
        royVec.reserve (6);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        Vector<std::string> royVec2;
        royVec2.reserve (6);

        ASSERT_NO_THROW (royVec2.push_back ("X"));
        ASSERT_NO_THROW (royVec2.push_back ("X"));
        ASSERT_NO_THROW (royVec2.push_back ("X"));
        ASSERT_NO_THROW (royVec2.push_back ("X"));
        ASSERT_NO_THROW (royVec2.push_back ("X"));
        ASSERT_NO_THROW (royVec2.push_back ("X"));

        royVec.insert (royVec.end() - 2, royVec2.begin(), royVec2.begin() + 5);

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "X");
        ASSERT_EQ (royVec.at (5), "X");
        ASSERT_EQ (royVec.at (6), "X");
        ASSERT_EQ (royVec.at (7), "X");
        ASSERT_EQ (royVec.at (8), "X");
        ASSERT_EQ (royVec.at (9), "5");
        ASSERT_EQ (royVec.at (10), "6");

        ASSERT_EQ (royVec.size(), 11u);
        ASSERT_NE (royVec.capacity(), 6u);
    }

    {
        // Realloc neccessary
        Vector<std::string> royVec;
        ASSERT_NO_THROW (royVec.push_back ("Hello World #1"));
        ASSERT_NO_THROW (royVec.push_back ("Hello World #4"));

        Vector<std::string> firstRoyVec (royVec);

        Vector<std::string> royVec2;
        ASSERT_NO_THROW (royVec2.push_back ("Hello World #2"));
        ASSERT_NO_THROW (royVec2.push_back ("Hello World #3"));

        Vector<std::string> secondRoyVec (royVec2);

        size_t oldCap = royVec.capacity();

        royVec.insert (royVec.begin() + 1, royVec2.begin(), royVec2.end());

        ASSERT_NE (oldCap, royVec.capacity());

        ASSERT_EQ (royVec.at (0), "Hello World #1");
        ASSERT_EQ (royVec.at (1), "Hello World #2");
        ASSERT_EQ (royVec.at (2), "Hello World #3");
        ASSERT_EQ (royVec.at (3), "Hello World #4");

        royVec.reserve (20);

        royVec.insert (royVec.end(), firstRoyVec.begin(), firstRoyVec.end());

        ASSERT_EQ (royVec.at (0), "Hello World #1");
        ASSERT_EQ (royVec.at (1), "Hello World #2");
        ASSERT_EQ (royVec.at (2), "Hello World #3");
        ASSERT_EQ (royVec.at (3), "Hello World #4");
        ASSERT_EQ (royVec.at (4), "Hello World #1");
        ASSERT_EQ (royVec.at (5), "Hello World #4");

        royVec.insert (royVec.end() - 1, secondRoyVec.begin(), secondRoyVec.end());

        ASSERT_EQ (royVec.at (0), "Hello World #1");
        ASSERT_EQ (royVec.at (1), "Hello World #2");
        ASSERT_EQ (royVec.at (2), "Hello World #3");
        ASSERT_EQ (royVec.at (3), "Hello World #4");
        ASSERT_EQ (royVec.at (4), "Hello World #1");
        ASSERT_EQ (royVec.at (5), "Hello World #2");
        ASSERT_EQ (royVec.at (6), "Hello World #3");
        ASSERT_EQ (royVec.at (7), "Hello World #4");

        royVec.insert (royVec.begin(), firstRoyVec.rbegin(), firstRoyVec.rend());

        ASSERT_EQ (royVec.at (0), "Hello World #4");
        ASSERT_EQ (royVec.at (1), "Hello World #1");

        ASSERT_EQ (royVec.at (2), "Hello World #1");
        ASSERT_EQ (royVec.at (3), "Hello World #2");
        ASSERT_EQ (royVec.at (4), "Hello World #3");
        ASSERT_EQ (royVec.at (5), "Hello World #4");
        ASSERT_EQ (royVec.at (6), "Hello World #1");
        ASSERT_EQ (royVec.at (7), "Hello World #2");
        ASSERT_EQ (royVec.at (8), "Hello World #3");
        ASSERT_EQ (royVec.at (9), "Hello World #4");

        royVec.insert (royVec.begin() + 1, secondRoyVec.rbegin(), secondRoyVec.rend());

        // Added reverse order at begin
        ASSERT_EQ (royVec.at (0), "Hello World #4");
        ASSERT_EQ (royVec.at (1), "Hello World #3");
        ASSERT_EQ (royVec.at (2), "Hello World #2");
        ASSERT_EQ (royVec.at (3), "Hello World #1");

        ASSERT_EQ (royVec.at (4), "Hello World #1");
        ASSERT_EQ (royVec.at (5), "Hello World #2");
        ASSERT_EQ (royVec.at (6), "Hello World #3");
        ASSERT_EQ (royVec.at (7), "Hello World #4");
        ASSERT_EQ (royVec.at (8), "Hello World #1");
        ASSERT_EQ (royVec.at (9), "Hello World #2");
        ASSERT_EQ (royVec.at (10), "Hello World #3");
        ASSERT_EQ (royVec.at (11), "Hello World #4");
    }

    // Test with indicies
    {
        {
            Vector<std::string> royVec;
            royVec.reserve (12);

            ASSERT_NO_THROW (royVec.push_back ("1"));
            ASSERT_NO_THROW (royVec.push_back ("2"));
            ASSERT_NO_THROW (royVec.push_back ("3"));
            ASSERT_NO_THROW (royVec.push_back ("4"));
            ASSERT_NO_THROW (royVec.push_back ("9"));
            ASSERT_NO_THROW (royVec.push_back ("10"));

            Vector<std::string> royVec2;
            ASSERT_NO_THROW (royVec2.push_back ("5"));
            ASSERT_NO_THROW (royVec2.push_back ("6"));
            ASSERT_NO_THROW (royVec2.push_back ("7"));
            ASSERT_NO_THROW (royVec2.push_back ("8"));

            royVec.insert (royVec.size() - 2, royVec2);

            ASSERT_EQ (royVec.at (0), "1");
            ASSERT_EQ (royVec.at (1), "2");
            ASSERT_EQ (royVec.at (2), "3");
            ASSERT_EQ (royVec.at (3), "4");

            ASSERT_EQ (royVec.at (4), "5");
            ASSERT_EQ (royVec.at (5), "6");
            ASSERT_EQ (royVec.at (6), "7");
            ASSERT_EQ (royVec.at (7), "8");

            ASSERT_EQ (royVec.at (8), "9");
            ASSERT_EQ (royVec.at (9), "10");

            royVec.insert (royVec.size(), "11");
            ASSERT_EQ (royVec.at (10), "11");
            ASSERT_EQ (royVec.size(), 11u);
        }

        {
            Vector<std::string> royVec;
            royVec.reserve (12);

            ASSERT_NO_THROW (royVec.push_back ("1"));
            ASSERT_NO_THROW (royVec.push_back ("2"));
            ASSERT_NO_THROW (royVec.push_back ("3"));
            ASSERT_NO_THROW (royVec.push_back ("4"));
            ASSERT_NO_THROW (royVec.push_back ("9"));
            ASSERT_NO_THROW (royVec.push_back ("10"));

            Vector<std::string> royVec2;
            ASSERT_NO_THROW (royVec2.push_back ("5"));
            ASSERT_NO_THROW (royVec2.push_back ("6"));
            ASSERT_NO_THROW (royVec2.push_back ("7"));
            ASSERT_NO_THROW (royVec2.push_back ("8"));

            royVec.insert (royVec.size() - 2, royVec2.begin(), royVec2.end());

            ASSERT_EQ (royVec.at (0), "1");
            ASSERT_EQ (royVec.at (1), "2");
            ASSERT_EQ (royVec.at (2), "3");
            ASSERT_EQ (royVec.at (3), "4");

            ASSERT_EQ (royVec.at (4), "5");
            ASSERT_EQ (royVec.at (5), "6");
            ASSERT_EQ (royVec.at (6), "7");
            ASSERT_EQ (royVec.at (7), "8");

            ASSERT_EQ (royVec.at (8), "9");
            ASSERT_EQ (royVec.at (9), "10");

            royVec.insert (royVec.size(), "11");
            ASSERT_EQ (royVec.at (10), "11");
            ASSERT_EQ (royVec.size(), 11u);
        }

        {
            {
                Vector<std::string> royVec;
                royVec.reserve (10);

                ASSERT_NO_THROW (royVec.push_back ("1"));
                ASSERT_NO_THROW (royVec.push_back ("2"));
                ASSERT_NO_THROW (royVec.push_back ("3"));
                ASSERT_NO_THROW (royVec.push_back ("4"));
                ASSERT_NO_THROW (royVec.push_back ("9"));
                ASSERT_NO_THROW (royVec.push_back ("10"));

                Vector<std::string> royVec2;
                ASSERT_NO_THROW (royVec2.push_back ("7"));
                ASSERT_NO_THROW (royVec2.push_back ("8"));

                Vector<std::string>::iterator a = royVec.insert (royVec.end() - 2, royVec2);
                royVec.insert (a, { "5", "6" });

                ASSERT_EQ (royVec.at (0), "1");
                ASSERT_EQ (royVec.at (1), "2");
                ASSERT_EQ (royVec.at (2), "3");
                ASSERT_EQ (royVec.at (3), "4");

                ASSERT_EQ (royVec.at (4), "5");
                ASSERT_EQ (royVec.at (5), "6");
                ASSERT_EQ (royVec.at (6), "7");
                ASSERT_EQ (royVec.at (7), "8");

                ASSERT_EQ (royVec.at (8), "9");
                ASSERT_EQ (royVec.at (9), "10");

                royVec.insert (royVec.size(), "11");
                ASSERT_EQ (royVec.at (10), "11");
                ASSERT_EQ (royVec.size(), 11u);
            }

            {
                Vector<std::string> royVec;
                royVec.reserve (6);

                ASSERT_NO_THROW (royVec.push_back ("1"));
                ASSERT_NO_THROW (royVec.push_back ("2"));
                ASSERT_NO_THROW (royVec.push_back ("3"));
                ASSERT_NO_THROW (royVec.push_back ("4"));
                ASSERT_NO_THROW (royVec.push_back ("9"));
                ASSERT_NO_THROW (royVec.push_back ("10"));

                Vector<std::string> royVec2;
                ASSERT_NO_THROW (royVec2.push_back ("7"));
                ASSERT_NO_THROW (royVec2.push_back ("8"));

                royVec.insert (royVec.insert (royVec.end() - 2, royVec2), { "5", "6" });

                ASSERT_EQ (royVec.at (0), "1");
                ASSERT_EQ (royVec.at (1), "2");
                ASSERT_EQ (royVec.at (2), "3");
                ASSERT_EQ (royVec.at (3), "4");

                ASSERT_EQ (royVec.at (4), "5");
                ASSERT_EQ (royVec.at (5), "6");
                ASSERT_EQ (royVec.at (6), "7");
                ASSERT_EQ (royVec.at (7), "8");

                ASSERT_EQ (royVec.at (8), "9");
                ASSERT_EQ (royVec.at (9), "10");

                royVec.insert (royVec.size(), "11");
                ASSERT_EQ (royVec.at (10), "11");
                ASSERT_EQ (royVec.size(), 11u);
            }
        }

        {
            {
                Vector<std::string> royVec;
                royVec.reserve (10);

                ASSERT_NO_THROW (royVec.push_back ("1"));
                ASSERT_NO_THROW (royVec.push_back ("2"));
                ASSERT_NO_THROW (royVec.push_back ("3"));
                ASSERT_NO_THROW (royVec.push_back ("4"));
                ASSERT_NO_THROW (royVec.push_back ("9"));
                ASSERT_NO_THROW (royVec.push_back ("10"));

                std::vector<std::string> royVec2;
                ASSERT_NO_THROW (royVec2.push_back ("7"));
                ASSERT_NO_THROW (royVec2.push_back ("8"));

                royVec.insert (royVec.insert (royVec.end() - 2, royVec2), { "5", "6" });

                ASSERT_EQ (royVec.at (0), "1");
                ASSERT_EQ (royVec.at (1), "2");
                ASSERT_EQ (royVec.at (2), "3");
                ASSERT_EQ (royVec.at (3), "4");

                ASSERT_EQ (royVec.at (4), "5");
                ASSERT_EQ (royVec.at (5), "6");
                ASSERT_EQ (royVec.at (6), "7");
                ASSERT_EQ (royVec.at (7), "8");

                ASSERT_EQ (royVec.at (8), "9");
                ASSERT_EQ (royVec.at (9), "10");

                royVec.insert (royVec.size(), "11");
                ASSERT_EQ (royVec.at (10), "11");
                ASSERT_EQ (royVec.size(), 11u);
            }

            {
                Vector<std::string> royVec;
                royVec.reserve (6);

                ASSERT_NO_THROW (royVec.push_back ("1"));
                ASSERT_NO_THROW (royVec.push_back ("2"));
                ASSERT_NO_THROW (royVec.push_back ("3"));
                ASSERT_NO_THROW (royVec.push_back ("4"));
                ASSERT_NO_THROW (royVec.push_back ("9"));
                ASSERT_NO_THROW (royVec.push_back ("10"));

                std::vector<std::string> royVec2;
                ASSERT_NO_THROW (royVec2.push_back ("7"));
                ASSERT_NO_THROW (royVec2.push_back ("8"));

                royVec.insert (royVec.insert (royVec.end() - 2, royVec2), { "5", "6" });

                ASSERT_EQ (royVec.at (0), "1");
                ASSERT_EQ (royVec.at (1), "2");
                ASSERT_EQ (royVec.at (2), "3");
                ASSERT_EQ (royVec.at (3), "4");

                ASSERT_EQ (royVec.at (4), "5");
                ASSERT_EQ (royVec.at (5), "6");
                ASSERT_EQ (royVec.at (6), "7");
                ASSERT_EQ (royVec.at (7), "8");

                ASSERT_EQ (royVec.at (8), "9");
                ASSERT_EQ (royVec.at (9), "10");

                royVec.insert (royVec.size(), "11");
                ASSERT_EQ (royVec.at (10), "11");
                ASSERT_EQ (royVec.size(), 11u);
            }
        }
    }

    {
        Vector<std::string> royVec;
        royVec.reserve (6);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        royVec.insert (royVec.begin() + 3, 4, "X");

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");

        ASSERT_EQ (royVec.at (3), "X");
        ASSERT_EQ (royVec.at (4), "X");
        ASSERT_EQ (royVec.at (5), "X");
        ASSERT_EQ (royVec.at (6), "X");

        ASSERT_EQ (royVec.at (7), "4");
        ASSERT_EQ (royVec.at (8), "5");
        ASSERT_EQ (royVec.at (9), "6");
    }
}

TEST (TestVectorPair, TestVectorEmplace)
{
    // emplace_back
    {
        Vector<Pair<std::string, std::string>> royVec;
        royVec.push_back (Pair<std::string, std::string> (std::string ("1: first"), std::string ("1: second")));
        royVec.push_back (Pair<std::string, std::string> (std::string ("2: first"), std::string ("2: second")));
        royVec.push_back (Pair<std::string, std::string> (std::string ("3: first"), std::string ("3: second")));
        royVec.push_back (Pair<std::string, std::string> (std::string ("4: first"), std::string ("4: second")));
        royVec.emplace_back (std::string ("5: first"), std::string ("5: second"));

        ASSERT_EQ (royVec.at (royVec.size() - 1).first, "5: first");
        ASSERT_EQ (royVec.at (royVec.size() - 1).second, "5: second");
    }

    // emplace
    {
        Vector<Pair<std::string, std::string>> royVec;
        royVec.push_back (Pair<std::string, std::string> (std::string ("1: first"), std::string ("1: second")));
        royVec.push_back (Pair<std::string, std::string> (std::string ("3: first"), std::string ("3: second")));
        royVec.push_back (Pair<std::string, std::string> (std::string ("4: first"), std::string ("4: second")));
        royVec.emplace (royVec.begin() + 1, std::string ("2: first"), std::string ("2: second"));

        ASSERT_EQ (royVec.at (1).first, "2: first");
        ASSERT_EQ (royVec.at (1).second, "2: second");
    }
}

TEST (TestVectorPair, TestVectorReplace)
{
    // iterator driven
    {
        // replace without buffer overrun / new allocation
        {
            Vector<std::string> royVec;
            royVec.reserve (10);

            ASSERT_NO_THROW (royVec.push_back ("1"));
            ASSERT_NO_THROW (royVec.push_back ("2"));
            ASSERT_NO_THROW (royVec.push_back ("3"));
            ASSERT_NO_THROW (royVec.push_back ("4"));
            ASSERT_NO_THROW (royVec.push_back ("5"));
            ASSERT_NO_THROW (royVec.push_back ("6"));

            Vector<std::string> royVec2;
            royVec2.reserve (6);

            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));

            royVec.replace (royVec.end() - 2, royVec2.begin(), royVec2.begin() + 5);

            ASSERT_EQ (royVec.at (0), "1");
            ASSERT_EQ (royVec.at (1), "2");
            ASSERT_EQ (royVec.at (2), "3");
            ASSERT_EQ (royVec.at (3), "4");
            ASSERT_EQ (royVec.at (4), "X");
            ASSERT_EQ (royVec.at (5), "X");
            ASSERT_EQ (royVec.at (6), "X");
            ASSERT_EQ (royVec.at (7), "X");
            ASSERT_EQ (royVec.at (8), "X");

            ASSERT_EQ (royVec.size(), 9u);
            ASSERT_EQ (royVec.capacity(), 10u);
        }

        {
            Vector<std::string> royVec;
            royVec.reserve (10);

            ASSERT_NO_THROW (royVec.push_back ("1"));
            ASSERT_NO_THROW (royVec.push_back ("2"));
            ASSERT_NO_THROW (royVec.push_back ("3"));
            ASSERT_NO_THROW (royVec.push_back ("4"));
            ASSERT_NO_THROW (royVec.push_back ("5"));
            ASSERT_NO_THROW (royVec.push_back ("6"));

            Vector<std::string> royVec2;
            royVec2.reserve (5);

            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));

            royVec.replace (royVec.begin(), royVec.end() - 2, royVec2);

            ASSERT_EQ (royVec.at (0), "X");
            ASSERT_EQ (royVec.at (1), "X");
            ASSERT_EQ (royVec.at (2), "X");
            ASSERT_EQ (royVec.at (3), "X");
            ASSERT_EQ (royVec.at (4), "5");
            ASSERT_EQ (royVec.at (5), "6");

            ASSERT_EQ (royVec.size(), 6u);
            ASSERT_EQ (royVec.capacity(), 10u);
        }

        // replace without allocation; in the middle
        {
            Vector<std::string> royVec;
            royVec.reserve (6);

            ASSERT_NO_THROW (royVec.push_back ("1"));
            ASSERT_NO_THROW (royVec.push_back ("2"));
            ASSERT_NO_THROW (royVec.push_back ("3"));
            ASSERT_NO_THROW (royVec.push_back ("4"));
            ASSERT_NO_THROW (royVec.push_back ("5"));
            ASSERT_NO_THROW (royVec.push_back ("6"));

            Vector<std::string> royVec2;
            royVec2.reserve (6);

            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));

            royVec.replace (royVec.begin() + 2, royVec2.begin(), royVec2.begin() + 2);

            ASSERT_EQ (royVec.at (0), "1");
            ASSERT_EQ (royVec.at (1), "2");
            ASSERT_EQ (royVec.at (2), "X");
            ASSERT_EQ (royVec.at (3), "X");
            ASSERT_EQ (royVec.at (4), "5");
            ASSERT_EQ (royVec.at (5), "6");

            ASSERT_EQ (royVec.size(), 6u);
            ASSERT_EQ (royVec.capacity(), 6u);
        }

        // replace without buffer overrun / new allocation
        {
            Vector<std::string> royVec;
            royVec.reserve (10);

            ASSERT_NO_THROW (royVec.push_back ("1"));
            ASSERT_NO_THROW (royVec.push_back ("2"));
            ASSERT_NO_THROW (royVec.push_back ("3"));
            ASSERT_NO_THROW (royVec.push_back ("4"));
            ASSERT_NO_THROW (royVec.push_back ("5"));
            ASSERT_NO_THROW (royVec.push_back ("6"));

            Vector<std::string> royVec2;
            royVec2.reserve (6);

            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));

            royVec.replace (royVec.end() - 2, royVec2);

            ASSERT_EQ (royVec.at (0), "1");
            ASSERT_EQ (royVec.at (1), "2");
            ASSERT_EQ (royVec.at (2), "3");
            ASSERT_EQ (royVec.at (3), "4");
            ASSERT_EQ (royVec.at (4), "X");
            ASSERT_EQ (royVec.at (5), "X");
            ASSERT_EQ (royVec.at (6), "X");
            ASSERT_EQ (royVec.at (7), "X");
            ASSERT_EQ (royVec.at (8), "X");
            ASSERT_EQ (royVec.at (9), "X");

            ASSERT_EQ (royVec.size(), 10u);
            ASSERT_GT (royVec.capacity(), 10u);
        }

        // replace with buffer reallocation
        {
            Vector<std::string> royVec;
            royVec.reserve (6);

            ASSERT_NO_THROW (royVec.push_back ("1"));
            ASSERT_NO_THROW (royVec.push_back ("2"));
            ASSERT_NO_THROW (royVec.push_back ("3"));
            ASSERT_NO_THROW (royVec.push_back ("4"));
            ASSERT_NO_THROW (royVec.push_back ("5"));
            ASSERT_NO_THROW (royVec.push_back ("6"));

            Vector<std::string> royVec2;
            royVec2.reserve (6);

            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));

            royVec.replace (royVec.end() - 2, royVec2.begin(), royVec2.begin() + 5);

            ASSERT_EQ (royVec.at (0), "1");
            ASSERT_EQ (royVec.at (1), "2");
            ASSERT_EQ (royVec.at (2), "3");
            ASSERT_EQ (royVec.at (3), "4");
            ASSERT_EQ (royVec.at (4), "X");
            ASSERT_EQ (royVec.at (5), "X");
            ASSERT_EQ (royVec.at (6), "X");
            ASSERT_EQ (royVec.at (7), "X");
            ASSERT_EQ (royVec.at (8), "X");

            ASSERT_EQ (royVec.size(), 9u);
            ASSERT_NE (royVec.capacity(), 6u);
        }

        // replace without buffer reallocation
        {
            Vector<std::string> royVec;
            royVec.reserve (6);

            ASSERT_NO_THROW (royVec.push_back ("1"));
            ASSERT_NO_THROW (royVec.push_back ("2"));
            ASSERT_NO_THROW (royVec.push_back ("3"));
            ASSERT_NO_THROW (royVec.push_back ("4"));
            ASSERT_NO_THROW (royVec.push_back ("5"));
            ASSERT_NO_THROW (royVec.push_back ("6"));

            Vector<std::string> royVec2;
            royVec2.reserve (6);

            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));

            royVec.replace (royVec.end() - 2, royVec.end(), royVec2.begin(), royVec2.begin() + 5);

            ASSERT_EQ (royVec.at (0), "1");
            ASSERT_EQ (royVec.at (1), "2");
            ASSERT_EQ (royVec.at (2), "3");
            ASSERT_EQ (royVec.at (3), "4");
            ASSERT_EQ (royVec.at (4), "X");
            ASSERT_EQ (royVec.at (5), "X");

            ASSERT_EQ (royVec.size(), 6u);
            ASSERT_NE (royVec.capacity(), 6u);
        }

        // replace without buffer reallocation; in the middle
        {
            Vector<std::string> royVec;
            royVec.reserve (6);

            ASSERT_NO_THROW (royVec.push_back ("1"));
            ASSERT_NO_THROW (royVec.push_back ("2"));
            ASSERT_NO_THROW (royVec.push_back ("3"));
            ASSERT_NO_THROW (royVec.push_back ("4"));
            ASSERT_NO_THROW (royVec.push_back ("5"));
            ASSERT_NO_THROW (royVec.push_back ("6"));

            Vector<std::string> royVec2;
            royVec2.reserve (6);

            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));

            royVec.replace (royVec.begin() + 2, royVec.begin() + 4, royVec2.begin(), royVec2.begin() + 5);

            ASSERT_EQ (royVec.at (0), "1");
            ASSERT_EQ (royVec.at (1), "2");
            ASSERT_EQ (royVec.at (2), "X");
            ASSERT_EQ (royVec.at (3), "X");
            ASSERT_EQ (royVec.at (4), "5");
            ASSERT_EQ (royVec.at (5), "6");

            ASSERT_EQ (royVec.size(), 6u);
            ASSERT_EQ (royVec.capacity(), 6u);
        }

        // replace with buffer reallocation
        {
            Vector<std::string> royVec;
            royVec.reserve (6);

            ASSERT_NO_THROW (royVec.push_back ("1"));
            ASSERT_NO_THROW (royVec.push_back ("2"));
            ASSERT_NO_THROW (royVec.push_back ("3"));
            ASSERT_NO_THROW (royVec.push_back ("4"));
            ASSERT_NO_THROW (royVec.push_back ("5"));
            ASSERT_NO_THROW (royVec.push_back ("6"));

            Vector<std::string> royVec2;
            royVec2.reserve (6);

            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));

            royVec.replace (royVec.size() - 2, royVec2.begin(), royVec2.end());

            ASSERT_EQ (royVec.at (0), "1");
            ASSERT_EQ (royVec.at (1), "2");
            ASSERT_EQ (royVec.at (2), "3");
            ASSERT_EQ (royVec.at (3), "4");
            ASSERT_EQ (royVec.at (4), "X");
            ASSERT_EQ (royVec.at (5), "X");
            ASSERT_EQ (royVec.at (6), "X");
            ASSERT_EQ (royVec.at (7), "X");
            ASSERT_EQ (royVec.at (8), "X");
            ASSERT_EQ (royVec.at (9), "X");

            ASSERT_EQ (royVec.size(), 10u);
            ASSERT_NE (royVec.capacity(), 6u);
        }

        // replace with buffer reallocation
        {
            Vector<std::string> royVec;
            royVec.reserve (6);

            ASSERT_NO_THROW (royVec.push_back ("1"));
            ASSERT_NO_THROW (royVec.push_back ("2"));
            ASSERT_NO_THROW (royVec.push_back ("3"));
            ASSERT_NO_THROW (royVec.push_back ("4"));
            ASSERT_NO_THROW (royVec.push_back ("5"));
            ASSERT_NO_THROW (royVec.push_back ("6"));

            Vector<std::string> royVec2;
            royVec2.reserve (6);

            ASSERT_NO_THROW (royVec2.push_back ("1"));
            ASSERT_NO_THROW (royVec2.push_back ("2"));
            ASSERT_NO_THROW (royVec2.push_back ("3"));
            ASSERT_NO_THROW (royVec2.push_back ("4"));
            ASSERT_NO_THROW (royVec2.push_back ("5"));
            ASSERT_NO_THROW (royVec2.push_back ("6"));

            royVec.replace (royVec.begin(), royVec.end() - 2, royVec2.rbegin(), royVec2.rend());

            ASSERT_EQ (royVec.at (0), "6");
            ASSERT_EQ (royVec.at (1), "5");
            ASSERT_EQ (royVec.at (2), "4");
            ASSERT_EQ (royVec.at (3), "3");
            ASSERT_EQ (royVec.at (4), "5");
            ASSERT_EQ (royVec.at (5), "6");

            ASSERT_EQ (royVec.size(), 6u);
        }
    }

    // index driven
    {
        // replace without reallocation
        {
            Vector<std::string> royVec;
            royVec.reserve (10);

            ASSERT_NO_THROW (royVec.push_back ("1"));
            ASSERT_NO_THROW (royVec.push_back ("2"));
            ASSERT_NO_THROW (royVec.push_back ("3"));
            ASSERT_NO_THROW (royVec.push_back ("4"));
            ASSERT_NO_THROW (royVec.push_back ("5"));
            ASSERT_NO_THROW (royVec.push_back ("6"));

            Vector<std::string> royVec2;
            royVec2.reserve (6);

            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));

            royVec.replace (royVec.size() - 2, royVec2);

            ASSERT_EQ (royVec.at (0), "1");
            ASSERT_EQ (royVec.at (1), "2");
            ASSERT_EQ (royVec.at (2), "3");
            ASSERT_EQ (royVec.at (3), "4");
            ASSERT_EQ (royVec.at (4), "X");
            ASSERT_EQ (royVec.at (5), "X");
            ASSERT_EQ (royVec.at (6), "X");
            ASSERT_EQ (royVec.at (7), "X");
            ASSERT_EQ (royVec.at (8), "X");
            ASSERT_EQ (royVec.at (9), "X");

            ASSERT_EQ (royVec.size(), 10u);
            ASSERT_GT (royVec.capacity(), 10u);
        }

        // replace with buffer reallocation
        {
            Vector<std::string> royVec;
            royVec.reserve (6);

            ASSERT_NO_THROW (royVec.push_back ("1"));
            ASSERT_NO_THROW (royVec.push_back ("2"));
            ASSERT_NO_THROW (royVec.push_back ("3"));
            ASSERT_NO_THROW (royVec.push_back ("4"));
            ASSERT_NO_THROW (royVec.push_back ("5"));
            ASSERT_NO_THROW (royVec.push_back ("6"));

            Vector<std::string> royVec2;
            royVec2.reserve (6);

            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));
            ASSERT_NO_THROW (royVec2.push_back ("X"));

            royVec.replace (royVec.size() - 2, royVec2.begin(), royVec2.begin() + 5);

            ASSERT_EQ (royVec.at (0), "1");
            ASSERT_EQ (royVec.at (1), "2");
            ASSERT_EQ (royVec.at (2), "3");
            ASSERT_EQ (royVec.at (3), "4");
            ASSERT_EQ (royVec.at (4), "X");
            ASSERT_EQ (royVec.at (5), "X");
            ASSERT_EQ (royVec.at (6), "X");
            ASSERT_EQ (royVec.at (7), "X");
            ASSERT_EQ (royVec.at (8), "X");

            ASSERT_EQ (royVec.size(), 9u);
            ASSERT_NE (royVec.capacity(), 6u);
        }
    }
}

TEST (TestVectorPair, TestVectorIndexIteratorConversions)
{
    // reverse iterator
    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (*royVec.iteratorFromIndex (2), "3");
        ASSERT_EQ (*royVec.iteratorFromIndex (5), "6");

        ASSERT_EQ (* (royVec.begin() + 2), "3");
        ASSERT_EQ (* (royVec.begin() + 5), "6");

        ASSERT_EQ (royVec.indexFromIterator (royVec.begin() + 2), 2u);
        ASSERT_EQ (royVec.indexFromIterator (royVec.begin() + 5), 5u);

        ASSERT_EQ (royVec.indexFromIterator (royVec.end()), royVec.size());

        Vector<std::string>::reverse_iterator revIter (royVec.reverseIteratorFromIndex (2));
        ASSERT_EQ (*revIter, "3");
        ASSERT_EQ (*++revIter, "2");
        ASSERT_EQ (* (--revIter), "3");
        ASSERT_EQ (* (revIter + 1), "2");
        ASSERT_EQ (* (revIter - 1), "4");

        ASSERT_EQ (*revIter, "3");
        revIter = revIter.next();
        ASSERT_EQ (*revIter, "2");
        ASSERT_EQ (* (revIter.next()), "1");
        ASSERT_EQ (revIter.nextItem(), "2");
        ASSERT_EQ (revIter.prev()->length(), 1u);
        ASSERT_EQ (revIter->length(), 1u);
        revIter = revIter.prev().prev();

        ASSERT_EQ (* (revIter.prev()), "4");
        ASSERT_EQ (revIter.prevItem(), "3");
        ASSERT_EQ (revIter.prev()->length(), 1u);
        ASSERT_EQ (revIter->length(), 1u);

        ASSERT_EQ (revIter.nextItem(), "4");
        ASSERT_EQ (revIter.prevItem(), "3");

        revIter++;

        ASSERT_EQ (*revIter, * (royVec.data() + 2));
        ASSERT_EQ (*revIter.base(), * (royVec.data() + 2));

        revIter -= 3;

        ASSERT_EQ (*revIter, "6");
        revIter = royVec.reverseIteratorFromIndex (2);
        ASSERT_EQ (*revIter, "3");
        revIter += 1;
        ASSERT_EQ (*revIter, "2");
        ASSERT_EQ (revIter, royVec.reverseIteratorFromIndex (1));
        ASSERT_EQ (revIter, royVec.iteratorFromIndex (1));
        ASSERT_EQ (revIter, royVec.reverseIteratorFromIndex (1).base());
        revIter--;

        Vector<std::string>::iterator forwardIter;
        forwardIter = revIter.base();

        ASSERT_EQ (*forwardIter, "3");
        ASSERT_EQ (*++forwardIter, "4");
        ASSERT_EQ (* (--forwardIter), "3");
        ASSERT_EQ (* (forwardIter + 1), "4");
        ASSERT_EQ (* (forwardIter - 1), "2");

        ASSERT_EQ (* (forwardIter.next()), "4");
        ASSERT_EQ (forwardIter.nextItem(), "3");
        ASSERT_EQ (forwardIter.next()->length(), 1u);
        ASSERT_EQ (forwardIter->length(), 1u);
        forwardIter = forwardIter.prev();

        ASSERT_EQ (* (forwardIter.prev()), "2");
        ASSERT_EQ (forwardIter.prevItem(), "3");
        ASSERT_EQ (forwardIter.prev()->length(), 1u);
        ASSERT_EQ (forwardIter->length(), 1u);

        forwardIter = forwardIter.next().next();

        ASSERT_EQ (forwardIter.nextItem(), "4");
        ASSERT_EQ (forwardIter.prevItem(), "5");

        forwardIter--;

        ASSERT_EQ (*forwardIter, * (royVec.data() + 2));

        forwardIter += 3;

        ASSERT_EQ (*forwardIter, "6");
        forwardIter = royVec.iteratorFromIndex (2);
        ASSERT_EQ (*forwardIter, "3");
        forwardIter -= 1;
        ASSERT_EQ (*forwardIter, "2");
        ASSERT_EQ (forwardIter, royVec.reverseIteratorFromIndex (1));
        ASSERT_EQ (forwardIter, royVec.iteratorFromIndex (1));
        ASSERT_EQ (forwardIter, royVec.reverseIteratorFromIndex (1).base());
    }

    // const reverse iterator
    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        const Vector<std::string> &v = royVec;

        ASSERT_EQ (*v.iteratorFromIndex (2), "3");
        ASSERT_EQ (*v.iteratorFromIndex (5), "6");

        ASSERT_EQ (* (v.begin() + 2), "3");
        ASSERT_EQ (* (v.begin() + 5), "6");

        ASSERT_EQ (v.indexFromIterator (v.begin() + 2), 2u);
        ASSERT_EQ (v.indexFromIterator (v.begin() + 5), 5u);

        ASSERT_EQ (v.indexFromIterator (v.end()), v.size());

        Vector<std::string>::const_reverse_iterator revIter (v.reverseIteratorFromIndex (2));
        ASSERT_EQ (*revIter, "3");
        ASSERT_EQ (*++revIter, "2");
        ASSERT_EQ (* (--revIter), "3");
        ASSERT_EQ (* (revIter + 1), "2");
        ASSERT_EQ (* (revIter - 1), "4");

        ASSERT_EQ (*revIter, "3");
        revIter = revIter.next();
        ASSERT_EQ (*revIter, "2");
        ASSERT_EQ (* (revIter.next()), "1");
        ASSERT_EQ (revIter.nextItem(), "2");
        ASSERT_EQ (revIter.prev()->length(), 1u);
        ASSERT_EQ (revIter->length(), 1u);
        revIter = revIter.prev().prev();

        ASSERT_EQ (* (revIter.prev()), "4");
        ASSERT_EQ (revIter.prevItem(), "3");
        ASSERT_EQ (revIter.prev()->length(), 1u);
        ASSERT_EQ (revIter->length(), 1u);

        ASSERT_EQ (revIter.nextItem(), "4");
        ASSERT_EQ (revIter.prevItem(), "3");

        revIter++;

        ASSERT_EQ (*revIter, * (royVec.data() + 2));
        ASSERT_EQ (*revIter.base(), * (royVec.data() + 2));

        revIter -= 3;

        ASSERT_EQ (*revIter, "6");
        revIter = royVec.reverseIteratorFromIndex (2);
        ASSERT_EQ (*revIter, "3");
        revIter += 1;
        ASSERT_EQ (*revIter, "2");
        ASSERT_EQ (revIter, royVec.reverseIteratorFromIndex (1));
        ASSERT_EQ (revIter, royVec.iteratorFromIndex (1));
        ASSERT_EQ (revIter, royVec.reverseIteratorFromIndex (1).base());
        revIter--;

        Vector<std::string>::const_iterator forwardIter;
        forwardIter = revIter.base();

        ASSERT_EQ (*forwardIter, "3");
        ASSERT_EQ (*++forwardIter, "4");
        ASSERT_EQ (* (--forwardIter), "3");
        ASSERT_EQ (* (forwardIter + 1), "4");
        ASSERT_EQ (* (forwardIter - 1), "2");

        ASSERT_EQ (* (forwardIter.next()), "4");
        ASSERT_EQ (forwardIter.nextItem(), "3");
        ASSERT_EQ (forwardIter.next()->length(), 1u);
        ASSERT_EQ (forwardIter->length(), 1u);
        forwardIter = forwardIter.prev();

        ASSERT_EQ (* (forwardIter.prev()), "2");
        ASSERT_EQ (forwardIter.prevItem(), "3");
        ASSERT_EQ (forwardIter.prev()->length(), 1u);
        ASSERT_EQ (forwardIter->length(), 1u);

        forwardIter = forwardIter.next().next();

        ASSERT_EQ (forwardIter.nextItem(), "4");
        ASSERT_EQ (forwardIter.prevItem(), "5");

        forwardIter--;

        ASSERT_EQ (*forwardIter, * (royVec.data() + 2));

        forwardIter += 3;

        ASSERT_EQ (*forwardIter, "6");
        forwardIter = royVec.iteratorFromIndex (2);
        ASSERT_EQ (*forwardIter, "3");
        forwardIter -= 1;
        ASSERT_EQ (*forwardIter, "2");
        ASSERT_EQ (forwardIter, royVec.reverseIteratorFromIndex (1));
        ASSERT_EQ (forwardIter, royVec.iteratorFromIndex (1));
        ASSERT_EQ (forwardIter, royVec.reverseIteratorFromIndex (1).base());
    }

    {
        Vector<String> vec ({ "Hello", "World", "out", "there" });
        const Vector<String> &vecRef = vec;

        Vector<String>::const_reverse_iterator cRevIter (vecRef.rbegin());
        ASSERT_EQ (*cRevIter, "there");
        ASSERT_EQ (* (vecRef.rend() - 1), "Hello");

        Vector<String>::const_iterator cIter (vecRef.cbegin());
        ASSERT_EQ (*cIter, "Hello");
        ASSERT_EQ (* (vecRef.cend() - 1), "there");

        cRevIter = vecRef.crbegin();
        ASSERT_EQ (*cRevIter, "there");
        ASSERT_EQ (* (vecRef.crend() - 1), "Hello");

        ASSERT_EQ (* (vecRef.crend() - 1), * (vecRef.rend() - 1));
        ASSERT_EQ (* (vecRef.cbegin()), * (vecRef.begin()));

        ASSERT_EQ (vecRef.indexFromIterator (cRevIter), 3u);

        Vector<String>::reverse_iterator revIter (vec.rbegin());
        ASSERT_EQ (*revIter, "there");
        ASSERT_EQ (vec.indexFromIterator (revIter), 3u);
        ASSERT_EQ (vecRef.indexFromIterator (cRevIter), 3u);
    }
}

TEST (TestVectorPair, TestVectorErase)
{
    {
        std::vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");


        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        royVec.erase ( (royVec.begin() + 1), (royVec.begin() + 5));

        ASSERT_EQ (royVec.size(), 2u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "6");
    }

    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        ASSERT_EQ (royVec.indexOf ("3"), 2u);
        ASSERT_EQ (royVec.indexOf ("7"), Vector<std::string>::npos);
        ASSERT_EQ (royVec.indexOf ("3", 1), 2u);
        ASSERT_EQ (royVec.indexOf ("3", 2), royVec.max_size());

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        royVec.erase ( (royVec.begin() + 1), (royVec.begin() + 5));

        ASSERT_EQ (royVec.size(), 2u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "6");

        ASSERT_EQ (royVec.contains ("3"), false);
        ASSERT_EQ (royVec.indexOf ("3"), Vector<std::string>::npos);
    }

    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        //ASSERT_EQ(royVec.contains("3"), true);
        ASSERT_EQ (royVec.indexOf ("3"), 2u);
        ASSERT_EQ (royVec.indexOf ("7"), Vector<std::string>::npos);
        ASSERT_EQ (royVec.indexOf ("3", 1), 2u);
        ASSERT_EQ (royVec.indexOf ("3", 2u), royVec.max_size());

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        royVec.erase (royVec.begin(), royVec.begin() + 1);

        ASSERT_EQ (royVec.size(), 5u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "2");
        ASSERT_EQ (royVec.at (1), "3");
        ASSERT_EQ (royVec.at (2), "4");
        ASSERT_EQ (royVec.at (3), "5");
        ASSERT_EQ (royVec.at (4), "6");

        ASSERT_EQ (royVec.contains ("3"), true);
        ASSERT_GE (royVec.indexOf ("3"), 0u);
    }

    {
        std::vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        royVec.erase (royVec.begin(), royVec.begin() + 1);

        ASSERT_EQ (royVec.size(), 5u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "2");
        ASSERT_EQ (royVec.at (1), "3");
        ASSERT_EQ (royVec.at (2), "4");
        ASSERT_EQ (royVec.at (3), "5");
        ASSERT_EQ (royVec.at (4), "6");
    }

    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        royVec.erase ( (royVec.begin() + 1), (royVec.begin() + 5));

        ASSERT_EQ (royVec.size(), 2u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "6");

        royVec.erase (royVec.end() - 1);
        ASSERT_EQ (royVec.size(), 1u);
        ASSERT_EQ (royVec.capacity(), 10u);
        ASSERT_EQ (royVec.at (0), "1");

        royVec.erase (royVec.begin());
        ASSERT_EQ (royVec.size(), 0u);
        ASSERT_EQ (royVec.capacity(), 10u);
    }

    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        royVec.erase ( (royVec.begin() + 1), (royVec.begin() + 3));

        ASSERT_EQ (royVec.size(), 4u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "4");
        ASSERT_EQ (royVec.at (2), "5");
        ASSERT_EQ (royVec.at (3), "6");
    }

    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        royVec.erase (royVec.begin() + 3, royVec.end());

        ASSERT_EQ (royVec.size(), 3u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
    }

    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        royVec.erase (royVec.begin(), royVec.begin() + 1);

        ASSERT_EQ (royVec.size(), 5u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "2");
        ASSERT_EQ (royVec.at (1), "3");
        ASSERT_EQ (royVec.at (2), "4");
        ASSERT_EQ (royVec.at (3), "5");
        ASSERT_EQ (royVec.at (4), "6");
    }

    {
        Vector<char> royVec = { 'a', 'b', 'c', 'd', 'e', 'f' };
        royVec.pop_back();
        royVec.pop_back();
        ASSERT_EQ (royVec.at (0), 'a');
        ASSERT_EQ (royVec.at (1), 'b');
        ASSERT_EQ (royVec.at (2), 'c');
        ASSERT_EQ (royVec.at (3), 'd');
    }

    {
        Vector<char> royVec = { 'a', 'b', 'c', 'd', 'e', 'f' };
        royVec.erase (1, 2);
        ASSERT_EQ (royVec.at (0), 'a');
        ASSERT_EQ (royVec.at (1), 'd');
        ASSERT_EQ (royVec.at (2), 'e');
        ASSERT_EQ (royVec.at (3), 'f');

        std::vector<char> stdVec = { 'a', 'b', 'c', 'd', 'e', 'f' };
        stdVec.erase (stdVec.begin() + 1, stdVec.begin() + 3);
        ASSERT_EQ (stdVec.at (0), 'a');
        ASSERT_EQ (stdVec.at (1), 'd');
        ASSERT_EQ (stdVec.at (2), 'e');
        ASSERT_EQ (stdVec.at (3), 'f');
    }

    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        royVec.erase ( (royVec.begin() + 1), (royVec.begin() + 5));

        ASSERT_EQ (royVec.size(), 2u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "6");

        royVec.erase (royVec.end() - 1);
        ASSERT_EQ (royVec.size(), 1u);
        ASSERT_EQ (royVec.capacity(), 10u);
        ASSERT_EQ (royVec.at (0), "1");

        royVec.erase (royVec.begin());
        ASSERT_EQ (royVec.size(), 0u);
        ASSERT_EQ (royVec.capacity(), 10u);
    }

    {
        std::vector<std::string> stdVec;
        stdVec.reserve (10);

        ASSERT_NO_THROW (stdVec.push_back ("1"));
        ASSERT_NO_THROW (stdVec.push_back ("2"));
        ASSERT_NO_THROW (stdVec.push_back ("3"));
        ASSERT_NO_THROW (stdVec.push_back ("4"));
        ASSERT_NO_THROW (stdVec.push_back ("5"));
        ASSERT_NO_THROW (stdVec.push_back ("6"));

        ASSERT_EQ (stdVec.at (0), "1");
        ASSERT_EQ (stdVec.at (1), "2");
        ASSERT_EQ (stdVec.at (2), "3");
        ASSERT_EQ (stdVec.at (3), "4");
        ASSERT_EQ (stdVec.at (4), "5");
        ASSERT_EQ (stdVec.at (5), "6");

        ASSERT_EQ (stdVec.size(), 6u);
        ASSERT_EQ (stdVec.capacity(), 10u);

        stdVec.erase ( (stdVec.begin() + 1), (stdVec.begin() + 3));

        ASSERT_EQ (stdVec.size(), 4u);
        ASSERT_EQ (stdVec.capacity(), 10u);

        ASSERT_EQ (stdVec.at (0), "1");
        ASSERT_EQ (stdVec.at (1), "4");
        ASSERT_EQ (stdVec.at (2), "5");
        ASSERT_EQ (stdVec.at (3), "6");
    }

    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        royVec.erase ( (royVec.begin() + 1), (royVec.begin() + 3));

        ASSERT_EQ (royVec.size(), 4u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "4");
        ASSERT_EQ (royVec.at (2), "5");
        ASSERT_EQ (royVec.at (3), "6");
    }

    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        royVec.erase (royVec.begin() + 3, royVec.end());

        ASSERT_EQ (royVec.size(), 3u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
    }

    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        royVec.erase (royVec.begin(), royVec.begin() + 1);

        ASSERT_EQ (royVec.size(), 5u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "2");
        ASSERT_EQ (royVec.at (1), "3");
        ASSERT_EQ (royVec.at (2), "4");
        ASSERT_EQ (royVec.at (3), "5");
        ASSERT_EQ (royVec.at (4), "6");
    }
}

TEST (TestVectorPair, TestVectorAssign)
{
    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        Vector<std::string> assignVec;
        ASSERT_NO_THROW (assignVec.push_back ("a"));
        ASSERT_NO_THROW (assignVec.push_back ("b"));
        ASSERT_NO_THROW (assignVec.push_back ("c"));

        royVec.assign (assignVec.begin(), assignVec.end());

        ASSERT_EQ (royVec.at (0), "a");
        ASSERT_EQ (royVec.at (1), "b");
        ASSERT_EQ (royVec.at (2), "c");

        ASSERT_EQ (royVec.size(), 3u);
        ASSERT_EQ (royVec.capacity(), 10u);
    }

    {
        Vector<std::string> royVec;
        royVec.reserve (6);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 6u);

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        Vector<std::string> assignVec;
        ASSERT_NO_THROW (assignVec.push_back ("a"));
        ASSERT_NO_THROW (assignVec.push_back ("b"));
        ASSERT_NO_THROW (assignVec.push_back ("c"));

        royVec.assign (assignVec.begin(), assignVec.end());

        ASSERT_EQ (royVec.at (0), "a");
        ASSERT_EQ (royVec.at (1), "b");
        ASSERT_EQ (royVec.at (2), "c");

        ASSERT_EQ (royVec.size(), 3u);
        ASSERT_EQ (royVec.capacity(), 6u);

        royVec.emplace_back ("d");
        royVec.emplace_back ("e");
        royVec.emplace_back ("f");
        royVec.emplace_back ("g");
        royVec.emplace_back ("h");

        ASSERT_GT (royVec.capacity(), 6u);
        ASSERT_EQ (royVec.at (0), "a");
        ASSERT_EQ (royVec.at (1), "b");
        ASSERT_EQ (royVec.at (2), "c");
        ASSERT_EQ (royVec.at (3), "d");
        ASSERT_EQ (royVec.at (4), "e");
        ASSERT_EQ (royVec.at (5), "f");
        ASSERT_EQ (royVec.at (6), "g");
        ASSERT_EQ (royVec.at (7), "h");
    }

    {
        Vector<std::string> royVec;
        royVec.reserve (3);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));

        ASSERT_EQ (royVec.size(), 3u);
        ASSERT_EQ (royVec.capacity(), 3u);

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");

        Vector<std::string> assignVec;
        ASSERT_NO_THROW (assignVec.push_back ("a"));
        ASSERT_NO_THROW (assignVec.push_back ("b"));
        ASSERT_NO_THROW (assignVec.push_back ("c"));
        ASSERT_NO_THROW (assignVec.push_back ("d"));
        ASSERT_NO_THROW (assignVec.push_back ("e"));
        ASSERT_NO_THROW (assignVec.push_back ("f"));

        royVec.assign (assignVec.begin(), assignVec.end());

        ASSERT_EQ (royVec.at (0), "a");
        ASSERT_EQ (royVec.at (1), "b");
        ASSERT_EQ (royVec.at (2), "c");
        ASSERT_EQ (royVec.at (3), "d");
        ASSERT_EQ (royVec.at (4), "e");
        ASSERT_EQ (royVec.at (5), "f");

        ASSERT_EQ (royVec.size(), 6u);
    }

    {
        Vector<std::string> royVec;

        royVec.assign (10, "Hello World");

        ASSERT_EQ (royVec.at (0), "Hello World");
        ASSERT_EQ (royVec.at (1), "Hello World");
        ASSERT_EQ (royVec.at (2), "Hello World");
        ASSERT_EQ (royVec.at (3), "Hello World");
        ASSERT_EQ (royVec.at (4), "Hello World");
        ASSERT_EQ (royVec.at (5), "Hello World");
        ASSERT_EQ (royVec.at (6), "Hello World");
        ASSERT_EQ (royVec.at (7), "Hello World");
        ASSERT_EQ (royVec.at (8), "Hello World");
        ASSERT_EQ (royVec.at (9), "Hello World");


        ASSERT_EQ (royVec.size(), 10u);
        ASSERT_GE (royVec.capacity(), 10u);
    }

    {
        Vector<std::string> royVec;
        royVec.reserve (100);
        royVec.assign (10, "Hello World");

        ASSERT_EQ (royVec.at (0), "Hello World");
        ASSERT_EQ (royVec.at (1), "Hello World");
        ASSERT_EQ (royVec.at (2), "Hello World");
        ASSERT_EQ (royVec.at (3), "Hello World");
        ASSERT_EQ (royVec.at (4), "Hello World");
        ASSERT_EQ (royVec.at (5), "Hello World");
        ASSERT_EQ (royVec.at (6), "Hello World");
        ASSERT_EQ (royVec.at (7), "Hello World");
        ASSERT_EQ (royVec.at (8), "Hello World");
        ASSERT_EQ (royVec.at (9), "Hello World");


        ASSERT_EQ (royVec.size(), 10u);
        ASSERT_GE (royVec.capacity(), 10u);
    }

    {
        Vector<std::string> royVec;
        royVec.resize (100);
        royVec.assign (10, "Hello World");

        ASSERT_EQ (royVec.at (0), "Hello World");
        ASSERT_EQ (royVec.at (1), "Hello World");
        ASSERT_EQ (royVec.at (2), "Hello World");
        ASSERT_EQ (royVec.at (3), "Hello World");
        ASSERT_EQ (royVec.at (4), "Hello World");
        ASSERT_EQ (royVec.at (5), "Hello World");
        ASSERT_EQ (royVec.at (6), "Hello World");
        ASSERT_EQ (royVec.at (7), "Hello World");
        ASSERT_EQ (royVec.at (8), "Hello World");
        ASSERT_EQ (royVec.at (9), "Hello World");

        ASSERT_EQ (royVec.size(), 10u);
        ASSERT_GE (royVec.capacity(), 10u);
    }

    {
        Vector<std::string> royVec;
        royVec.resize (100);
        royVec.assign (10, "Hello World");

        ASSERT_EQ (royVec.at (0), "Hello World");
        ASSERT_EQ (royVec.at (1), "Hello World");
        ASSERT_EQ (royVec.at (2), "Hello World");
        ASSERT_EQ (royVec.at (3), "Hello World");
        ASSERT_EQ (royVec.at (4), "Hello World");
        ASSERT_EQ (royVec.at (5), "Hello World");
        ASSERT_EQ (royVec.at (6), "Hello World");
        ASSERT_EQ (royVec.at (7), "Hello World");
        ASSERT_EQ (royVec.at (8), "Hello World");
        ASSERT_EQ (royVec.at (9), "Hello World");

        ASSERT_EQ (royVec.size(), 10u);
        ASSERT_GE (royVec.capacity(), 10u);

        royVec.assign ({ "Hello World #1", "Hello World #2", "Hello World #3", "Hello World #4" });

        ASSERT_EQ (royVec.at (0), "Hello World #1");
        ASSERT_EQ (royVec.at (1), "Hello World #2");
        ASSERT_EQ (royVec.at (2), "Hello World #3");
        ASSERT_EQ (royVec.at (3), "Hello World #4");

        ASSERT_EQ (royVec.size(), 4u);
        ASSERT_GE (royVec.capacity(), 4u);
    }
}

TEST (TestVectorPair, TestTakeItems)
{
    // TakeItem
    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.takeItem (royVec.begin() + 3), "4");
        ASSERT_EQ (royVec.size(), 5u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "5");
        ASSERT_EQ (royVec.at (4), "6");

        ASSERT_THROW (royVec.takeItem (royVec.end() + 1), std::out_of_range);
    }

    // TakeItems
    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        Vector<std::string> check = { "2", "3", "4", "5" };
        Vector<std::string> takenElements (royVec.takeItems ( (royVec.begin() + 1), 4));
        ASSERT_EQ (takenElements, check);

        ASSERT_EQ (royVec.size(), 2u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "6");
        ASSERT_THROW (royVec.takeItems (royVec.end() + 1, 3), std::out_of_range);
    }

    // TakeItems
    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        Vector<std::string> check = { "2", "3", "4", "5" };
        Vector<std::string> takenElements (royVec.takeItems (1, 4));
        ASSERT_EQ (takenElements, check);

        ASSERT_EQ (royVec.size(), 2u);
        ASSERT_EQ (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "6");
    }

    // TakeItems and swap
    {
        Vector<std::string> royVec;
        royVec.reserve (10);

        ASSERT_NO_THROW (royVec.push_back ("1"));
        ASSERT_NO_THROW (royVec.push_back ("2"));
        ASSERT_NO_THROW (royVec.push_back ("3"));
        ASSERT_NO_THROW (royVec.push_back ("4"));
        ASSERT_NO_THROW (royVec.push_back ("5"));
        ASSERT_NO_THROW (royVec.push_back ("6"));

        ASSERT_EQ (royVec.at (0), "1");
        ASSERT_EQ (royVec.at (1), "2");
        ASSERT_EQ (royVec.at (2), "3");
        ASSERT_EQ (royVec.at (3), "4");
        ASSERT_EQ (royVec.at (4), "5");
        ASSERT_EQ (royVec.at (5), "6");

        ASSERT_EQ (royVec.size(), 6u);
        ASSERT_EQ (royVec.capacity(), 10u);

        Vector<std::string> check = { "2", "3", "4", "5" };
        royVec.swap (royVec.takeItems (1, 4));
        ASSERT_EQ (royVec, check);

        ASSERT_EQ (royVec.size(), 4u);

        ASSERT_EQ (royVec.at (0), "2");
        ASSERT_EQ (royVec.at (1), "3");
        ASSERT_EQ (royVec.at (2), "4");
        ASSERT_EQ (royVec.at (3), "5");
    }
}

TEST (TestVectorPair, TestVectorFromString)
{
    {
        std::vector<std::string> stdVec;
        ASSERT_NO_THROW (stdVec.push_back ("Hello World #1"));
        ASSERT_NO_THROW (stdVec.push_back ("Hello World #2"));
        ASSERT_NO_THROW (stdVec.push_back ("Hello World #3"));
        ASSERT_NO_THROW (stdVec.push_back ("Hello World #4"));

        Vector<std::string> royVec (stdVec.begin(), stdVec.end());

        for (Vector<std::string>::iterator i = royVec.begin(); i != royVec.end(); i++)
        {
            auto diff = i - royVec.begin();
            String checkString = String ("Hello World #");
            checkString += String::fromAny<Vector<std::string>::iterator::difference_type> (diff + 1);
            ASSERT_EQ (checkString, *i);
        }

        Vector<char> characterVec = Vector<char>::fromString (stdVec.at (0));
        for (Vector<char>::iterator i = characterVec.begin(); i != characterVec.end(); i++)
        {
            ASSERT_EQ (stdVec[0][i - characterVec.begin()], *i);
        }
    }

    {
        std::vector<std::string> stdVec;
        ASSERT_NO_THROW (stdVec.push_back ("Hello World #1"));
        ASSERT_NO_THROW (stdVec.push_back ("Hello World #2"));
        ASSERT_NO_THROW (stdVec.push_back ("Hello World #3"));
        ASSERT_NO_THROW (stdVec.push_back ("Hello World #4"));

        Vector<std::string> royVec (stdVec.begin(), stdVec.end());

        for (Vector<std::string>::iterator i = royVec.begin(); i != royVec.end(); i++)
        {
            auto diff = i - royVec.begin();
            String checkString = String ("Hello World #");
            checkString += String::fromAny<Vector<std::string>::iterator::difference_type> (diff + 1);
            ASSERT_EQ (checkString, *i);
        }

        Vector<char> characterVec;
        characterVec.reserve (20);
        characterVec.insert (characterVec.begin(), stdVec[0].begin(), stdVec[0].end());
        for (Vector<char>::iterator i = characterVec.begin(); i != characterVec.end(); i++)
        {
            ASSERT_EQ (stdVec[0][i - characterVec.begin()], *i);
        }
    }
}

TEST (TestVectorPair, TestVectorEdgeCases)
{
    {
        Vector<std::string> royVec;
        ASSERT_THROW (royVec.back(), std::out_of_range);
        ASSERT_THROW (royVec.front(), std::out_of_range);

        const Vector<std::string> croyVec;
        ASSERT_THROW (croyVec.back(), std::out_of_range);
        ASSERT_THROW (croyVec.front(), std::out_of_range);
    }

    // Royale
    {
        std::vector<std::string> stdVec;
        ASSERT_NO_THROW (stdVec.push_back ("Hello World #1"));
        ASSERT_NO_THROW (stdVec.push_back ("Hello World #2"));
        ASSERT_NO_THROW (stdVec.push_back ("Hello World #3"));
        ASSERT_NO_THROW (stdVec.push_back ("Hello World #4"));

        Vector<std::string> royVec (stdVec.begin(), stdVec.end());

        ASSERT_EQ (royVec.size(), 4u);
        ASSERT_EQ (royVec.count(), 4u);
        ASSERT_EQ (royVec.capacity(), 4u);

        //ASSERT_NO_THROW(ASSERT_EQ(royVec.erase(4, 1), royVec.begin()));

        ASSERT_NO_THROW (royVec.clear());
        ASSERT_NO_THROW (royVec.clear());
        ASSERT_NO_THROW (royVec.clear());

        ASSERT_EQ (royVec.size(), 0u);
        ASSERT_EQ (royVec.count(), 0u);
        ASSERT_EQ (royVec.capacity(), 0u);

        Vector<std::string>::iterator replaceIterator (royVec.replace (0, stdVec));
        ASSERT_EQ (replaceIterator, royVec.begin());
        ASSERT_EQ (royVec.size(), 4u);
        ASSERT_EQ (royVec.count(), 4u);
        ASSERT_GE (royVec.capacity(), 4u);

        Vector<std::string>::iterator checkIterator = royVec.erase (royVec.size() - 1);
        ASSERT_EQ (checkIterator, royVec.end());
        checkIterator = royVec.erase (royVec.begin(), royVec.end());
        ASSERT_EQ (checkIterator, royVec.begin());

        // removes all elements with the value 5
        //royVec.erase(std::remove(royVec.begin(), royVec.end(), "Hello World #2"), royVec.end());
        //std::cout << royVec << std::endl;

        ASSERT_EQ (royVec.size(), 0u);
        ASSERT_EQ (royVec.count(), 0u);
        ASSERT_GE (royVec.capacity(), 4u);
        ASSERT_NO_THROW (royVec.clear());
        ASSERT_EQ (royVec.capacity(), 0u);
        ASSERT_EQ (royVec.erase (royVec.begin(), royVec.end()), royVec.begin());
        ASSERT_EQ (royVec.erase (royVec.begin(), 100u), royVec.begin());
        ASSERT_EQ (royVec.erase (10, 100), royVec.begin());

        ASSERT_EQ (royVec.erase (10, 100), royVec.begin());

        ASSERT_NO_THROW (stdVec.push_back ("Hello World #5"));
        ASSERT_NO_THROW (royVec.assign (stdVec.begin(), stdVec.end()));
        ASSERT_EQ (royVec.size(), 5u);
        ASSERT_EQ (royVec.count(), 5u);
        ASSERT_GE (royVec.capacity(), 5u);

        checkIterator = royVec.replace (royVec.size(), stdVec);
        ASSERT_EQ (checkIterator, royVec.begin() + 5);

        ASSERT_EQ (royVec.size(), 10u);
        ASSERT_EQ (royVec.count(), 10u);
        ASSERT_GE (royVec.capacity(), 10u);

        checkIterator = royVec.replace (royVec.end(), stdVec.crbegin(), stdVec.crend());
        ASSERT_NO_THROW (ASSERT_EQ (checkIterator, royVec.end() - 5));
        ASSERT_EQ (royVec.size(), 15u);
        ASSERT_EQ (royVec.count(), 15u);
        ASSERT_GE (royVec.capacity(), 10u);

        ASSERT_EQ (royVec.at (0), "Hello World #1");
        ASSERT_EQ (royVec.at (1), "Hello World #2");
        ASSERT_EQ (royVec.at (2), "Hello World #3");
        ASSERT_EQ (royVec.at (3), "Hello World #4");
        ASSERT_EQ (royVec.at (4), "Hello World #5");
        ASSERT_EQ (royVec.at (5), "Hello World #1");
        ASSERT_EQ (royVec.at (6), "Hello World #2");
        ASSERT_EQ (royVec.at (7), "Hello World #3");
        ASSERT_EQ (royVec.at (8), "Hello World #4");
        ASSERT_EQ (royVec.at (9), "Hello World #5");
        ASSERT_EQ (royVec.at (10), "Hello World #5");
        ASSERT_EQ (royVec.at (11), "Hello World #4");
        ASSERT_EQ (royVec.at (12), "Hello World #3");
        ASSERT_EQ (royVec.at (13), "Hello World #2");
        ASSERT_EQ (royVec.at (14), "Hello World #1");
    }
}

namespace
{
    class testIntClass
    {
    public:
        uint32_t data;
        DoubleDestructionCanary canary;
    };

    class testStringClass
    {
    public:
        royale::String data;
        DoubleDestructionCanary canary;

        explicit testStringClass () = default;

        explicit testStringClass (royale::String s) :
            data (s)
        {
        }

        bool operator== (const testStringClass &rhs) const
        {
            return data == rhs.data;
        }

        bool operator!= (const testStringClass &rhs) const
        {
            return ! (*this == rhs);
        }
    };
}

TEST (TestVectorPair, TestForMemoryLeaks)
{
    // This test was only added to reveal certain memory leaks.
    // It does not test Vector functionality.
    DestructorCounter counter;
    {
        royale::Vector<testStringClass> vec;

        for (auto i = 0; i < 5; ++i)
        {
            testStringClass dummy;
            dummy.data = "testing";

            vec.push_back (dummy);
        }
    }

    {
        royale::Vector<royale::Vector<testIntClass>> vec;
        royale::Vector<testIntClass> vec2;

        testIntClass dummy;
        dummy.data = 32;

        vec2.push_back (dummy);
        vec2.push_back (dummy);

        vec.push_back (vec2);
        vec.push_back (vec2);

        vec.resize (1);
    }

    // TakeItems
    {
        Vector<testStringClass> vec;
        vec.reserve (10u);

        ASSERT_NO_THROW (vec.emplace_back ("1"));
        ASSERT_NO_THROW (vec.emplace_back ("2"));
        ASSERT_NO_THROW (vec.emplace_back ("3"));
        ASSERT_NO_THROW (vec.emplace_back ("4"));
        ASSERT_NO_THROW (vec.emplace_back ("5"));
        ASSERT_NO_THROW (vec.emplace_back ("6"));

        auto takenElements = vec.takeItems (vec.begin() + 1, 4);

        ASSERT_EQ (vec.size(), 2u);
        ASSERT_EQ (vec.capacity(), 10u);

        ASSERT_EQ (vec.at (0), testStringClass ("1"));
        ASSERT_EQ (vec.at (1), testStringClass ("6"));
        ASSERT_NE (vec.at (1), testStringClass ("1"));
        ASSERT_THROW (vec.takeItems (vec.end() + 1, 3), std::out_of_range);
    }
}
