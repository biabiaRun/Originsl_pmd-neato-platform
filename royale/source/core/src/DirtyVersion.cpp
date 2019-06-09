/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/DirtyVersion.hpp>

#ifdef ROYALE_VERSION_DIRTY
# ifdef ROYALE_TARGET_PLATFORM_WINDOWS

#include <Windows.h>
#include <royale/String.hpp>

static bool isExcepted(royale::String target)
{
    TCHAR buf[128];
    DWORD len = 128;
    if (!GetUserName (buf, &len))
    {
        return false;
    }

    return (target == buf);
}

DWORD WINAPI threadedMessageBox (LPVOID)
{
    MessageBox (0, "This is an unversioned, untested, unofficial build of Royale.\n\nWarning:\n* Only for internal use\n* Only for testing\n* No support\n* No further modifications possible\n* EYE SAFETY NOT VERIFIED\n\nThis window closes automatically after 5 seconds.", "Dirty Royale Version", MB_OK | MB_ICONWARNING);
    return 0;
}
# endif
#endif


void royale::common::showDirtyVersionWarning ()
{
#ifdef ROYALE_VERSION_DIRTY
# ifdef ROYALE_TARGET_PLATFORM_WINDOWS
    if (!isExcepted (DIRTY_EXCEPTION))
    {
        // We need to use Windows threads, because C++ std::threads can't be terminated easily.
        DWORD threadId;
        HANDLE msgThread = CreateThread (NULL, 0, threadedMessageBox, NULL, 0, &threadId);

        DWORD result = WaitForSingleObject(msgThread, 5000);

        switch (result)
        {
            case WAIT_OBJECT_0:
                break;
            case WAIT_ABANDONED:
            case WAIT_TIMEOUT:
            case WAIT_FAILED:
                TerminateThread(msgThread, 0);
                break;
        }
    }
# endif
#endif
}

