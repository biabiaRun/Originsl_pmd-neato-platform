#include "catch.hpp"
#include "Helper.hpp"
#include "ContextCameraDevice.hpp"

#include <chrono>
#include <thread>

using namespace royale::bdd;
using namespace royale;
using namespace std;

//Scenario: Measure the depth data framerate for all usecases
//  Given I got a connected camera device
//  And I have registered a data listener
//  When I start the capture mode
//  And I sweep over all available usecases and calculate the FPS over an equivalent range of 5 seconds
//  Then all measured framerates must match the given framerates with plusminus 1 FPS

SCENARIO ("Measure the depth data framerate for all usecases", "[framerate]")
{
    LOG_OUTPUT ("Measure the depth data framerate for all usecases\r\n");
    ContextCameraDevice context;
    GIVEN ("I got a connected camera device")
    {
        CHECK_ROYALE_SUCCESS (context.connect());
        AND_THEN ("I have registered a data listener")
        {
            CHECK_ROYALE_SUCCESS (context.registerDataListener());
            WHEN ("I start the capture mode")
            {
                CHECK_ROYALE_SUCCESS (context.startCapture());
                AND_THEN ("I sweep over all available usecases and calculate the FPS over an equivalent range of 5 seconds")
                {
                    CHECK_ROYALE_SUCCESS (context.sweepAllUseCasesAndCalcFPS (ContextCameraDevice::MeasureMode::Equivalent, 5));
                    THEN ("all measured framerates must match the given framerates with plusminus 1 FPS")
                    {
                        CHECK_ROYALE_SUCCESS (context.checkCorrectFPSForAllUseCases (1));
                    }
                }
            }
        }
    }
}

//Scenario: Measure the raw data framerate for all usecases
//  Given I got a connected camera with activation code ROYALE_ACCESS_CODE_LEVEL2
//  And I have registered a raw data listener
//  When I start the capture mode
//  And I sweep over all available usecases and calculate the FPS over an equivalent range of 5 seconds
//  Then all measured framerates must match the given framerates with plusminus 1 FPS
SCENARIO ("Measure the raw data framerate for all usecases", "[framerate]")
{
    LOG_OUTPUT ("Measure the raw data framerate for all usecases\r\n");
    ContextCameraDevice context;
    GIVEN ("I got a connected camera with activation code for level 2")
    {
        CHECK_ROYALE_SUCCESS (context.connect (ROYALE_ACCESS_CODE_LEVEL2));
        AND_THEN ("I have registered a raw data listener")
        {
            CHECK_ROYALE_SUCCESS (context.registerDataListenerExtended());
            WHEN ("I start the capture mode")
            {
                CHECK_ROYALE_SUCCESS (context.startCapture());
                AND_THEN ("I sweep over all available usecases and calculate the FPS over an equivalent range of 5 seconds")
                {
                    CHECK_ROYALE_SUCCESS (context.sweepAllUseCasesAndCalcFPS (ContextCameraDevice::MeasureMode::Equivalent, 5));
                    THEN ("all measured framerates must match the given framerates with plusminus 1 FPS")
                    {
                        CHECK_ROYALE_SUCCESS (context.checkCorrectFPSForAllUseCases (1));
                    }
                }
            }
        }
    }
}

//Scenario: Change the framerate during operation to a valid value
// Given I got a connected camera device
// And I have registered a data listener
// When I start the capture mode
// And I changed the target framerate and sleep for some time
// Then I need to measure the target FPS by counting the depth callbacks
SCENARIO ("Change the framerate during operation to a valid value", "[framerate]")
{
    LOG_OUTPUT ("Change the framerate during operation to a valid value\r\n");
    const int targetFPS = 2;
    const int sleepTime = 5;
    ContextCameraDevice context;
    GIVEN ("I got a connected camera device")
    {
        CHECK_ROYALE_SUCCESS (context.connect());
        AND_THEN ("I have registered a data listener")
        {
            CHECK_ROYALE_SUCCESS (context.registerDataListener());
            WHEN ("I start the capture mode")
            {
                CHECK_ROYALE_SUCCESS (context.startCapture());
                AND_THEN ("I change the target framerate")
                {
                    CHECK_ROYALE_SUCCESS (context.getCameraDevice()->setFrameRate (targetFPS));
                    WHEN ("I measure the actual frame rate")
                    {
                        context.measureAverageFrameRate (sleepTime);
                        THEN ("The actual frame rate matches the target frame rate")
                        {
                            auto count = context.getAverageFps() * sleepTime;
                            auto minTarget = targetFPS * sleepTime - 1;
                            auto maxTarget = targetFPS * sleepTime + 1;
                            REQUIRE (count >= minTarget);
                            REQUIRE (count <= maxTarget);
                        }
                    }
                }
            }
        }
    }
}
