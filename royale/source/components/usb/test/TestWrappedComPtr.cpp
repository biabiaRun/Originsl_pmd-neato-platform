/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#include <usb/bridge/WrappedComPtr.hpp>

#include <memory>
#include <gtest/gtest.h>

using namespace royale::usb::bridge;

namespace
{
    /**
     * The number of ComRefCheck instances in existance (measured as the
     * difference between constructions - deletions).
     */
    static signed int balanceCRCs = 0;

    /**
     * Simulation of a COM object's ref-count altering methods.
     */
    class ComRefCheck
    {
    public:
        ComRefCheck () :
            m_refCount {1}
        {
            balanceCRCs++;
        }

        void AddRef()
        {
            m_refCount++;
        }

        void Release()
        {
            ASSERT_GE (m_refCount, 1);
            m_refCount--;
            if (m_refCount == 0)
            {
                delete this;
            }
        }

        /**
         * This can only be called if the object is reachable, check that tihs
         * expects to be reachable.
         */
        void checkRefCount()
        {
            ASSERT_GE (m_refCount, 1);
        }

        ~ComRefCheck()
        {
            // Asserting in a destructor may terminate the process (throwing an
            // exception from a destructor), but that's acceptable for a test
            // that must fail.
            checkRefCountZero();
            balanceCRCs--;
        }

    private:
        /**
         * Number of references to this object.  This can be negative, in which
         * the assert in the destructor will fail the test.
         */
        signed int m_refCount;

        /**
         * Test for the destructor (the gtest framework expands the ASSERT_ to
         * statements including a return, and GCC refuses to compile a
         * statement that returns a value from a constructor.
         */
        void checkRefCountZero()
        {
            ASSERT_EQ (m_refCount, 0);
        }
    };

    void CreateComRefCheckInstance (void **ref)
    {
        *ref = new ComRefCheck();
    }
}

/**
 * A fixture to always ensure that the final refcounts are tested, but also to
 * ensure that a failing test doesn't cause all later tests to fail.
 */
class TestWrappedComPtr : public ::testing::Test
{
public:
    void SetUp() override
    {
        balanceCRCs = 0;
    }

    void TearDown() override
    {
        ASSERT_EQ (balanceCRCs, 0);
    }
};

/**
 * A WrappedComPtr that is never used.
 * Nothing in this test creates a ComRefCheck, thus the early assert should pass.
 */
TEST_F (TestWrappedComPtr, constructAndDeleteEmpty)
{
    {
        WrappedComPtr<ComRefCheck> wrapper;
        ASSERT_EQ (balanceCRCs, 0);
    }
}

TEST_F (TestWrappedComPtr, constructAndDeleteObject)
{
    {
        WrappedComPtr<ComRefCheck> wrapper;
        CreateComRefCheckInstance (wrapper.getRef());
        ASSERT_EQ (balanceCRCs, 1);
        wrapper->checkRefCount();
    }
}

/**
 * Test that WrappedComPtr throws if getRef() is called when the wrapper
 * already has a ref.
 *
 * This doesn't leak, because the exception is thrown before the second
 * CreateComRefCheckInstance call happens.
 */
TEST_F (TestWrappedComPtr, leakGuard)
{
    {
        WrappedComPtr<ComRefCheck> wrapper;
        CreateComRefCheckInstance (wrapper.getRef());
        ASSERT_EQ (balanceCRCs, 1);
        wrapper->checkRefCount();
        ASSERT_THROW (CreateComRefCheckInstance (wrapper.getRef()), royale::common::RuntimeError);
    }
}

/**
 * Test the constructor that takes a reference, and adds an extra reference
 */
TEST_F (TestWrappedComPtr, constructWithObjectAddRef)
{
    ComRefCheck *barePtr {new ComRefCheck};
    {
        WrappedComPtr<ComRefCheck> wrapper {barePtr};
        ASSERT_EQ (balanceCRCs, 1);
        wrapper->checkRefCount();
    }
    barePtr->checkRefCount();
    barePtr->Release();
}

/**
 * Test two instances of WrappedComPtr pointing to the same object.
 */
TEST_F (TestWrappedComPtr, twoWrappers)
{
    {
        WrappedComPtr<ComRefCheck> firstWrapper;
        CreateComRefCheckInstance (firstWrapper.getRef());
        WrappedComPtr<ComRefCheck> secondWrapper;

        firstWrapper.get()->AddRef();
        * (secondWrapper.getRef()) = firstWrapper.get();

        ASSERT_EQ (balanceCRCs, 1);
        firstWrapper->checkRefCount();
        ASSERT_EQ (firstWrapper.get(), secondWrapper.get());
    }
}

/**
 * Test two instances of WrappedComPtr assigning one to the other.
 */
TEST_F (TestWrappedComPtr, assignment)
{
    {
        WrappedComPtr<ComRefCheck> firstWrapper;
        WrappedComPtr<ComRefCheck> secondWrapper;
        CreateComRefCheckInstance (secondWrapper.getRef());

        firstWrapper = secondWrapper;
        ASSERT_EQ (balanceCRCs, 1);
        firstWrapper->checkRefCount();
        ASSERT_EQ (firstWrapper.get(), secondWrapper.get());

        secondWrapper.reset();
        ASSERT_EQ (balanceCRCs, 1);
        firstWrapper->checkRefCount();

        CreateComRefCheckInstance (secondWrapper.getRef());
        ASSERT_EQ (balanceCRCs, 2);

        firstWrapper = secondWrapper;
        ASSERT_EQ (balanceCRCs, 1);
        firstWrapper->checkRefCount();
        ASSERT_EQ (firstWrapper.get(), secondWrapper.get());
    }
}
