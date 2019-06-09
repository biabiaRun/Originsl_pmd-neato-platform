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

#include <royale/String.hpp>
#include <royale/Vector.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fstream>
#include <istream>
#include <iostream>
#include <stdlib.h>

#include <common/MsvcMacros.hpp>

#include <common/exceptions/CouldNotOpen.hpp>

#if defined(ROYALE_TARGET_PLATFORM_WINDOWS)
#include <WinError.h>
#include <Shlobj.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <errno.h>
#endif



namespace royale
{
    namespace common
    {
        inline uint64_t getFileSize (const royale::String &filename);
    }
}

namespace
{
    //template<template<class> class VecType, class T>
    template<typename Container>
    inline uint64_t readFileToAnyVector (const royale::String &filename, Container &data)
    {
        uint64_t length = royale::common::getFileSize (filename);

        if (!length)
        {
            return 0;
        }

        std::ifstream f (filename.c_str(), std::ios_base::in | std::ios_base::binary);
        if (!f.is_open())
        {
            return 0;
        }

        if (length % sizeof (typename Container::value_type) != 0)
        {
            return 0;
        }

        data.resize (static_cast<size_t> (length) / sizeof (typename Container::value_type));
        f.read ( (char *) &data[0], static_cast<std::streamsize> (length));
        f.close();

        return length;
    }
}

namespace royale
{
    namespace common
    {
        /**
         * Checks if a file exists.
         * @param filename Filename of the file.
         * @return true if it exists, false if not.
         */
        inline bool fileexists (const royale::String &filename)
        {
            if (filename.empty ())
            {
                return false;
            }

            struct stat_royale info;
            return stat_royale (filename.c_str(), &info) == 0;
        }


        /**
         * Checks if a folder exists.
         * @param folder Name of the folder.
         * @return true if it exists, false if not.
         */
        inline bool folderExists (const royale::String &folder)
        {
            struct stat info;

            if (stat (folder.c_str (), &info) != 0)
            {
                return false;
            }
            else if (info.st_mode & S_IFDIR)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        /**
        * Retrieves the size of a given file.
        * @param filename Filename of the file.
        * @return Size of the file.
        */
        inline uint64_t getFileSize (const royale::String &filename)
        {
            std::ifstream inputfile;
            inputfile.open (filename.c_str(), std::ios_base::in | std::ios_base::binary);
            if (!inputfile.is_open())
            {
                return 0;
            }

            // get length of file:
            inputfile.seekg (0, std::ios::end);
            uint64_t length = static_cast<uint64_t> (inputfile.tellg());
            inputfile.seekg (0, std::ios::beg);

            inputfile.close();

            return length;
        }

        /**
        * Reads a file into a given Vector. The Vector will be resized according
        * to the size of the file and the template type.
        * @param filename Filename of the file.
        * @param data Vector into which the data will be read.
        * @return Number of bytes that were read.
        */
        template<class T>
        inline uint64_t readFileToVector (const royale::String &filename, royale::Vector<T> &data)
        {
            return readFileToAnyVector (filename, data);
        }

        template<class T>
        inline uint64_t readFileToStdVector (const royale::String &filename, std::vector<T> &data)
        {
            return readFileToAnyVector (filename, data);
        }

        /**
        * Writes the content of a vector into a file.
        * @param filename Filename of the file.
        * @param data Vector from which the data will be read.
        * @return Number of bytes that were read.
        */
        template<class T>
        inline uint64_t writeVectorToFile (const royale::String &filename, const royale::Vector<T> &data)
        {
            std::ofstream f (filename.c_str(), std::ios_base::out | std::ios_base::binary);
            if (!f.is_open())
            {
                return 0;
            }

            f.write ( (char *) &data[0], data.size() * sizeof (T));
            f.close();

            return data.size() * sizeof (T);
        }

        /**
         * On Android systems this will return the external storage path.
         * For all other systems it will just return an empty String.
         * @return External storage path (Android), Empty (all other systems)
         */
        inline royale::String getExternalStoragePath()
        {
#ifdef ROYALE_TARGET_PLATFORM_ANDROID
            return getenv ("EXTERNAL_STORAGE");
#else
            return "";
#endif
        }
    }
}
