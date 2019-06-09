/****************************************************************************\
* Copyright (C) 2016 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

namespace spectre
{
    namespace common
    {
        /// Template which holds a list of types (typelist)
        /**
         * The TypeList holds a list of types. It allows to traverse through the list
         * using the head typedef (which is the first type in the list), and the tail
         * typedef which defines a new type list without the current head.
         *
         * *NOTE:* For convenience the last entry of the TypeList still defines
         * a tail entry returning the same entry. To check for the last entry,
         * you have to use the static last constant.
         */
        template<typename HEAD, typename... Tail>
        struct TypeList
        {
            using head = HEAD;

            using tail = TypeList<Tail...>;

            static const bool last = false;
        };

        /// Typelist specialization for last entry
        template<typename HEAD>
        struct TypeList<HEAD>
        {
            using head = HEAD;

            using tail = TypeList<HEAD>;

            static const bool last = true;
        };
    }
}
