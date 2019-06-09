#include "catch.hpp"
#include "Helper.hpp"
#include "ContextCameraDevice.hpp"

#include <chrono>
#include <thread>

using namespace royale::bdd;
using namespace royale;
using namespace std;

//Scenario: Default behavior - start capture
//  Given I got a connected camera device
//  And I have registered a data listener
//  And I have set the default usecase
//  When I start the capture mode
//  Then this start capture operation took less than 600 ms
//  And the isCapture call must be true
SCENARIO ("Default behavior - start capture", "[capture]")
{
    ContextCameraDevice context;
    GIVEN ("I got a connected camera device")
    {
        CHECK_ROYALE_SUCCESS (context.connect());
        AND_THEN ("I have registered a data listener")
        {
            CHECK_ROYALE_SUCCESS (context.registerDataListener());
            AND_THEN ("I have set the default usecase")
            {
                CHECK_ROYALE_SUCCESS (context.setUseCaseDefault());
                WHEN ("I start the capture mode")
                {
                    CHECK_ROYALE_SUCCESS (context.startCapture());
                    THEN ("this start capture operation took less than 600 ms")
                    {
                        REQUIRE (context.getStartCaptureDuration() < 600);
                        AND_THEN ("the isCapture call must be true")
                        {
                            REQUIRE (context.isCapturing());
                        }
                    }
                }
            }
        }
    }
}

//Scenario: Default behavior - stop capture
//  Given I started capturing with the fastest usecase
//  Then this start capture operation took less than 600 ms
//  When I stop the capture mode
//  Then this stop capture operation took less than 600 ms
SCENARIO ("Default behavior - stop capture", "[capture]")
{
    ContextCameraDevice context;
    GIVEN ("I started capturing with the fastest usecase")
    {
        CHECK_ROYALE_SUCCESS (context.connect());
        CHECK_ROYALE_SUCCESS (context.setUseCaseFastest());
        CHECK_ROYALE_SUCCESS (context.registerDataListener());
        CHECK_ROYALE_SUCCESS (context.startCapture());
        THEN ("this start capture operation took less than 600 ms")
        {
            REQUIRE (context.getStartCaptureDuration() < 600);
            WHEN ("I stop the capture mode")
            {
                CHECK_ROYALE_SUCCESS (context.stopCapture());
                THEN ("this stop capture operation took less than 600 ms")
                {
                    REQUIRE (context.getStopCaptureDuration() < 600);
                }
            }
        }
    }
}

//Scenario: Measure the startup time(delay before receiving the first frame)
//  Given I got a connected camera device
//  And I have registered a data listener
//  And I have set the default usecase
//  Then it takes less than 900 milliseconds till the frames arrive after starting the capture mode
SCENARIO ("Measure the startup time(delay before receiving the first frame)", "[capture]")
{
    ContextCameraDevice context;
    GIVEN ("I got a connected camera device")
    {
        CHECK_ROYALE_SUCCESS (context.connect());
        AND_THEN ("I have registered a data listener")
        {
            CHECK_ROYALE_SUCCESS (context.registerDataListener());
            AND_THEN ("I have set the default usecase")
            {
                CHECK_ROYALE_SUCCESS (context.setUseCaseDefault());
                THEN ("it takes less than 900 milliseconds till the frames arrive after starting the capture mode")
                {
                    CHECK_ROYALE_SUCCESS (context.startCapture());
                    std::this_thread::sleep_for (std::chrono::seconds (2));
                    REQUIRE (context.startUpPeriod() < 900);
                }
            }
        }
    }
}

//Scenario: Test the depth callback
//  Given I started capturing with the fastest usecase
//  When I sleep for 4 seconds
//  Then I want the DepthData struct being filled correctly
SCENARIO ("Test the depth callback")
{

}