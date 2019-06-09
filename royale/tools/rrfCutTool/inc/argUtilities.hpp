/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <cstdlib>
#include <cstring>

namespace args
{
    bool str_equal (char const *str0, char const *str1)
    {
        return strcmp (str0, str1) == 0;
    }

    bool index_of (char const *arg, int const argc, char const **argv, int &index)
    {
        for (auto i = 0; i < argc; i++)
        {
            if (str_equal (arg, argv[i]))
            {
                index = i;
                return true;
            }
        }

        return false;
    }

    bool contains (char const *arg, int const argc, char const **argv)
    {
        for (auto i = 0; i < argc; i++)
        {
            if (str_equal (arg, argv[i]))
            {
                return true;
            }
        }

        return false;
    }

    bool may_param (char const *arg)
    {
        return arg[0] != '-';
    }
}
