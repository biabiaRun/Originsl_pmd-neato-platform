/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <unity.h>
#include <TestHelpers.h>

#define WAIT_MS_BETWEEN_CALLBACK_CHECK  200
#define DEFAULT_TIMEOUT_MS (20 * WAIT_MS_BETWEEN_CALLBACK_CHECK)

bool wait_for_callback_called (bool *flag_to_check)
{
    return wait_for_callback_called_count_timeout (flag_to_check,
            1, DEFAULT_TIMEOUT_MS);
}

bool wait_for_callback_called_count (bool *flag_to_check, const int count)
{
    return wait_for_callback_called_count_timeout (flag_to_check,
            count, DEFAULT_TIMEOUT_MS);
}

bool wait_for_callback_called_count_timeout (bool *flag_to_check, const int count, const int timeout)
{
    bool retry = true;
    bool success = false;
    int count_received = 0;
    int current_try = 0;
    int max_retries = timeout / WAIT_MS_BETWEEN_CALLBACK_CHECK;
    while (retry)
    {
        TEST_SLEEP_MS (WAIT_MS_BETWEEN_CALLBACK_CHECK);
        TEST_MUTEX_LOCK;

        if (*flag_to_check)
        {
            count_received++;
            if (count_received >= count)
            {
                success = true;
            }
            else
            {
                *flag_to_check = false;
            }
        }

        TEST_MUTEX_UNLOCK;

        if (success || ++current_try >= max_retries)
        {
            retry = false;
        }
    }
    return success;
}
