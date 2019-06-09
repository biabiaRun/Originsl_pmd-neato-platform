/****************************************************************************\
 * Copyright (C) 2016 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <RRFWriter.h>
#include <record/FileWriter.hpp>

#include <vector>
#include <algorithm>
#include <map>

using namespace royale;
using namespace royale::record;

static uint32_t g_numWriterInstances = 0;
static std::map<royale_rrf_handle, std::unique_ptr<FileWriter>> g_instances;

RRFACCESSAPI royale_rrf_api_error royale_open_output_file (royale_rrf_handle *handle, const char *filename,
        const struct royale_fileinformation_v3 *fileinfo)
{
    std::unique_ptr<FileWriter> curInstance (new FileWriter());

    try
    {
        std::vector<std::string> tempComponentNames;
        std::vector<std::string> tempComponentTypes;
        std::vector<std::string> tempComponentVersions;

        for (auto i = 0u; i < fileinfo->numComponents; ++i)
        {
            tempComponentNames.push_back (fileinfo->componentNames[i]);
            tempComponentTypes.push_back (fileinfo->componentTypes[i]);
            tempComponentVersions.push_back (fileinfo->componentVersions[i]);
        }

        curInstance->open (filename, std::vector<uint8_t> (fileinfo->calibrationData, fileinfo->calibrationData + fileinfo->calibrationDataSize),
                           fileinfo->imagerSerial, fileinfo->cameraName, fileinfo->imagerType, fileinfo->pseudoDataInter,
                           fileinfo->royaleMajor, fileinfo->royaleMinor, fileinfo->royalePatch, fileinfo->royaleBuild,
                           fileinfo->platform, tempComponentNames, tempComponentTypes, tempComponentVersions);
    }
    catch (const std::invalid_argument &)
    {
        return royale_rrf_api_error::RRF_COULD_NOT_OPEN;
    }
    catch (const std::runtime_error &)
    {
        return royale_rrf_api_error::RRF_RUNTIME_ERROR;
    }

    *handle = g_numWriterInstances;
    g_instances[*handle] = std::move (curInstance);
    g_numWriterInstances++;

    return royale_rrf_api_error::RRF_NO_ERROR;
}

RRFACCESSAPI royale_rrf_api_error royale_close_output_file (const royale_rrf_handle handle)
{
    CHECK_RRFHANDLE (FileWriter, handle, royale_rrf_api_error::RRF_HANDLE_INVALID);

    royale_rrf_api_error ret = royale_rrf_api_error::RRF_NO_ERROR;

    try
    {
        rw->close();
    }
    catch (...)
    {
        ret = royale_rrf_api_error::RRF_RUNTIME_ERROR;
    }

    instanceit->second.reset();
    g_instances.erase (instanceit);

    return ret;
}

RRFACCESSAPI royale_rrf_api_error royale_output_data (const royale_rrf_handle handle, struct royale_frame_v3 *frame)
{
    CHECK_RRFHANDLE (FileWriter, handle, royale_rrf_api_error::RRF_HANDLE_INVALID);

    try
    {
        rw->put (std::vector<const uint16_t *> (frame->imageData, frame->imageData + frame->frameHeader.numRawFrames),
                 std::vector<const uint16_t *> (frame->pseudoData, frame->pseudoData + frame->frameHeader.numRawFrames),
                 &frame->frameHeader,
                 std::vector<royale_streamheader_v3> (frame->streamHeaders, frame->streamHeaders + frame->frameHeader.numStreams),
                 std::vector<royale_framegroupheader_v3> (frame->frameGroupHeaders, frame->frameGroupHeaders + frame->frameHeader.numFrameGroups),
                 std::vector<royale_exposuregroupheader_v3> (frame->exposureGroupHeaders, frame->exposureGroupHeaders + frame->frameHeader.numExposureGroups),
                 std::vector<royale_rawframesetheader_v3> (frame->rawFrameSetHeaders, frame->rawFrameSetHeaders + frame->frameHeader.numRawFrameSets),
                 std::vector<royale_processingparameter_v3> (frame->processingParameters, frame->processingParameters + frame->frameHeader.numParameters),
                 std::vector<royale_additionaldata_v3> (frame->additionalData, frame->additionalData + frame->frameHeader.numAdditionalData));
    }
    catch (...)
    {
        return royale_rrf_api_error::RRF_RUNTIME_ERROR;
    }

    return royale_rrf_api_error::RRF_NO_ERROR;
}

