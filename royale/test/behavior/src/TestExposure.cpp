#include "catch.hpp"
#include "Helper.hpp"
#include "ContextCameraDevice.hpp"

using namespace royale::bdd;
using namespace royale;
using namespace std;

//Scenario: Change the exposure time after starting capture mode
//  Given I started capturing with the fastest usecase
//  When I change the exposure time from max to min by a resolution of 10
//  Then I end up with an exposure time of min
//  And the maximum switching time between the exposure times was less than 50 ms
SCENARIO ("Change the exposure time after starting capture mode", "[exposure]")
{
    LOG_OUTPUT ("Change the exposure time after starting capture mode\r\n");
    ContextCameraDevice context;
    GIVEN ("I started capturing with the fastest usecase")
    {
        CHECK_ROYALE_SUCCESS (context.connect());
        CHECK_ROYALE_SUCCESS (context.setUseCaseFastest());
        CHECK_ROYALE_SUCCESS (context.registerDataListener());
        CHECK_ROYALE_SUCCESS (context.startCapture());
        WHEN ("I change the exposure time from max to min by a resolution of 10")
        {
            unsigned int stepSize = 10;
            unsigned int maxValue, minValue;
            context.getExposureLimits (maxValue, minValue);

            for (uint32_t i = maxValue; i > minValue; i -= stepSize)
            {
                INFO ("Setting exposure to " << i);
                CHECK_ROYALE_SUCCESS (context.setExposureTime (i));
            }

            // set to minimal exposure time finally
            CHECK_ROYALE_SUCCESS (context.setExposureTime (minValue));

            THEN ("I end up with an exposure time of min")
            {
                REQUIRE (context.getLastExposureTime() == minValue);

                AND_THEN ("the maximum switching time between the exposure times was less than 50 ms")
                {
                    context.diffBetweenExposureTimeSwitch (50000);
                }
            }
        }
    }
}

//Scenario: Measure time for changing exposure mode
//  Given I got a connected camera device
//  And I have registered a data listener
//  When I start the capture mode
//  And I switch the exposure mode to automatic
//  And I switch the exposure mode to manual
//  Then the maximum switching time between the exposure modes was less than 1 ms
SCENARIO ("Measure time for changing exposure mode", "[exposure]")
{
    LOG_OUTPUT ("Measure time for changing exposure mode\r\n");
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
                AND_THEN ("I switch the exposure mode to automatic")
                {
                    CHECK_ROYALE_SUCCESS (context.setExposureMode (ExposureMode::AUTOMATIC));
                    AND_THEN ("I switch the exposure mode to manual")
                    {
                        CHECK_ROYALE_SUCCESS (context.setExposureMode (ExposureMode::MANUAL));
                        THEN ("the maximum switching time between the exposure modes was less than 1 ms")
                        {
                            context.diffBetweenExposureModeSwitch (1000);
                        }
                    }
                }
            }
        }
    }
}