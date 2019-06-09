/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <royale.hpp>
#include <royale/IDepthDataListener.hpp>
#include <royale/IReplay.hpp>
#include <string>
#include <thread>
#include "ISystemInfo.hpp"

#ifdef WIN32
#include "WinSystemInfo.hpp"
#elif __linux__ // incl. Linux and Android
#include "LinuxSystemInfo.hpp"
#endif

class SysInfoTool : public royale::IDepthDataListener
{
public:
    SysInfoTool (const std::string &playbackFilename,
                 const std::string &findFilePath,
                 int seconds);
    ~SysInfoTool();

private:
    void onNewData (const royale::DepthData *data) override;

    /**
    *  The control part of this tool
    */
    void showControlMenu();

    /**
    *  Output the information of the queried RRF file
    *
    *  @param processID: the number of the RRF file being listed
    */
    void showRRFInfo (int rrfNo);

    /**
    *  Find all RRF files under the target folder
    *
    *  @param path: the path of the target folder
    *  @param format: the file type, here is RRF file
    *  @param files: the list of full paths of the found RRF fiels
    *  @param names: the list of file names of the found RRF fiels
    *  @param paths: the list of paths of the found RRF fiels
    */
    void getRRFFiles (std::string path, const char *format, std::vector<std::string> &files, std::vector<std::string> &names, std::vector<std::string> &paths);

    /**
    *  Check the playback time, the minimum is not less than 10 seconds
    *
    *  @param seconds: the seconds of the playback time
    *
    *  @return the checked playback time (in seconds)
    */
    int checkPlaybackTime (int seconds);

    /**
    *  Change the target folder to search the RRF files
    */
    void changeDirectory();

    /**
    *  Call the method getRRFFiles() to find RRF files and show the prompt information
    */
    void searchRRFfiles();

    /**
    *  Get the char in Linux and run the program without enter key
    */
    char getch();

private:
    std::unique_ptr<royale::ICameraDevice> m_cameraDevice;
    std::string                            m_playbackFilename;
    int                                    m_seconds;

    const char                             *m_findFileFormat;
    std::string                            m_findFilePath;
    std::vector<std::string>               m_files;
    std::vector<std::string>               m_fileNames;
    std::vector<std::string>               m_filePaths;

    ISystemInfo                            *m_systemInfo;
    std::vector<uint16_t>                  m_framesCount;
    royale::Vector<royale::StreamId>       m_streamIds;
};

