/****************************************************************************\
* Copyright (C) 2016 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <unity.h>

#include <TestFileReader.h>
#include <TestFileWriter.h>

void setUp (void)
{
}

void tearDown (void)
{
}

int main (void)
{
    UNITY_BEGIN();

    printf ("===========================================================\n");
    printf ("testing file writing\n");
    printf ("===========================================================\n");
    RUN_TEST (test_recording_writefile);

    printf ("===========================================================\n");
    printf ("testing file reading\n");
    printf ("===========================================================\n");
    RUN_TEST (test_recording_readfile);

    return UNITY_END();
}
