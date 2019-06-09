#include "catch.hpp"
#include "Helper.hpp"
#include "ContextCameraDevice.hpp"

#include <chrono>
#include <thread>

using namespace royale::bdd;
using namespace royale;
using namespace std;

//Scenario: Test the depth callback
//  Given I started capturing with the fastest usecase
//  When I sleep for 4 seconds
//  Then I want the DepthData struct being filled correctly
SCENARIO ("Test the depth callback", "[callback]")
{
    LOG_OUTPUT ("Test the depth callback\r\n");
    ContextCameraDevice context;
    GIVEN ("Given I started capturing with the fastest usecase")
    {
        CHECK_ROYALE_SUCCESS (context.connect());
        CHECK_ROYALE_SUCCESS (context.setUseCaseFastest());
        CHECK_ROYALE_SUCCESS (context.registerDataListener());
        CHECK_ROYALE_SUCCESS (context.startCapture());
        WHEN ("I sleep for 4 seconds")
        {
            std::this_thread::sleep_for (std::chrono::seconds (4));
            THEN ("I want the DepthData struct being filled correctly")
            {
                CHECK_ROYALE_SUCCESS (context.checkDepthData());
            }
        }
    }
}