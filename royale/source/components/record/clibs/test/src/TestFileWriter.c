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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <TestRecordingHelper.h>
#include <TestFileWriter.h>

#include <RRFWriter.h>

royale_rrf_handle g_rrfhandle;

void test_recording_openwritefile (const char *filename)
{
    TEST_PRINT_FUNCTION_NAME;

    struct royale_fileinformation_v3 fileinfo;

    uint8_t caldata[8];

    fileinfo.calibrationDataSize = sizeof (caldata);
    uint32_t i;
    for (i = 0u; i < fileinfo.calibrationDataSize; ++i)
    {
        caldata[i] = (uint8_t) i;
    }
    fileinfo.calibrationData = caldata;

    fileinfo.imagerSerial = "1234";
    fileinfo.cameraName = "TestCam";
    fileinfo.imagerType = "TestImager";
    fileinfo.pseudoDataInter = "M2452";
    fileinfo.royaleMajor = 1u;
    fileinfo.royaleMinor = 2u;
    fileinfo.royalePatch = 3u;
    fileinfo.royaleBuild = 4u;
    fileinfo.platform = RRF_ROYALE_WINDOWS;

    char *names[] = { "Core",
                      "TestComponent1",
                      "TestComponent2"
                    };
    fileinfo.componentNames = names;
    fileinfo.numComponents = sizeof (names) / sizeof (char *);

    char *types[] = { "Type1",
                      "Type2",
                      "Type3"
                    };
    fileinfo.componentTypes = types;

    char *versions[] = { "1.2.3",
                         "2.3.4",
                         "3.4.5"
                       };
    fileinfo.componentVersions = versions;

    royale_rrf_api_error ret = royale_open_output_file (&g_rrfhandle, filename, &fileinfo);
    TEST_ASSERT_EQUAL_MESSAGE (RRF_NO_ERROR, ret, "Could not open the file for writing");
}

void test_recording_putframe (uint32_t framenumber)
{
    struct royale_frame_v3 frame;
    uint8_t i;

    frame.frameHeader.frameSize = 0u; //< Leave this empty, will be filled by underlying layer
    frame.frameHeader.targetFrameRate = 25u;
    frame.frameHeader.numColumns = 2u;
    frame.frameHeader.numRows = 2u;
    frame.frameHeader.numRawFrames = 9u;
    frame.frameHeader.numRawFrameSets = 3u;
    frame.frameHeader.timestamp = framenumber * 100u;
    frame.frameHeader.illuTemperature = 30.0f;
    frame.frameHeader.numParameters = 2u;
    memset (&frame.frameHeader.useCaseName, 0, ROYALE_FILEHEADER_V3_USE_CASE_LENGTH);
    memcpy (&frame.frameHeader.useCaseName, "Test", 4);
    frame.frameHeader.numAdditionalData = 2u;

    frame.frameHeader.numStreams = 2;
    royale_streamheader_v3 sth[2];
    for (i = 0u; i < 2u; ++i)
    {
        sth[i].streamId = (uint16_t) (0xdefau + i);
        sth[i].numFrameGroups = 1u;
        sth[i].frameGroupIdxs[0] = 0u;
    }
    frame.streamHeaders = sth;

    frame.frameHeader.numExposureGroups = 3u;
    royale_exposuregroupheader_v3 expg[3];
    for (i = 0u; i < 3u; ++i)
    {
        memset (expg[i].exposureGroupName, 0, ROYALE_FILEHEADER_V3_EXPOSUREGROUP_NAME_LENGTH);
        memcpy (expg[i].exposureGroupName, "Test", 4);
        expg[i].exposureMax = (uint32_t) (1000u * i);
        expg[i].exposureMin = (uint32_t) (100u * i);
        expg[i].exposureTime = (uint32_t) (200u * i);
    }
    frame.exposureGroupHeaders = expg;

    frame.frameHeader.numFrameGroups = 1;
    royale_framegroupheader_v3 fgh[1];
    fgh[0].numRawFrameSets = 3;
    fgh[0].rawFrameSetIdxs[0] = 0;
    fgh[0].rawFrameSetIdxs[1] = 1;
    fgh[0].rawFrameSetIdxs[2] = 2;
    frame.frameGroupHeaders = fgh;

    royale_rawframesetheader_v3 rfs[3];
    for (i = 0u; i < 3u; ++i)
    {
        rfs[i].capturedExpTime = 100u;
        rfs[i].modFreq = 8000000u;
        rfs[i].phaseDefinition = 1u;
    }
    rfs[0].numRawFrames = 4u;
    rfs[1].numRawFrames = 4u;
    rfs[2].numRawFrames = 1u;
    frame.rawFrameSetHeaders = rfs;

    royale_processingparameter_v3 params[2];
    for (i = 0u; i < 2u; ++i)
    {
        params[i].dataType = i;
        params[i].processingFlag = i;
        params[i].value = i;
    }
    frame.processingParameters = params;

    royale_additionaldata_v3 add_data[2];
    for (i = 0u; i < 2u; ++i)
    {
        memset (&add_data[i].dataName, 0, ROYALE_FILEHEADER_V3_ADDITIONAL_DATA_NAME_LENGTH);
        memcpy (&add_data[i].dataName, "Test", 4);
        add_data[i].dataSize = (i + 1u) * 100u;
        add_data[i].data = (uint8_t *) malloc ( (size_t) add_data[i].dataSize);
        memset (add_data[i].data, i, (size_t) add_data[i].dataSize);
    }
    frame.additionalData = add_data;

    uint16_t *imagedata[9];
    uint16_t *pseudodata[9];
    uint32_t imagedatasize = (uint32_t) (frame.frameHeader.numColumns * frame.frameHeader.numRows);
    uint32_t pseudodatasize = frame.frameHeader.numColumns;
    for (i = 0u; i < 9u; ++i)
    {
        imagedata[i] = (uint16_t *) malloc (imagedatasize * sizeof (uint16_t));
        pseudodata[i] = (uint16_t *) malloc (pseudodatasize * sizeof (uint16_t));

        uint32_t j;
        for (j = 0u; j < imagedatasize; ++j)
        {
            imagedata[i][j] = i;
        }
        for (j = 0u; j < pseudodatasize; ++j)
        {
            pseudodata[i][j] = i;
        }
    }

    frame.imageData = imagedata;
    frame.pseudoData = pseudodata;

    // Put the frame to file
    royale_rrf_api_error ret = royale_output_data (g_rrfhandle, &frame);
    TEST_ASSERT_EQUAL_MESSAGE (RRF_NO_ERROR, ret, "Could not close the file after writing");

    // Clean up structures
    for (i = 0u; i < 2u; ++i)
    {
        free (add_data[i].data);
    }

    for (i = 0u; i < 9u; ++i)
    {
        free (imagedata[i]);
        free (pseudodata[i]);
    }
}

void test_recording_closewritefile()
{
    TEST_PRINT_FUNCTION_NAME;

    royale_rrf_api_error ret = royale_close_output_file (g_rrfhandle);
    TEST_ASSERT_EQUAL_MESSAGE (RRF_NO_ERROR, ret, "Could not close the file after writing");
}

void test_recording_writefile()
{
    TEST_PRINT_FUNCTION_NAME;

    test_recording_openwritefile (RECORDING_FILE_EMPTY);
    test_recording_closewritefile();

    test_recording_openwritefile (RECORDING_FILE_FRAMES);
    test_recording_putframe (0u);
    test_recording_putframe (1u);
    test_recording_putframe (2u);
    test_recording_putframe (3u);
    test_recording_putframe (4u);
    test_recording_putframe (5u);
    test_recording_closewritefile();
}
