/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <string>
#include <sstream>
#include <stdint.h>
#include <royale/String.hpp>
#include <royale/Vector.hpp>

namespace royale
{
    namespace common
    {

        /**
         * Converts an arbitrary type to a standard string. The type must be supported by std::stringstream.
         * @param value The value to convert.
         * @return value as a std::string.
         */
        template <typename T>
        std::string toStdString (T value)
        {
            std::ostringstream os;
            os << value;
            return os.str();
        }


        /**
         * Strip string. Remove whitespace from the beginning and the end.
         * @param s The std::string to process.
         * @return The stripped std::string.
         */
        inline std::string stripString (std::string s)
        {
            if (s.empty())
            {
                return s;
            }

            while (!s.empty() && isspace (s[0]))
            {
                s = s.substr (1);
            }
            while (!s.empty() && isspace (s[s.length() - 1]))
            {
                s = s.substr (0, s.length() - 1);
            }
            return s;
        }

        /**
         * Takes the input string and checks if it starts
         * with one of the strings from the Vector.
         * @param stringToCheck String that should be checked.
         * @param stringVector Vector that contains possible string beginnings.
         * @return True if stringToCheck starts with one of the strings from stringVector.
         */
        inline bool stringStartsWith (const royale::String &stringToCheck,
                                      const royale::Vector<royale::String> &stringVector)
        {
            for (auto curString : stringVector)
            {
                if (stringToCheck.length() >= curString.length() &&
                        stringToCheck.compare (0, curString.length(), curString) == 0)
                {
                    return true;
                }
            }

            return false;
        }

    }
}
