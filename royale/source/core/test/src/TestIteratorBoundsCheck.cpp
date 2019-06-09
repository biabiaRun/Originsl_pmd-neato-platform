/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <gtest/gtest.h>
#include <common/IteratorBoundsCheck.hpp>
#include <common/exceptions/OutOfBounds.hpp>

#include <cstdint>
#include <vector>

using namespace royale::common;

TEST (TestIteratorBoundsCheck, PointerChecks)
{
    const auto src = std::vector<uint8_t> {0, 1, 2, 3, 4, 5};
    const uint8_t *begin = src.data();
    const uint8_t *end = begin + src.size();

    ASSERT_NO_THROW (iteratorBoundsCheck (begin, end, src.size()));
    // there's no reverse-iterator with pair pointers
    ASSERT_THROW (iteratorBoundsCheck (begin, end, src.size() + 1), OutOfBounds);
    // OutOfBounds is a subclass of LogicError, the next tests are lenient accepting any
    // kind of LogicError, and we might in future give a different LogicError subclass
    // for swapped start and end.
    ASSERT_THROW (iteratorBoundsCheck (end, begin, 0), LogicError);

    // iterator which is pointing to the first element
    const uint8_t *oneIter = begin + 1;

    ASSERT_NO_THROW (iteratorBoundsCheck (begin, oneIter, 1));
    ASSERT_NO_THROW (iteratorBoundsCheck (oneIter, end, src.size() - 1));
    ASSERT_THROW (iteratorBoundsCheck (oneIter, end, src.size()), OutOfBounds);

    // check that accessing an element works
    ASSERT_EQ (src[0], *iteratorBoundsCheck (begin, end, src.size()));
    // check the behaviour of the returned iterator, does it point to the same place?
    ASSERT_EQ (src.data(), &*iteratorBoundsCheck (begin, end, src.size()));
}
