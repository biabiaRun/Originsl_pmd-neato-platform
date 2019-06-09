/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <pal/IStorageWriteFullOverwrite.hpp>

#include <memory>

namespace royale
{
    namespace pal
    {
        /**
         * This interface can be implemented by a class which owns an IStorageWriteFullOverwrite,
         * and returns access to that IStorageWriteFullOverwrite.  For example, a concrete
         * implementation of INonVolatileStorage may allow the underlying access to be retrieved.
         */
        class IStorageAccessUnderlying
        {
        public:
            virtual ~IStorageAccessUnderlying() = default;

            /**
             * If the callee allows access to the storage that it wraps, returns a non-nullptr to
             * the storage.  May return nullptr even though the callee implements
             * IStorageAccessUnderlying, which should be interpreted as meaning that write access is
             * not available.
             */
            virtual std::shared_ptr<pal::IStorageWriteFullOverwrite> getUnderlyingWriteAccess() = 0;
        };
    }
}

