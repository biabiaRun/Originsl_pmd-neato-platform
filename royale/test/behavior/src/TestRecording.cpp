#include "catch.hpp"
#include "Helper.hpp"
#include "ContextCameraDevice.hpp"
#include <MsvcMacros.hpp>

#include <chrono>
#include <thread>
#include <cstdio>

using namespace royale::bdd;
using namespace royale;
using namespace std;

//Scenario: Record and replay a depth data file
//  Given I started capturing with the fastest usecase
//  And I start the record mode
//  And I sleep for 10 seconds
//  When I stop the record mode
//  Then I want a record file to be created
//  Given I have a depth data record file
//  And I start the replay mode
//  When I calculate the FPS over 5 seconds
//  Then I want the framerate to differ by plusminus 1 FPS from the recorded framerate
SCENARIO ("Record and replay a depth data file", "[record]")
{
    LOG_OUTPUT ("Record and replay a depth data file\r\n");
    std::string recordFileName = ANDROID_OUTPUT_DIRECTORY + std::string ("royale_bdd_record.rrf");
    ContextCameraDevice context;
    GIVEN ("I started capturing with the fastest usecase")
    {
        std::remove (recordFileName.c_str());
        CHECK_ROYALE_SUCCESS (context.connect());
        CHECK_ROYALE_SUCCESS (context.setUseCaseFastest());
        CHECK_ROYALE_SUCCESS (context.registerDataListener());
        CHECK_ROYALE_SUCCESS (context.startCapture());
        AND_THEN ("I start the record mode")
        {
            CHECK_ROYALE_SUCCESS (context.startRecording (recordFileName));
            AND_THEN ("I sleep for 10 seconds")
            {
                std::this_thread::sleep_for (std::chrono::seconds (10));
                WHEN ("I stop the record mode")
                {
                    CHECK_ROYALE_SUCCESS (context.stopRecording());
                    THEN ("I want a record file to be created")
                    {
                        REQUIRE (context.fileExists (recordFileName));
                        GIVEN ("I have a depth data record file")
                        {
                            REQUIRE (context.fileExists (recordFileName));
                            CHECK_ROYALE_SUCCESS (context.destroyCamera());
                            CHECK_ROYALE_SUCCESS (context.setPlaybackFile (recordFileName));
                            AND_THEN ("I start the replay mode")
                            {
                                CHECK_ROYALE_SUCCESS (context.connect());
                                CHECK_ROYALE_SUCCESS (context.registerDataListener());
                                CHECK_ROYALE_SUCCESS (context.startCapture());
                                WHEN ("I calculate the FPS over 5 seconds")
                                {
                                    context.measureAverageFrameRate (5);
                                    THEN ("I want the framerate to differ by plusminus 1 FPS from the recorded framerate")
                                    {
                                        unsigned int difference = 1u;
                                        unsigned int minFps, maxFps, fps, sequence_dummy, exposure_dummy;

                                        sscanf_royale_v3 (context.getLastUseCaseName().c_str(), "MODE_%d_%dFPS_%d", &sequence_dummy, &fps, &exposure_dummy);

                                        std::cout << "Last FPS: " << fps << std::endl;

                                        minFps = fps - difference;
                                        maxFps = fps + difference;

                                        float averageFps = context.getAverageFps();

                                        std::cout << "Average FPS is " << averageFps << std::endl;
                                        REQUIRE (averageFps >= minFps);
                                        REQUIRE (averageFps <= maxFps);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
