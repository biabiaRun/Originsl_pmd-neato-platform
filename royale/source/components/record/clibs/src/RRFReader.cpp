/****************************************************************************\
 * Copyright (C) 2016 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <RRFReader.h>
#include <record/FileReaderDispatcher.hpp>

#include <vector>
#include <algorithm>

using namespace royale;
using namespace royale::record;

static uint32_t g_numReaderInstances = 0;
static std::map<royale_rrf_handle, std::unique_ptr<FileReaderDispatcher>> g_instances;
static std::map<royale_rrf_handle, std::unique_ptr<struct royale_fileinformation_v3>> g_fileinfos;

namespace
{
    royale_rrf_api_error fill_fileinfo (royale_rrf_handle handle)
    {
        CHECK_RRFHANDLE (FileReaderDispatcher, handle, royale_rrf_api_error::RRF_HANDLE_INVALID);

        std::unique_ptr<struct royale_fileinformation_v3> curInstance (new struct royale_fileinformation_v3());
        royale_fileinformation_v3 *info = static_cast<royale_fileinformation_v3 *> (curInstance.get());
        memset (info, 0, sizeof (struct royale_fileinformation_v3));

        try
        {
            info->calibrationDataSize = static_cast<uint32_t> (rw->getCalibrationData().size());
            info->calibrationData = new uint8_t[info->calibrationDataSize];
            memcpy (info->calibrationData, rw->getCalibrationData().data(), info->calibrationDataSize);

            std::string imagerSerial = rw->imagerSerial();
            info->imagerSerial = new char[imagerSerial.size() + 1];
            memset (info->imagerSerial, 0, imagerSerial.size() + 1);
            memcpy (info->imagerSerial, imagerSerial.c_str(), imagerSerial.size());

            std::string cameraName = rw->cameraName();
            info->cameraName = new char[cameraName.size() + 1];
            memset (info->cameraName, 0, cameraName.size() + 1);
            memcpy (info->cameraName, cameraName.c_str(), cameraName.size());

            std::string imagerType = rw->imagerType();
            info->imagerType = new char[imagerType.size() + 1];
            memset (info->imagerType, 0, imagerType.size() + 1);
            memcpy (info->imagerType, imagerType.c_str(), imagerType.size());

            std::string pseudoDataInter = rw->pseudoDataInterpreterType();
            info->pseudoDataInter = new char[pseudoDataInter.size() + 1];
            memset (info->pseudoDataInter, 0, pseudoDataInter.size() + 1);
            memcpy (info->pseudoDataInter, pseudoDataInter.c_str(), pseudoDataInter.size());

            info->royaleMajor = rw->royaleMajor();
            info->royaleMinor = rw->royaleMinor();
            info->royalePatch = rw->royalePatch();
            info->royaleBuild = rw->royaleBuild();

            info->platform = rw->platform();

            auto cversions = rw->getComponentVersions();

            info->numComponents = static_cast<uint16_t> (cversions.size());

            if (!cversions.empty())
            {
                info->componentNames = new char *[info->numComponents];
                info->componentTypes = new char *[info->numComponents];
                info->componentVersions = new char *[info->numComponents];

                for (uint16_t i = 0u; i < info->numComponents; ++i)
                {
                    std::string cname = cversions[i].componentName;
                    std::string ctype = cversions[i].componentType;
                    std::string cversion = cversions[i].componentVersion;

                    info->componentNames[i] = new char[cname.size() + 1];
                    memset (info->componentNames[i], 0, cname.size() + 1);
                    memcpy (info->componentNames[i], cname.c_str(), cname.size());

                    info->componentTypes[i] = new char[ctype.size() + 1];
                    memset (info->componentTypes[i], 0, ctype.size() + 1);
                    memcpy (info->componentTypes[i], ctype.c_str(), ctype.size());

                    info->componentVersions[i] = new char[cversion.size() + 1];
                    memset (info->componentVersions[i], 0, cversion.size() + 1);
                    memcpy (info->componentVersions[i], cversion.c_str(), cversion.size());
                }
            }
        }
        catch (...)
        {
            return royale_rrf_api_error::RRF_RUNTIME_ERROR;
        }

        g_fileinfos[handle] = std::move (curInstance);

        return royale_rrf_api_error::RRF_NO_ERROR;
    }

    void free_fileinfo (royale_rrf_handle handle)
    {
        auto infoit = g_fileinfos.find (handle);
        if (infoit == g_fileinfos.end())
        {
            return;
        }
        royale_fileinformation_v3 *info = static_cast<royale_fileinformation_v3 *> (infoit->second.get());

        delete[] info->calibrationData;
        delete[] info->imagerSerial;
        delete[] info->cameraName;
        delete[] info->imagerType;
        delete[] info->pseudoDataInter;

        for (uint16_t i = 0u; i < info->numComponents; ++i)
        {
            delete[] info->componentNames[i];
            delete[] info->componentTypes[i];
            delete[] info->componentVersions[i];
        }
        delete[] info->componentNames;
        delete[] info->componentTypes;
        delete[] info->componentVersions;

        infoit->second.reset();
        g_fileinfos.erase (infoit);
    }
}

RRFACCESSAPI royale_rrf_api_error royale_open_input_file (royale_rrf_handle *handle, const char *filename)
{
    std::unique_ptr<FileReaderDispatcher> curInstance (new FileReaderDispatcher());

    try
    {
        curInstance->open (filename);
    }
    catch (const std::length_error &)
    {
        return royale_rrf_api_error::RRF_FILE_EMPTY;
    }
    catch (const std::runtime_error &)
    {
        return royale_rrf_api_error::RRF_WRONG_FILE_FORMAT;
    }
    catch (const std::logic_error &)
    {
        return royale_rrf_api_error::RRF_WRONG_VERSION;
    }
    catch (...)
    {
        return royale_rrf_api_error::RRF_COULD_NOT_OPEN;
    }

    *handle = g_numReaderInstances;
    g_instances[*handle] = std::move (curInstance);
    g_numReaderInstances++;

    auto ret = fill_fileinfo (*handle);
    if (ret != royale_rrf_api_error::RRF_NO_ERROR)
    {
        g_instances[*handle].reset();
        g_instances.erase (*handle);
        return ret;
    }

    return royale_rrf_api_error::RRF_NO_ERROR;
}

RRFACCESSAPI royale_rrf_api_error royale_close_input_file (const royale_rrf_handle handle)
{
    CHECK_RRFHANDLE (FileReaderDispatcher, handle, royale_rrf_api_error::RRF_HANDLE_INVALID);

    royale_rrf_api_error ret = royale_rrf_api_error::RRF_NO_ERROR;

    try
    {
        rw->close();
    }
    catch (...)
    {
        ret = royale_rrf_api_error::RRF_RUNTIME_ERROR;
    }

    free_fileinfo (handle);

    instanceit->second.reset();
    g_instances.erase (instanceit);

    return ret;
}

RRFACCESSAPI const struct royale_fileinformation_v3 *royale_get_fileinformation (const royale_rrf_handle handle)
{
    auto infoit = g_fileinfos.find (handle);
    if (infoit == g_fileinfos.end())
    {
        return NULL;
    }
    royale_fileinformation_v3 *info = static_cast<royale_fileinformation_v3 *> (infoit->second.get());

    return info;
}

RRFACCESSAPI royale_rrf_api_error royale_seek (const royale_rrf_handle handle, const uint32_t framenumber)
{
    CHECK_RRFHANDLE (FileReaderDispatcher, handle, royale_rrf_api_error::RRF_HANDLE_INVALID);
    try
    {
        rw->seek (framenumber);
    }
    catch (...)
    {
        return royale_rrf_api_error::RRF_FRAME_NOT_FOUND;
    }
    return royale_rrf_api_error::RRF_NO_ERROR;
}

RRFACCESSAPI royale_rrf_api_error royale_get_frame (const royale_rrf_handle handle, struct royale_frame_v3 **outframe)
{
    CHECK_RRFHANDLE (FileReaderDispatcher, handle, royale_rrf_api_error::RRF_HANDLE_INVALID);

    std::vector <std::vector<uint16_t>> imageData;
    std::vector <std::vector<uint16_t>> pseudoData;
    royale_frameheader_v3 frameHeader;
    std::vector <royale_streamheader_v3> streamHeaders;
    std::vector <royale_framegroupheader_v3> frameGroupHeaders;
    std::vector <royale_exposuregroupheader_v3> exposureGroupHeaders;
    std::vector <royale_rawframesetheader_v3> rawFrameSetHeaders;
    std::vector <royale_processingparameter_v3> processingParameters;
    std::vector<std::pair<std::string, std::vector<uint8_t>>> additionalData;

    try
    {
        rw->get (imageData, pseudoData, &frameHeader, streamHeaders,
                 frameGroupHeaders, exposureGroupHeaders, rawFrameSetHeaders, processingParameters, additionalData);
    }
    catch (...)
    {
        return royale_rrf_api_error::RRF_RUNTIME_ERROR;
    }

    *outframe = new royale_frame_v3;
    royale_frame_v3 *frame = *outframe;

    memset (frame, 0, sizeof (royale_frame_v3));

    // Fill the frame header
    memcpy (&frame->frameHeader, &frameHeader, sizeof (royale_frameheader_v3));

    // Fill stream headers
    if (frame->frameHeader.numStreams)
    {
        frame->streamHeaders = new royale_streamheader_v3[frame->frameHeader.numStreams];

        for (auto i = 0u; i < frame->frameHeader.numStreams; ++i)
        {
            memcpy (&frame->streamHeaders[i], &streamHeaders[i], sizeof (royale_streamheader_v3));
        }
    }

    // Fill frame group headers
    frame->frameGroupHeaders = new royale_framegroupheader_v3[frame->frameHeader.numFrameGroups];

    for (auto i = 0u; i < frame->frameHeader.numFrameGroups; ++i)
    {
        memcpy (&frame->frameGroupHeaders[i], &frameGroupHeaders[i], sizeof (royale_framegroupheader_v3));
    }

    // Fill exposure group headers
    frame->exposureGroupHeaders = new royale_exposuregroupheader_v3[frame->frameHeader.numExposureGroups];

    for (auto i = 0u; i < frame->frameHeader.numExposureGroups; ++i)
    {
        memcpy (&frame->exposureGroupHeaders[i], &exposureGroupHeaders[i], sizeof (royale_exposuregroupheader_v3));
    }

    // Fill raw frame sets
    if (frame->frameHeader.numRawFrames)
    {
        frame->rawFrameSetHeaders = new royale_rawframesetheader_v3[frame->frameHeader.numRawFrameSets];

        for (auto i = 0u; i < frame->frameHeader.numRawFrameSets; ++i)
        {
            memcpy (&frame->rawFrameSetHeaders[i], &rawFrameSetHeaders[i], sizeof (royale_rawframesetheader_v3));
        }
    }

    // Copy the parameters
    if (frame->frameHeader.numParameters)
    {
        frame->processingParameters = new royale_processingparameter_v3[frame->frameHeader.numParameters];

        for (auto i = 0u; i < frame->frameHeader.numParameters; ++i)
        {
            memcpy (&frame->processingParameters[i], &processingParameters[i], sizeof (royale_processingparameter_v3));
        }
    }

    // Copy the additional data
    if (frame->frameHeader.numAdditionalData)
    {
        frame->additionalData = new royale_additionaldata_v3[frame->frameHeader.numAdditionalData];

        int i = 0;
        for (auto curData = additionalData.begin(); curData != additionalData.end(); ++curData, ++i)
        {
            frame->additionalData[i].dataSize = static_cast<uint64_t> (curData->second.size());
            frame->additionalData[i].data = new uint8_t[static_cast<size_t> (frame->additionalData[i].dataSize)];
            memcpy (frame->additionalData[i].data, &curData->second[0], static_cast<size_t> (frame->additionalData[i].dataSize));
            std::string tempName = curData->first;
            tempName.resize (ROYALE_FILEHEADER_V3_ADDITIONAL_DATA_NAME_LENGTH);
            memcpy (frame->additionalData[i].dataName, tempName.c_str(), ROYALE_FILEHEADER_V3_ADDITIONAL_DATA_NAME_LENGTH);
        }
    }

    // Copy the image data
    frame->imageData = new uint16_t *[imageData.size()];
    for (auto i = 0u; i < imageData.size(); ++i)
    {
        frame->imageData[i] = new uint16_t[imageData[i].size()];
        memcpy (&frame->imageData[i][0], &imageData[i][0], sizeof (uint16_t) * imageData[i].size ());
    }

    frame->pseudoData = new uint16_t *[pseudoData.size()];
    for (auto i = 0u; i < pseudoData.size(); ++i)
    {
        frame->pseudoData[i] = new uint16_t[pseudoData[i].size()];
        memcpy (&frame->pseudoData[i][0], &pseudoData[i][0], sizeof (uint16_t) * pseudoData[i].size());
    }

    return royale_rrf_api_error::RRF_NO_ERROR;
}

RRFACCESSAPI royale_rrf_api_error royale_free_frame (struct royale_frame_v3 **inframe)
{
    if (!inframe)
    {
        return royale_rrf_api_error::RRF_NO_VALID_FRAME;
    }

    royale_frame_v3 *frame = *inframe;

    if (!frame)
    {
        return royale_rrf_api_error::RRF_NO_VALID_FRAME;
    }

    for (auto i = 0u; i < frame->frameHeader.numRawFrames; ++i)
    {
        delete[] frame->imageData[i];
        delete[] frame->pseudoData[i];
    }

    delete[] frame->imageData;
    delete[] frame->pseudoData;
    delete[] frame->streamHeaders;
    delete[] frame->frameGroupHeaders;
    delete[] frame->exposureGroupHeaders;
    delete[] frame->rawFrameSetHeaders;
    delete[] frame->processingParameters;

    for (auto i = 0u; i < frame->frameHeader.numAdditionalData; ++i)
    {
        delete[] frame->additionalData[i].data;
    }

    delete[] frame->additionalData;

    delete frame;
    frame = nullptr;

    return royale_rrf_api_error::RRF_NO_ERROR;
}

RRFACCESSAPI uint32_t royale_get_num_frames (const royale_rrf_handle handle)
{
    CHECK_RRFHANDLE (FileReaderDispatcher, handle, 0u);
    try
    {
        return rw->numFrames();
    }
    catch (...)
    {
        return 0u;
    }
}

RRFACCESSAPI uint32_t royale_get_current_frame (const royale_rrf_handle handle)
{
    CHECK_RRFHANDLE (FileReaderDispatcher, handle, 0u);
    try
    {
        return rw->currentFrame();
    }
    catch (...)
    {
        return 0u;
    }
}

RRFACCESSAPI uint16_t royale_get_max_width (const royale_rrf_handle handle)
{
    CHECK_RRFHANDLE (FileReaderDispatcher, handle, 0u);
    try
    {
        return rw->getMaxWidth();
    }
    catch (...)
    {
        return 0u;
    }
}

RRFACCESSAPI uint16_t royale_get_max_height (const royale_rrf_handle handle)
{
    CHECK_RRFHANDLE (FileReaderDispatcher, handle, 0u);
    try
    {
        return rw->getMaxHeight();
    }
    catch (...)
    {
        return 0u;
    }
}

RRFACCESSAPI bool royale_is_open (const royale_rrf_handle handle)
{
    CHECK_RRFHANDLE (FileReaderDispatcher, handle, false);
    try
    {
        return rw->isOpen();
    }
    catch (...)
    {
        return false;
    }
}

RRFACCESSAPI uint16_t royale_get_version (const royale_rrf_handle handle)
{
    CHECK_RRFHANDLE (FileReaderDispatcher, handle, 0u);

    try
    {
        return rw->getFileVersion();
    }
    catch (...)
    {
        return 0u;
    }
}
