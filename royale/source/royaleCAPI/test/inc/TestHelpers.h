/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <stdbool.h>
#include <royaleCAPI.h>

#ifdef _WINDOWS
#include <Windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

#ifdef _WINDOWS
CRITICAL_SECTION                m_callback_lock;
#define TEST_MUTEX_INIT         InitializeCriticalSection (&m_callback_lock)
#define TEST_MUTEX_LOCK         EnterCriticalSection (&m_callback_lock)
#define TEST_MUTEX_UNLOCK       LeaveCriticalSection (&m_callback_lock)
#define TEST_MUTEX_DELETE       DeleteCriticalSection (&m_callback_lock)
#define TEST_SLEEP_MS(x)        Sleep(x)
#define FILE_UNLINK(x)          _unlink(x)
#else
pthread_mutex_t                 m_callback_lock;
#define TEST_MUTEX_INIT         pthread_mutex_init (&m_callback_lock, NULL)
#define TEST_MUTEX_LOCK         pthread_mutex_lock (&m_callback_lock)
#define TEST_MUTEX_UNLOCK       pthread_mutex_unlock (&m_callback_lock)
#define TEST_MUTEX_DELETE       pthread_mutex_destroy (&m_callback_lock)
#define TEST_SLEEP_MS(x)        usleep((x)*1000)
#define FILE_UNLINK(x)          unlink(x)
#endif

#define TEST_PRINT_FUNCTION_NAME    printf ("%s\n", __FUNCTION__)

#define WAIT_MS_AFTER_UNREGISTER    500

/**
 * If no error has occurred, this will be NULL. If an error occurs then it's a pointer to a string
 * with sufficient lifetime to be accessed in the main test thread (probably a string literal).
 */
const char *helper_callback_error_message;

/**
 * During a callback, check that condition is true. This will `return;` if the condition isn't true.
 *
 * The message must be a string literal, or have sufficient lifetime to be accessiable in the main
 * test thread.
 */
#define TEST_CALLBACK_CONDITION_RETVOID(condition, message) \
    do { \
        if (!(condition)) { \
            helper_callback_error_message = message; \
            TEST_MUTEX_UNLOCK; \
            return; \
                } \
    } while (0)

/**
 * During a callback, check that condition is true. This will `return NULL;` if the condition isn't true.
 *
 * The message must be a string literal, or have sufficient lifetime to be accessiable in the main
 * test thread.
 */
#define TEST_CALLBACK_CONDITION_RETNULL(condition, message) \
    do { \
        if (!(condition)) { \
            helper_callback_error_message = message; \
            TEST_MUTEX_UNLOCK; \
            return NULL; \
                } \
    } while (0)

/**
 * Resets the TEST_CALLBACK_CONDITION / TEST_ASSERT_NO_ASYNC_ERROR to the state that no async error
 * has happened.
 *
 * Callable from any thread with the TEST_MUTEX_LOCK mutex held.
 */
#define TEST_CLEAR_ASYNC_ERROR_FLAG \
        helper_callback_error_message = NULL

/**
 * Called from the main test thread, causes a TEST_ASSERT if one of the TEST_CALLBACK_CONDITION
 * asserts failed.
 */
#define TEST_ASSERT_NO_ASYNC_ERROR TEST_ASSERT_NULL_MESSAGE(helper_callback_error_message, helper_callback_error_message)

/**
 * Wait for count callbacks, with the given timeout (in ms).  Returns true (and leaves the flag set)
 * if enough callbacks arrive before the timeout, otherwise returns false (and has set the flag
 * false).
 *
 * If the flag_to_check already true when called, that will count as one callback.
 *
 * Note that there's a built-in polling frequency, multiple callbacks within the polling time will
 * only register as a single one for the counter.
 */
bool wait_for_callback_called_count_timeout (bool *flag_to_check, const int count, const int timeout);

/**
 * Calls the method above with a preconfigured timeout.
 */
bool wait_for_callback_called_count (bool *flag_to_check, const int count);

/**
 * Wait for 1 callback, with a preconfigured timeout.
 */
bool wait_for_callback_called (bool *flag_to_check);
