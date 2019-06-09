/****************************************************************************\
* Copyright (C) 2016 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

/**
* File header for version 3.
* https://git.ifm.com/projects/ROYALE/repos/royaledoc/browse/content/rrf_file_format_v3.md
*/

#include <record/CommonHeader.h>
#include <record/v2/FileHeader.h>

#define ROYALE_FILEHEADER_V3_SERIAL_LENGTH ROYALE_FILEHEADER_V2_SERIAL_LENGTH
#define ROYALE_FILEHEADER_V3_CHECKSUM_LENGTH ROYALE_FILEHEADER_V2_CHECKSUM_LENGTH
#define ROYALE_FILEHEADER_V3_USE_CASE_LENGTH ROYALE_FILEHEADER_V2_USE_CASE_LENGTH
#define ROYALE_FILEHEADER_V3_CAMERA_NAME_LENGTH ROYALE_FILEHEADER_V2_CAMERA_NAME_LENGTH
#define ROYALE_FILEHEADER_V3_IMAGER_TYPE_LENGTH ROYALE_FILEHEADER_V2_IMAGER_TYPE_LENGTH
#define ROYALE_FILEHEADER_V3_PSEUDODATA_INTER_LENGTH ROYALE_FILEHEADER_V2_PSEUDODATA_INTER_LENGTH
#define ROYALE_FILEHEADER_V3_COMPONENT_NAME_LENGTH ROYALE_FILEHEADER_V2_COMPONENT_NAME_LENGTH
#define ROYALE_FILEHEADER_V3_COMPONENT_TYPE_LENGTH 20
#define ROYALE_FILEHEADER_V3_COMPONENT_VERSION_LENGTH ROYALE_FILEHEADER_V2_COMPONENT_VERSION_LENGTH
#define ROYALE_FILEHEADER_V3_ADDITIONAL_DATA_NAME_LENGTH ROYALE_FILEHEADER_V2_ADDITIONAL_DATA_NAME_LENGTH
#define ROYALE_FILEHEADER_V3_EXPOSUREGROUP_NAME_LENGTH 20
#define ROYALE_FILEHEADER_V3_STREAM_MAX_FRAMEGROUPS 128
#define ROYALE_FILEHEADER_V3_FRAMEGROUP_MAX_RFS 128

#pragma pack(push, 1)

typedef struct
{
    char            magic[ROYALE_FILEHEADER_MAGIC_LENGTH];
    uint8_t         version;
    uint8_t         platform;
    char            cameraName[ROYALE_FILEHEADER_V3_CAMERA_NAME_LENGTH];
    char            imagerType[ROYALE_FILEHEADER_V3_IMAGER_TYPE_LENGTH];
    char            pseudoDataInterpreter[ROYALE_FILEHEADER_V3_PSEUDODATA_INTER_LENGTH];
    uint32_t        royaleMajor;
    uint32_t        royaleMinor;
    uint32_t        royalePatch;
    uint32_t        royaleBuild;
    char            imagerSerial[ROYALE_FILEHEADER_V3_SERIAL_LENGTH];
    uint32_t        numFrames;
    uint8_t         compressionType;
    uint64_t        calibrationOffset;
    uint32_t        calibrationSize;
    uint64_t        framesOffset;
    uint64_t        framesSize;
    uint8_t         checksum[ROYALE_FILEHEADER_V3_CHECKSUM_LENGTH];
    uint32_t        numVersions;
    uint8_t         reserved[66];
} royale_fileheader_v3;

typedef struct
{
    char            componentName[ROYALE_FILEHEADER_V3_COMPONENT_NAME_LENGTH];
    char            componentType[ROYALE_FILEHEADER_V3_COMPONENT_TYPE_LENGTH];
    char            componentVersion[ROYALE_FILEHEADER_V3_COMPONENT_VERSION_LENGTH];
} royale_versioninformation_v3;

typedef royale_processingparameter_v2 royale_processingparameter_v3;

typedef struct
{
    uint32_t        frameSize;
    uint16_t        targetFrameRate;
    uint16_t        numColumns;
    uint16_t        numRows;
    uint16_t        numRawFrames;
    uint16_t        numRawFrameSets;
    uint64_t        timestamp;
    float           illuTemperature;
    uint32_t        numParameters;
    char            useCaseName[ROYALE_FILEHEADER_V3_USE_CASE_LENGTH];
    uint32_t        numAdditionalData;
    uint16_t        curStreamId;
    uint16_t        numStreams;
    uint16_t        numFrameGroups;
    uint16_t        numExposureGroups;
} royale_frameheader_v3;

typedef struct
{
    uint16_t        streamId;
    uint16_t        numFrameGroups;
    uint16_t        frameGroupIdxs[ROYALE_FILEHEADER_V3_STREAM_MAX_FRAMEGROUPS];
} royale_streamheader_v3;

typedef struct
{
    uint16_t        numRawFrameSets;
    uint16_t        rawFrameSetIdxs[ROYALE_FILEHEADER_V3_FRAMEGROUP_MAX_RFS];
} royale_framegroupheader_v3;

typedef struct
{
    char            exposureGroupName[ROYALE_FILEHEADER_V3_EXPOSUREGROUP_NAME_LENGTH];
    uint32_t        exposureMin;
    uint32_t        exposureMax;
    uint32_t        exposureTime;
} royale_exposuregroupheader_v3;

typedef struct
{
    uint32_t        modFreq;
    uint32_t        capturedExpTime;
    uint8_t         phaseDefinition;
    uint16_t        numRawFrames;
    uint16_t        dutyCycle;
    uint16_t        alignment;
    uint16_t        exposureGroupIdx;
    double          eyeSafetyGap;
} royale_rawframesetheader_v3;

typedef struct
{
    char            dataName[ROYALE_FILEHEADER_V3_ADDITIONAL_DATA_NAME_LENGTH];
    uint64_t        dataSize;
    uint8_t        *data;
} royale_additionaldata_v3;

#pragma pack(pop)

struct royale_frame_v3
{
    uint16_t                                **imageData;
    uint16_t                                **pseudoData;
    royale_frameheader_v3                     frameHeader;
    royale_streamheader_v3                   *streamHeaders;
    royale_framegroupheader_v3               *frameGroupHeaders;
    royale_exposuregroupheader_v3            *exposureGroupHeaders;
    royale_rawframesetheader_v3              *rawFrameSetHeaders;
    royale_processingparameter_v3            *processingParameters;
    royale_additionaldata_v3                 *additionalData;
};

struct royale_fileinformation_v3
{
    /// Current calibration data
    uint8_t                 *calibrationData;
    /// Size of the calibration data
    uint32_t                 calibrationDataSize;
    /// Serial number of the imager used
    char                    *imagerSerial;
    /// Camera name used
    char                    *cameraName;
    /// Imager type used
    char                    *imagerType;
    /// Pseudo data interpreter used
    char                    *pseudoDataInter;
    /// Royale major version used for the recording
    uint32_t                 royaleMajor;
    /// Royale minor version used for the recording
    uint32_t                 royaleMinor;
    /// Royale patch version used for the recording
    uint32_t                 royalePatch;
    /// Royale build version used for the recording
    uint32_t                 royaleBuild;
    /// Platform used for the recording
    royale_rrf_platformtype  platform;
    /// Number of components
    uint16_t                 numComponents;
    /// Version information(component names)
    char                   **componentNames;
    /// Version information(component types)
    char                   **componentTypes;
    /// Version information(component versions)
    char                   **componentVersions;
};

typedef enum
{
    GRAYSCALE = 1,
    MODULATED_4PH_CW = 0
} royale_phasedefinition_v3;
