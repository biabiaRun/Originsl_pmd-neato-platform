#include "catch.hpp"
#include "Helper.hpp"
#include "ContextCameraDevice.hpp"

using namespace royale::bdd;
using namespace royale;
using namespace std;

//Scenario: Measure time for switching usecases
//   Given I got a connected camera device
//   And I have registered a data listener
//   When I start the capture mode
//   And I sweep over all available usecases
//   Then the maximum switching time between the usecases was less than 900 ms
SCENARIO ("Measure time for switching usecases", "[usecase]")
{
    LOG_OUTPUT ("Measure time for switching usecases\r\n");
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
                AND_THEN ("I sweep over all available usecases")
                {
                    CHECK_ROYALE_SUCCESS (context.sweepAllUseCasesAndCalcFPS());
                    THEN ("the maximum switching time between the usecases was less than 900 ms")
                    {
                        context.diffBetweenUseCaseSwitch (900);
                    }
                }
            }
        }
    }
}