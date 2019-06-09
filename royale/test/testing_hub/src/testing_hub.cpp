/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "rs232.h"
#include "royale.hpp"
#include "royale/ICameraDevice.hpp"
#include "royale/CameraManager.hpp"

#include <PlatformResources.hpp>

using namespace std;
using namespace royale;

enum class CommandState
{
    SendCommandString,
    ActivatePin,
    RunRoyaleTests,
    PrepareNextPin
};

string nameTest;                                    //!< the name of the test which should be run on the USB hub
int activePin = 1;                                  //!< current pin/relay/camera which gets to turned on/off
const int sleepTime = 150;                          //!< the sleep time between each CommandState
const int sleepTimeComport = 150;                   //!< the sleep time befor checking next comport if current comport wasnt connected to microcontroller XMC 4500
const int noCameraWaitTime = 15000;                 //!< the sleep time befor the programm moves to the next pin/relay/camera if no camera is connected - 15 seconds because it might take some time to install camera drivers
bool lookForMircocontroller = true;                 //!< a loop to find the port for the microcontroller XMC 4500
bool switchRelays = false;                          //!< a loop which switches the relays on/off thus switching the cameras on/off
ofstream resultsFile;                               //!< used to create a file
const string signalMicrocontroller = "A";           //!< a string which is used to find the microcontroller XMC 4500
const string commandStringMicrocontroller = "a";    //!< a string which has to be sent befor sending a pin/relay/camera number

const int microcontrollerReply = 10;                //!< when the programm recieves over x (default 20) bytes from the current COM port it schouold be the microcontroller XMC 4500
int testSuccess;                                    //!< when running the tests the function system() returns a value which gets saved in this variable and is the printed.
int folderSuccess;                                  //!< when creating a folder the function system() returns a value which gets saved in this variable and is the printed.
const int maxUSBPorts = 6;
CommandState currentCommandState = CommandState::SendCommandString;

struct stat info;
string systemCommand;                               //!< the sequence of characters that will be sent to the microcontroller XMC 4500
string resultsFolder;                               //!< path of the folder in which the results of the test will be saved in

#ifdef _WIN32
int highestPort_nr = 18;                            //!< highest port number to which the microcontroller XMC 4500 can be connected
#else
int highestPort_nr = 39;                            //!< highest port number to which the microcontroller XMC 4500 can be connected
#endif

// rs232 variables
int cport_nr = 0;                                   //!< port number to which the microcontroller XMC 4500 is connected - leave this at 0 since programm automatically detects XMC 4500 by trying to communicate to it
int bdrate = 9600;                                  //!< baud-rate of communication port to microcontroller XMC 4500

char mode[] = { '8', 'N', '1', 0 };                 //!< mode which the serial connectin should have - '8' Databits - 'None' Parity - '1' Stopbits - No Idea, just dont change this because it works
char sendBytes[512];                                //!< sends bytes to determin which pin/relay/camera to switch on/off
char sendSignal[512];                               //!< bytes which are sent to microcontroller XMC 4500

unsigned char buf[4096];                            //!< actual data of the recieved bytes from the microcontroller XMC 4500

void sendString (const string &stringValue)
{
    RS232_cputs (cport_nr, stringValue.c_str());
}

void runTestRoyaleBDD (const string &name, const string &id)
{
    string fileName = name + "_" + id + ".txt";
#ifdef _WIN32
    systemCommand = nameTest + ".exe" + " --out " + resultsFolder + "\\" + fileName;
#else
    systemCommand = "./bin/" + nameTest + " --out " + resultsFolder + "/" + fileName;
#endif
    testSuccess = system (systemCommand.c_str());                   //!< running royale BDD test
    printf ("%d", testSuccess);
}

const std::string getLogTime (const std::string &dateTimeFormat)
{
    time_t mytime;
    mytime = time (NULL);

#ifdef _WIN32
    struct tm _timeStruct;
    localtime_s (&_timeStruct, &mytime);
#endif

    char mbstr[100];
    memset (&mbstr, 0, sizeof (mbstr));

#ifdef _WIN32
    std::strftime (mbstr, sizeof (mbstr), dateTimeFormat.c_str(), &_timeStruct);
#else
    std::strftime (mbstr, sizeof (mbstr), dateTimeFormat.c_str(), std::localtime (&mytime));
#endif
    return mbstr;
}

void createResultsFolder()
{
#ifndef _WIN32
    resultsFolder = "results";
    if (0 != stat (resultsFolder.c_str(), &info))                   //Make dir if it doesnt exist
    {
        printf ("\n Creating new folder: %s\n", resultsFolder.c_str());
        systemCommand = "mkdir " + resultsFolder;
        folderSuccess = system (systemCommand.c_str());             //Creating dir
        printf ("%d", folderSuccess);
    }
#endif
    string LocalDateTime = getLogTime ("testing_hub_results_%Y_%m_%d_Time_%H_%M_%S");
    resultsFolder = "results/" + LocalDateTime;
    if (0 == stat (resultsFolder.c_str(), &info))                   //Exit because folder already exists - tests are already running
    {
        return;
    }
    if (0 != stat (resultsFolder.c_str(), &info))                   //Make dir if it doesnt exist
    {
        printf ("\n Creating new folder: %s\n", resultsFolder.c_str());
#ifdef _WIN32
        resultsFolder = "results\\" + LocalDateTime;
        systemCommand = "mkdir " + resultsFolder;
        system (systemCommand.c_str());                             //Creating dir
#else
        resultsFolder = "results/" + LocalDateTime;
        mkdir (resultsFolder.c_str(), 0777);
#endif
        //Creating dir
    }
}

// checking if there is a serial port to connect to
void searchForMicrocontroller()
{
    int receivedBytes; //!< aproximate number of bytes the microcontroller XMC 4500 recieves after sending signalMicrocontroller

#ifdef _WIN32                   // Windows

    while (lookForMircocontroller)
    {
        sendString (signalMicrocontroller);
        receivedBytes = RS232_PollComport (cport_nr, buf, 4095);
        if (0 == receivedBytes)
        {
            if (RS232_OpenComport (cport_nr, bdrate, mode))
            {
                printf ("\n Can not connect to microcontroller, trying on port  %u\n\n", cport_nr + 1);
                cport_nr++;
            }
        }
        if (highestPort_nr < cport_nr)
        {
            printf ("\n Can not connect to microcontroller! Please check if everything is connected correctly and start the Program again! Closing...");
            printf ("\n===============================================================================\n");
            return;
        }
        if (microcontrollerReply < receivedBytes)
        {
            printf ("\n Port %u / COM Port %u has correct microcontroller! Proceeding to test...\n", cport_nr, cport_nr + 1);
            printf ("\n===============================================================================\n");
            receivedBytes = 0;
            lookForMircocontroller = false;
            switchRelays = true;
        }
        std::this_thread::sleep_for (std::chrono::milliseconds (sleepTimeComport));
    }
#else                   // Linux

    while (RS232_OpenComport (cport_nr, bdrate, mode))
    {
        cport_nr++;
        if (highestPort_nr <= cport_nr)
        {
            printf ("\n Can not connect to microcontroller! Please check if everything is connected correctly and start the Program again! Closing...");
            printf ("\n===============================================================================\n");
            return;
        }
        printf ("\n Can not connect to microcontroller, trying on port  %u\n\n", cport_nr);
        std::this_thread::sleep_for (std::chrono::milliseconds (sleepTimeComport));
    }
    while (lookForMircocontroller)
    {
        printf ("===============================================================================\n");
        sendString (signalMicrocontroller);
        std::this_thread::sleep_for (std::chrono::milliseconds (sleepTimeComport));
        receivedBytes = RS232_PollComport (cport_nr, buf, 4095);
        if (0 == receivedBytes)
        {
            printf ("\n Found a communication port! It doesnt seem to be the correct microcontroller! Please disconnect it and start the programm again! Closing... \n");
            printf ("\n===============================================================================\n");
        }
        if (microcontrollerReply < receivedBytes)
        {
            printf ("\n Port %u / COM Port %u has correct microcontroller! Proceeding to test...\n", cport_nr, cport_nr + 1);
            printf ("\n===============================================================================\n");
            receivedBytes = 0;
            lookForMircocontroller = false;
            switchRelays = true;
        }
    }
#endif
}

int main (int argc, char *argv[])
{
    sample_utils::PlatformResources resources;

    for (int count = 0; count < argc; count++)
    {
        switch (count)
        {
            case 1:
                {
                    nameTest = argv[1];
                }
                break;

            default:
                {
                    nameTest = "test_royale_bdd";
                    if (count > 1)
                    {
                        printf ("Too many arguments! Please give max 1 argument. Closing...");
                        return -1;
                    }
                }
                break;
        }
    }

    createResultsFolder();
    resultsFile.open ("results/testing_hub_protocol.txt");

    int cameraCounter = 0; //!< a counter which decides if no camera or too many cameras are connected
    CameraManager *manager = new CameraManager();
    Vector<String> cameras = manager->getConnectedCameraList();
    size_t size = cameras.size();

    resultsFile << "===============================================================================\n";
    printf ("\n===============================================================================\n");

    searchForMicrocontroller();

    while (switchRelays)
    {
        Vector<String> cameras = manager->getConnectedCameraList();
        size = cameras.size();
        resultsFile.flush();

        switch (currentCommandState)
        {
            case CommandState::SendCommandString:
                {
                    sendString (commandStringMicrocontroller);
                    currentCommandState = CommandState::ActivatePin;
                }
                break;

            case CommandState::ActivatePin:
                {
#ifdef _WIN32
                    sprintf_s (sendBytes, "%d", activePin);
#else
                    sprintf (sendBytes, "%d", activePin);
#endif
                    sendString (sendBytes); // activate relais
                    printf ("\n Switch camera %s on...\n", sendBytes);
                    printf ("\n-------------------------------------------------------------------------------\n");
                    currentCommandState = CommandState::RunRoyaleTests;
                }
                break;

            case CommandState::RunRoyaleTests:
                {
                    if (1 == size)
                    {
                        String name;
                        String id = "";
                        {
                            std::unique_ptr<ICameraDevice> camera = manager->createCamera (cameras[0]);
                            if (nullptr != camera)
                            {
                                auto statusId = camera->getId (id);
                                auto statusName = camera->getCameraName (name);
                                resultsFile << "\n Camera detected on port: " << activePin << "\n";
                                if (statusId != CameraStatus::SUCCESS || statusName != CameraStatus::SUCCESS)
                                {
                                    resultsFile << " Error getting name: " << uint32_t (statusName) << " - number: " << uint32_t (statusId) << " \n";
                                }
                                resultsFile << " Camera: " << name.c_str() << " - Number: " << id.c_str() << " \n";
                                resultsFile << "\n-------------------------------------------------------------------------------\n";
                                resultsFile.flush();
                                printf ("\n Camera detected: \n");
                                printf ("Camera: %s - Number: %s ", name.c_str(), id.c_str());
                            }
                        }

                        if (id.length())
                        {
                            printf ("\n Starting Test!\n");
                            printf ("\n-------------------------------------------------------------------------------\n");
                            runTestRoyaleBDD (name.toStdString(), id.toStdString());
                            printf ("\n-------------------------------------------------------------------------------\n");
                            printf ("\n Test is done for camera: %s - %s ", name.c_str(), id.c_str());
                            resultsFile << "\n Test is done for camera: " << name.c_str() << " - " << id.c_str() << "\n";
                            resultsFile << "\n===============================================================================\n";
                            resultsFile.flush();
                            cameraCounter = 0;
                            currentCommandState = CommandState::PrepareNextPin;
                        }
                    }
                    if ( (0 == size) || (1 < size))
                    {
                        cameraCounter++;
                    }

                    if ( (noCameraWaitTime / sleepTime) < cameraCounter)
                    {
                        resultsFile << "\n No camera or more than one camera connected on port " << activePin << "! Proceeding without executing test... \n";
                        resultsFile << "\n===============================================================================\n";
                        printf ("\n No camera or more than one camera connected! Proceeding without executing test... \n");
                        cameraCounter = 0;
                        currentCommandState = CommandState::PrepareNextPin;
                    }
                }
                break;

            case CommandState::PrepareNextPin:
                {
                    printf ("\n-------------------------------------------------------------------------------\n");
                    if (maxUSBPorts > activePin)
                    {
                        activePin++;
                        printf ("\n===============================================================================\n");
                        printf ("\n Starting next camera: \n");
                        printf ("\n===============================================================================\n");
                    }
                    else
                    {
                        printf ("\n===============================================================================\n");
                        printf ("\nNo Camera avalible! Testing done! Ending Program... \n");
                        printf ("\n===============================================================================\n");
                        switchRelays = false;
                        RS232_CloseComport (cport_nr);
                        return 0;
                    }
                    currentCommandState = CommandState::SendCommandString;
                }
                break;

            default:
                return -2;
                break;
        }
        std::this_thread::sleep_for (std::chrono::milliseconds (sleepTime));
    }
    sendString ("D"); // turn off all relais
}
