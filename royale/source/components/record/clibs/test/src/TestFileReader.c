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
#include <TestFileReader.h>

#include <RRFReader.h>

royale_rrf_handle g_rrfhandle;

void test_recording_openemptyfile()
{
    TEST_PRINT_FUNCTION_NAME;

    royale_rrf_api_error ret = royale_open_input_file (&g_rrfhandle, RECORDING_FILE_EMPTY);
    TEST_ASSERT_EQUAL_MESSAGE (RRF_FILE_EMPTY, ret, "Could not open the file for reading");
}

void test_recording_getframe (uint32_t framenumber)
{
    royale_rrf_api_error ret = royale_seek (g_rrfhandle, framenumber);
    TEST_ASSERT_EQUAL_MESSAGE (RRF_NO_ERROR, ret, "Could not seek to frame");

    struct royale_frame_v3 *frame;
    uint8_t i;

    ret = royale_get_frame (g_rrfhandle, &frame);
    TEST_ASSERT_EQUAL_MESSAGE (RRF_NO_ERROR, ret, "Could not retrieve frame");


    TEST_ASSERT_EQUAL_MESSAGE (25u, frame->frameHeader.targetFrameRate, "Wrong targetFrameRate");
    TEST_ASSERT_EQUAL_MESSAGE (2u, frame->frameHeader.numColumns, "Wrong numColumns");
    TEST_ASSERT_EQUAL_MESSAGE (2u, frame->frameHeader.numRows, "Wrong numRows");
    TEST_ASSERT_EQUAL_MESSAGE (9u, frame->frameHeader.numRawFrames, "Wrong numRawFrames");
    TEST_ASSERT_EQUAL_MESSAGE (3u, frame->frameHeader.numRawFrameSets, "Wrong numRawFrameSets");
    TEST_ASSERT_EQUAL_MESSAGE (framenumber * 100u, frame->frameHeader.timestamp, "Wrong timestamp");
    TEST_ASSERT_EQUAL_MESSAGE (30.0f, frame->frameHeader.illuTemperature, "Wrong illuTemperature");
    TEST_ASSERT_EQUAL_MESSAGE (2u, frame->frameHeader.numParameters, "Wrong numParameters");
    TEST_ASSERT_EQUAL_STRING_MESSAGE ("Test", frame->frameHeader.useCaseName, "Wrong use case name");
    TEST_ASSERT_EQUAL_MESSAGE (2u, frame->frameHeader.numAdditionalData, "Wrong numAdditionalData");

    TEST_ASSERT_EQUAL_MESSAGE (2u, frame->frameHeader.numStreams, "Wrong numStreams");
    for (i = 0u; i < frame->frameHeader.numStreams; ++i)
    {
        royale_streamheader_v3 *sth = &frame->streamHeaders[i];
        TEST_ASSERT_EQUAL_MESSAGE ( (uint16_t) (0xdefau + i), sth->streamId, "Wrong streamId");
        TEST_ASSERT_EQUAL_MESSAGE (1u, sth->numFrameGroups, "Wrong numFrameGroups");
        TEST_ASSERT_EQUAL_MESSAGE (0u, sth->frameGroupIdxs[0], "Wrong frameGroupIdxs");
    }

    TEST_ASSERT_EQUAL_MESSAGE (3u, frame->frameHeader.numExposureGroups, "Wrong numExposureGroups");
    for (i = 0u; i < frame->frameHeader.numExposureGroups; ++i)
    {
        royale_exposuregroupheader_v3 *expg = &frame->exposureGroupHeaders[i];
        TEST_ASSERT_EQUAL_STRING_MESSAGE ("Test", expg->exposureGroupName, "Wrong exposure group name");
        TEST_ASSERT_EQUAL_MESSAGE (1000u * i, expg->exposureMax, "Wrong exposureMax");
        TEST_ASSERT_EQUAL_MESSAGE (100u * i, expg->exposureMin, "Wrong exposureMin");
        TEST_ASSERT_EQUAL_MESSAGE (200u * i, expg->exposureTime, "Wrong exposureTime");
    }

    TEST_ASSERT_EQUAL_MESSAGE (1u, frame->frameHeader.numFrameGroups, "Wrong numFrameGroups");
    for (i = 0u; i < frame->frameHeader.numFrameGroups; ++i)
    {
        royale_framegroupheader_v3 *fgh = &frame->frameGroupHeaders[i];
        TEST_ASSERT_EQUAL_MESSAGE (3u, fgh->numRawFrameSets, "Wrong numRawFrameSets");
        TEST_ASSERT_EQUAL_MESSAGE (0u, fgh->rawFrameSetIdxs[0], "Wrong rawFrameSetIdxs");
        TEST_ASSERT_EQUAL_MESSAGE (1u, fgh->rawFrameSetIdxs[1], "Wrong rawFrameSetIdxs");
        TEST_ASSERT_EQUAL_MESSAGE (2u, fgh->rawFrameSetIdxs[2], "Wrong rawFrameSetIdxs");
    }

    for (i = 0u; i < 3u; ++i)
    {
        royale_rawframesetheader_v3 *rfs = &frame->rawFrameSetHeaders[i];
        TEST_ASSERT_EQUAL_MESSAGE (100u, rfs->capturedExpTime, "Wrong RFS capturedExpTime");
        TEST_ASSERT_EQUAL_MESSAGE (8000000u, rfs->modFreq, "Wrong RFS modFreq");
        TEST_ASSERT_EQUAL_MESSAGE (1u, rfs->phaseDefinition, "Wrong RFS phaseDefinition");
    }

    TEST_ASSERT_EQUAL_MESSAGE (4u, frame->rawFrameSetHeaders[0].numRawFrames, "Wrong numRawFrames 0");
    TEST_ASSERT_EQUAL_MESSAGE (4u, frame->rawFrameSetHeaders[1].numRawFrames, "Wrong numRawFrames 1");
    TEST_ASSERT_EQUAL_MESSAGE (1u, frame->rawFrameSetHeaders[2].numRawFrames, "Wrong numRawFrames 2");


    for (i = 0u; i < 2u; ++i)
    {
        royale_processingparameter_v3 *params = &frame->processingParameters[i];
        TEST_ASSERT_EQUAL_MESSAGE (i, params->dataType, "Wrong Params dataType");
        TEST_ASSERT_EQUAL_MESSAGE (i, params->processingFlag, "Wrong Params processingFlag");
        TEST_ASSERT_EQUAL_MESSAGE (i, params->value, "Wrong Params value");
    }

    for (i = 0u; i < 2u; ++i)
    {
        royale_additionaldata_v3 *add_data = &frame->additionalData[i];
        TEST_ASSERT_EQUAL_STRING_MESSAGE ("Test", add_data->dataName, "Wrong additional data name");
        TEST_ASSERT_EQUAL_MESSAGE ( (i + 1u) * 100u, add_data->dataSize, "Wrong additional data size");
        uint32_t j;
        for (j = 0u; j < (i + 1u) * 100u; ++j)
        {
            TEST_ASSERT_EQUAL_MESSAGE (i, add_data->data[j], "Wrong additional data");
        }
    }

    uint32_t imagedatasize = (uint32_t) (frame->frameHeader.numColumns * (frame->frameHeader.numRows - 1u));
    uint32_t pseudodatasize = frame->frameHeader.numColumns;
    for (i = 0u; i < 9u; ++i)
    {
        uint16_t *imagedata = frame->imageData[i];
        uint16_t *pseudodata = frame->pseudoData[i];

        uint32_t j;
        for (j = 0u; j < imagedatasize; ++j)
        {
            TEST_ASSERT_EQUAL_MESSAGE (i, imagedata[j], "Wrong image data");
        }

        for (j = 0u; j < pseudodatasize; ++j)
        {
            TEST_ASSERT_EQUAL_MESSAGE (i, pseudodata[j], "Wrong pseudo data");
        }
    }

    ret = royale_free_frame (&frame);
    TEST_ASSERT_EQUAL_MESSAGE (RRF_NO_ERROR, ret, "Could not free frame");
}

void test_recording_openreadfile()
{
    TEST_PRINT_FUNCTION_NAME;

    royale_rrf_api_error ret = royale_open_input_file (&g_rrfhandle, RECORDING_FILE_FRAMES);
    TEST_ASSERT_EQUAL_MESSAGE (RRF_NO_ERROR, ret, "Could not open the file for reading");

    const struct royale_fileinformation_v3 *fileinfo = royale_get_fileinformation (g_rrfhandle);

    TEST_ASSERT_EQUAL_MESSAGE (8, fileinfo->calibrationDataSize, "Wrong calibration data size");

    uint32_t i;
    for (i = 0; i < fileinfo->calibrationDataSize; ++i)
    {
        TEST_ASSERT_EQUAL_MESSAGE (i, fileinfo->calibrationData[i], "Wrong calib data");
    }

    TEST_ASSERT_EQUAL_STRING_MESSAGE ("1234", fileinfo->imagerSerial, "Wrong imager serial");
    TEST_ASSERT_EQUAL_STRING_MESSAGE ("TestCam", fileinfo->cameraName, "Wrong camera name");
    TEST_ASSERT_EQUAL_STRING_MESSAGE ("TestImager", fileinfo->imagerType, "Wrong imager type");
    TEST_ASSERT_EQUAL_STRING_MESSAGE ("M2452", fileinfo->pseudoDataInter, "Wrong pseudo data interpreter");

    TEST_ASSERT_EQUAL_MESSAGE (1u, fileinfo->royaleMajor, "Wrong major version");
    TEST_ASSERT_EQUAL_MESSAGE (2u, fileinfo->royaleMinor, "Wrong minor version");
    TEST_ASSERT_EQUAL_MESSAGE (3u, fileinfo->royalePatch, "Wrong patch version");
    TEST_ASSERT_EQUAL_MESSAGE (4u, fileinfo->royaleBuild, "Wrong build version");

    TEST_ASSERT_EQUAL_MESSAGE (RRF_ROYALE_WINDOWS, fileinfo->platform, "Wrong platform");

    TEST_ASSERT_EQUAL_MESSAGE (3u, fileinfo->numComponents, "Wrong number of component versions");

    TEST_ASSERT_EQUAL_STRING_MESSAGE ("Core", fileinfo->componentNames[0], "Wrong component name 0");
    TEST_ASSERT_EQUAL_STRING_MESSAGE ("TestComponent1", fileinfo->componentNames[1], "Wrong component name 1");
    TEST_ASSERT_EQUAL_STRING_MESSAGE ("TestComponent2", fileinfo->componentNames[2], "Wrong component name 2");

    TEST_ASSERT_EQUAL_STRING_MESSAGE ("Type1", fileinfo->componentTypes[0], "Wrong component type 0");
    TEST_ASSERT_EQUAL_STRING_MESSAGE ("Type2", fileinfo->componentTypes[1], "Wrong component type 1");
    TEST_ASSERT_EQUAL_STRING_MESSAGE ("Type3", fileinfo->componentTypes[2], "Wrong component type 2");

    TEST_ASSERT_EQUAL_STRING_MESSAGE ("1.2.3", fileinfo->componentVersions[0], "Wrong component version 0");
    TEST_ASSERT_EQUAL_STRING_MESSAGE ("2.3.4", fileinfo->componentVersions[1], "Wrong component version 1");
    TEST_ASSERT_EQUAL_STRING_MESSAGE ("3.4.5", fileinfo->componentVersions[2], "Wrong component version 2");

    TEST_ASSERT_EQUAL_MESSAGE (6u, royale_get_num_frames (g_rrfhandle), "Wrong number of frames");
}

void test_recording_closereadfile()
{
    TEST_PRINT_FUNCTION_NAME;

    royale_rrf_api_error ret = royale_close_input_file (g_rrfhandle);
    TEST_ASSERT_EQUAL_MESSAGE (RRF_NO_ERROR, ret, "Could not close the file after reading");
}

void test_recording_readfile()
{
    TEST_PRINT_FUNCTION_NAME;

    test_recording_openemptyfile();
    test_recording_openreadfile();
    test_recording_getframe (0u);
    test_recording_getframe (1u);
    test_recording_getframe (2u);
    test_recording_getframe (3u);
    test_recording_getframe (4u);
    test_recording_getframe (5u);
    test_recording_closereadfile();
}
