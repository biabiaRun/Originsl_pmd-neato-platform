/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include "SysInfoTool.hpp"
#include <stdio.h>

#ifdef WIN32
#include <conio.h>
#include <direct.h>
#include <io.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#elif __linux__ // incl. Linux and Android
#include <termios.h>
#include <dirent.h>
#include <unistd.h>
#include <regex>
#endif

using namespace std;
using namespace royale;

SysInfoTool::SysInfoTool (const string &playbackFilename,
                          const string &findFilePath,
                          int seconds) :
    m_playbackFilename (playbackFilename),
    m_findFilePath (findFilePath)
{
#ifdef WIN32
    m_systemInfo = new WinSystemInfo();
    m_findFileFormat = ".rrf";
#elif __linux__ // incl. Linux and Android
    m_systemInfo = new LinuxSystemInfo();
    m_findFileFormat = ".*.rrf";
#endif

    m_seconds = checkPlaybackTime (seconds);

    if (m_findFilePath.empty())
    {
#ifdef WIN32
        m_findFilePath = "D:";
#elif __linux__ // incl. Linux and Android
#ifdef TARGET_PLATFORM_ANDROID
        m_findFilePath = "/sdcard";
#else
        m_findFilePath = "/home";
#endif
#endif
    }

    if (!m_playbackFilename.empty())
    {
        m_files.resize (1);
        m_fileNames.resize (1);
        m_filePaths.resize (1);

#ifdef WIN32
        int pos = static_cast<int> (m_playbackFilename.find_last_of ('\\'));
#elif __linux__ // incl. Linux and Android
        int pos = static_cast<int> (m_playbackFilename.find_last_of ('/'));
#endif
        m_files[0] = m_playbackFilename;
        m_fileNames[0] = m_playbackFilename.substr (pos + 1);
        m_filePaths[0] = m_playbackFilename.substr (0, pos);
        showRRFInfo (0);
        exit (0);
    }
    else
    {
        cout << "Searching for all RRF files under '" << m_findFilePath << "'!" << endl;
        cout << "Please enter:" << endl;
        cout << "   'd'    to change the searched directory" << endl;
        cout << "   'q'    to quit" << endl;
        cout << "other key to continue searching" << endl;

        char c;
#ifdef WIN32
        c = _getch();
#elif __linux__ // incl. Linux and Android
        c = getch();
#else
#error SysInfoTool is not supported on this platform
#endif

        if (c == 'q')
        {
            exit (0);
        }
        else
        {
            if (c == 'd')
            {
                changeDirectory();

#ifdef __linux__ // incl. Linux and Android
                getch();
#endif
            }
            searchRRFfiles();
        }
    }
    showControlMenu();
}

SysInfoTool::~SysInfoTool()
{
}

#ifdef __linux__ // incl. Linux and Android
char SysInfoTool::getch()
{
    char ch;
    struct termios oldt, newt;

    tcgetattr (STDIN_FILENO, &oldt);
    memcpy (&newt, &oldt, sizeof (newt));
    newt.c_lflag &= ~ (ECHO | ICANON | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
    tcsetattr (STDIN_FILENO, TCSANOW, &newt);
    ch = static_cast<char> (getchar());
    tcsetattr (STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}
#endif

void SysInfoTool::onNewData (const DepthData *data)
{
    for (auto id = 0u; id < m_streamIds.size(); id++)
    {
        if (data->streamId == m_streamIds[id])
        {
            m_framesCount[id]++;
        }
    }
}

int SysInfoTool::checkPlaybackTime (int seconds)
{
    int checkedSeconds;
    if (seconds == 0)
    {
        checkedSeconds = 20;
    }
    else if (seconds < 10)
    {
        checkedSeconds = 10;
    }
    else
    {
        checkedSeconds = seconds;
    }
    return checkedSeconds;
}

void SysInfoTool::changeDirectory()
{
    cout << endl;
    cout << "Please enter 'the path of directory' and 'Enter' to continue:" << endl;
    char dir[100];
    cin >> dir;
    m_findFilePath = dir;
    cout << endl;
    cout << "The current searched directory is '" << m_findFilePath << "'." << endl;
}

void SysInfoTool::searchRRFfiles()
{
    cout << endl;
    cout << "Searching for RRF files..." << endl;
    getRRFFiles (m_findFilePath, m_findFileFormat, m_files, m_fileNames, m_filePaths);
    cout << "Searching is finished!" << endl;
}

void SysInfoTool::showControlMenu()
{
    while (1)
    {
        if ( (int) m_files.size() == 0)
        {
            cout << endl;
            cout << "No files found in '" << m_findFilePath << "'!" << endl;
            exit (0);
        }

        cout << endl;
        cout << "Royale Replay File:" << endl;
        for (auto i = 0u; i < m_files.size(); i++)
        {
            cout << i + 1 << ".\t" << m_files[i] << endl;
        }
        cout << endl;
        cout << "Please enter:" << endl;
        cout << "'f' and then the No. to check the file" << endl;
        cout << "'t' to change the playback time" << endl;
        cout << "    (Min. is 10 seconds. Current is " << m_seconds << " seconds)" << endl;
        cout << "'d' to change the search directory" << endl;
        cout << "'q' to quit" << endl;

        char c;
#ifdef WIN32
        c = _getch();
#elif __linux__ // incl. Linux and Android
        c = getch();
#endif

        if (c == 't')
        {
            cout << endl;
            cout << "Please enter 'the playback time (in seconds)' and 'Enter' to continue:" << endl;
            char time[5];
            cin >> time;
            m_seconds = checkPlaybackTime (atoi (time));
            cout << endl;
            cout << "The current playback time is " << m_seconds << " seconds." << endl;

#ifdef __linux__ // incl. Linux and Android
            c = getch();
#endif
        }
        else if (c == 'd')
        {
            changeDirectory();
            m_files.clear();
            m_fileNames.clear();
            m_filePaths.clear();
            searchRRFfiles();

#ifdef __linux__ // incl. Linux and Android
            c = getch();
#endif
        }
        else if (c == 'q')
        {
            exit (0);
        }
        else if (c == 'f')
        {
            cout << endl;
            cout << "Please enter 'the No. of RRF file' and 'Enter' to continue:" << endl;
            char file[5];
            cin >> file;
            int num = atoi (file);
            if (num == 0 || num > (int) m_files.size())
            {
                cout << "Wrong input, try again!" << endl;
            }
            else
            {
#ifdef __linux__ // incl. Linux and Android
                c = getch();
#endif
                int rrfNo = num - 1;
                showRRFInfo (rrfNo);
            }
        }
        else
        {
            cout << endl;
            cout << "Wrong input, try again!" << endl;
        }
    }
}

void SysInfoTool::showRRFInfo (int rrfNo)
{
    cout << "Checking for this RRF file..." << endl;
    CameraManager manager;
    m_cameraDevice = manager.createCamera (m_files[rrfNo]);
    m_cameraDevice->initialize();
    m_cameraDevice->registerDataListener (this);

    auto replayControl = dynamic_cast<IReplay *> (m_cameraDevice.get());
    uint32_t numFrames = replayControl->frameCount();

    Vector<StreamId> streamIds;
    m_cameraDevice->getStreams (streamIds);
    m_framesCount.resize (streamIds.size());
    m_streamIds = streamIds;

    for (auto id = 0u; id < streamIds.size(); id++)
    {
        m_framesCount[id] = 0;
    }

    vector<double> meanFPS;
    meanFPS.resize (streamIds.size());

    m_cameraDevice->startCapture();

    this_thread::sleep_for (chrono::seconds (1));
    m_systemInfo->getProcCpuUsage();

    this_thread::sleep_for (chrono::seconds (m_seconds));
    double procCpuUsage = m_systemInfo->getProcCpuUsage();

    if (m_cameraDevice)
    {
        m_cameraDevice->stopCapture();

        for (auto id = 0u; id < streamIds.size(); ++id)
        {
            meanFPS[id] = (int) (1.0 * m_framesCount[id] / (m_seconds + 1) * 100 + 0.5) / 100.0;
        }
        double meanCPU = (int) (1.0 * procCpuUsage * 100 + 0.5) / 100.0;

        cout << "Checking is finished!" << endl;
        cout << endl << endl;
        cout << "Royale Replay File: " << m_fileNames[rrfNo] << endl;
        cout << "file path:          " << m_filePaths[rrfNo] << endl;
        cout << "total frames:       " << numFrames << endl;
        cout << "mean FPS:           ";
        for (auto id = 0u; id < streamIds.size(); id++)
        {
            cout << meanFPS[id] << " by streamID " << streamIds[id] << endl;
            if (id < (streamIds.size() - 1))
            {
                cout << "                    ";
            }
        }
        cout << "mean CPU load:      " << meanCPU << "% per second" << endl;

        cout << endl << endl;
        cout << "Please enter:" << endl;
        cout << "   'q'    to quit" << endl;
        cout << "other key to continue" << endl;

        char c;
#ifdef WIN32
        c = _getch();
#elif __linux__ // incl. Linux and Android
        c = getch();
#endif

        if (c == 'q')
        {
            exit (0);
        }
    }
    else
    {
        cout << endl;
        cout << "Cannot replay file" << endl;
    }
}

void SysInfoTool::getRRFFiles (string path, const char *format, vector<string> &files, vector<string> &names, vector<string> &paths)
{
#ifdef WIN32
    intptr_t hFile = 0;
    struct _finddata_t fileinfo;

    string p;
    if ( (hFile = _findfirst (p.assign (path).append ("\\*").c_str(), &fileinfo)) != -1)
    {
        do
        {
            if (fileinfo.attrib & _A_SUBDIR)
            {
                if (strcmp (fileinfo.name, ".") != 0 && strcmp (fileinfo.name, "..") != 0)
                {
                    getRRFFiles (p.assign (path).append ("\\").append (fileinfo.name), format, files, names, paths);
                }
            }
            else
            {
                string fileName (fileinfo.name);
                string token = fileName.substr (0, fileName.find ("_"));
                char *fileFormat = PathFindExtension (fileinfo.name);
                if (strcmp (fileFormat, format) == 0 && token == "royale")
                {
                    files.push_back (p.assign (path).append ("\\").append (fileinfo.name));
                    names.push_back (fileName);
                    paths.push_back (p.assign (path));
                }
            }
        }
        while (_findnext (hFile, &fileinfo) == 0);
        _findclose (hFile);
    }

#elif __linux__ // incl. Linux and Android
    DIR *dir = opendir (path.c_str());
    regex reg_obj (format, regex::icase);

    string p;
    if (dir != NULL)
    {
        struct dirent *ptr;
        while ( (ptr = readdir (dir)) != NULL)
        {
            if (ptr->d_name[0] != '.')
            {
                if (ptr->d_type == DT_DIR)
                {
                    getRRFFiles (p.assign (path).append ("/").append (ptr->d_name), format, files, names, paths);
                }
                else
                {
                    string fileName (ptr->d_name);
                    string token = fileName.substr (0, fileName.find ("_"));
                    if (regex_match (ptr->d_name, reg_obj) && token == "royale")
                    {
                        files.push_back (p.assign (path).append ("/").append (ptr->d_name));
                        names.push_back (ptr->d_name);
                        paths.push_back (p.assign (path));
                    }
                }
            }
        }
        closedir (dir);
    }
#endif
}
