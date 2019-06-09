/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef ROYALE_TARGET_PLATFORM_WINDOWS
#include <WinError.h>
#include <Shlobj.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <errno.h>
#endif

#ifdef ROYALE_TARGET_PLATFORM_ANDROID
#include <QAndroidJniObject>
#endif

namespace royaleviewer
{
    /**
     * Retrieves an output path for all systems (without trailing slash).
     * If an error occurs (e.g. the royale subfolder can't be created)
     * the return value is empty.
     * - Windows : DocumentsFolder/royale
     * - Linux : HomeFolder/royale
     * - OSX : Homefolder/royale
     * - Android : ExternalStorageDirectory/royale
     * @return path.
     */
    inline std::string getOutputPath()
    {
#ifdef ROYALE_TARGET_PLATFORM_WINDOWS
        TCHAR szPath[MAX_PATH];

        if (SUCCEEDED (SHGetFolderPathAndSubDir (NULL,
                       CSIDL_MYDOCUMENTS | CSIDL_FLAG_CREATE,
                       NULL,
                       SHGFP_TYPE_CURRENT,
                       TEXT ("royale"),
                       szPath)))
        {
            return std::string (szPath);
        }
        else
        {
            return "";
        }
#elif ROYALE_TARGET_PLATFORM_ANDROID

        QAndroidJniObject sdDir = QAndroidJniObject::callStaticObjectMethod ("android/os/Environment", "getExternalStorageDirectory", "()Ljava/io/File;");
        QAndroidJniObject sdPathObj = sdDir.callObjectMethod ("getAbsolutePath", "()Ljava/lang/String;");
        std::string sdPath = sdPathObj.toString().toStdString();

        {
            DIR *dir = opendir (sdPath.c_str ());
            if (!dir)
            {
                return "";
            }
        }
        sdPath += "/royale";

        DIR *dir = opendir (sdPath.c_str());

        if (dir)
        {
            // Directory exists
            closedir (dir);
        }
        else
        {
            int res = mkdir (sdPath.c_str(), S_IRWXU | S_IRWXO | S_IRWXG);
            if (res != 0)
            {
                return "";
            }
        }
        return sdPath;
#else
        struct passwd *pw = getpwuid (getuid());

        if (pw == NULL)
        {
            return "";
        }

        std::string homePath = pw->pw_dir;
        homePath += "/royale";

        DIR *dir = opendir (homePath.c_str());

        if (dir)
        {
            // Directory exists
            closedir (dir);
        }
        else
        {
            int res = mkdir (homePath.c_str(), S_IRWXU | S_IRWXO | S_IRWXG);
            if (res != 0)
            {
                return "";
            }
        }
        return homePath;
#endif

    }
}