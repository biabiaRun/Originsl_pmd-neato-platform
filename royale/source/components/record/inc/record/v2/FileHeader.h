/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

/**
* File header for version 2.
* https://git.ifm.com/projects/ROYALE/repos/royaledoc/browse/content/rrf_file_format_v2.md
*/

#include <record/CommonHeader.h>
#include <record/v1/FileHeader.h>

#define ROYALE_FILEHEADER_V2_SERIAL_LENGTH 20
#define ROYALE_FILEHEADER_V2_CHECKSUM_LENGTH ROYALE_FILEHEADER_V1_CHECKSUM_LENGTH
#define ROYALE_FILEHEADER_V2_USE_CASE_LENGTH ROYALE_FILEHEADER_V1_USE_CASE_LENGTH
#define ROYALE_FILEHEADER_V2_CAMERA_NAME_LENGTH 30
#define ROYALE_FILEHEADER_V2_IMAGER_TYPE_LENGTH 20
#define ROYALE_FILEHEADER_V2_PSEUDODATA_INTER_LENGTH 20
#define ROYALE_FILEHEADER_V2_COMPONENT_NAME_LENGTH 20
#define ROYALE_FILEHEADER_V2_COMPONENT_VERSION_LENGTH 10
#define ROYALE_FILEHEADER_V2_ADDITIONAL_DATA_NAME_LENGTH 20

#pragma pack(push, 1)

typedef struct
{
    char            magic[ROYALE_FILEHEADER_MAGIC_LENGTH];
    uint8_t         version;
    uint8_t         platform;
    char            cameraName[ROYALE_FILEHEADER_V2_CAMERA_NAME_LENGTH];
    char            imagerType[ROYALE_FILEHEADER_V2_IMAGER_TYPE_LENGTH];
    char            pseudoDataInterpreter[ROYALE_FILEHEADER_V2_PSEUDODATA_INTER_LENGTH];
    uint32_t        royaleMajor;
    uint32_t        royaleMinor;
    uint32_t        royalePatch;
    uint32_t        royaleBuild;
    char            imagerSerial[ROYALE_FILEHEADER_V2_SERIAL_LENGTH];
    uint32_t        numFrames;
    uint8_t         compressionType;
    uint64_t        calibrationOffset;
    uint32_t        calibrationSize;
    uint64_t        framesOffset;
    uint64_t        framesSize;
    uint8_t         checksum[ROYALE_FILEHEADER_V2_CHECKSUM_LENGTH];
    uint32_t        numVersions;
    uint8_t         reserved[66];
} royale_fileheader_v2;

typedef struct
{
    char            componentName[ROYALE_FILEHEADER_V2_COMPONENT_NAME_LENGTH];
    char            componentVersion[ROYALE_FILEHEADER_V2_COMPONENT_VERSION_LENGTH];
} royale_versioninformation_v2;

typedef royale_processingparameter_v1 royale_processingparameter_v2;

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
    uint32_t        exposureMin;
    uint32_t        exposureMax;
    char            useCaseName[ROYALE_FILEHEADER_V2_USE_CASE_LENGTH];
    uint32_t        numAdditionalData;
} royale_frameheader_v2;

typedef royale_rawframesetheader_v1 royale_rawframesetheader_v2;

typedef royale_rawframeheader_v1 royale_rawframeheader_v2;

typedef struct
{
    char            dataName[ROYALE_FILEHEADER_V2_ADDITIONAL_DATA_NAME_LENGTH];
    uint64_t        dataSize;
    uint8_t         *data;
} royale_additionaldata_v2;

#pragma pack(pop)

struct royale_frame_v2
{
    uint16_t                                **imageData;
    uint16_t                                **pseudoData;
    royale_frameheader_v2                     frameHeader;
    royale_rawframesetheader_v2              *rawFrameSetHeaders;
    royale_rawframeheader_v2                 *rawFrameHeaders;
    royale_processingparameter_v2            *processingParameters;
    royale_additionaldata_v2                 *additionalData;
};

struct royale_fileinformation_v2
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
    /// Version information(component names)
    char                   **componentNames;
    /// Number of component names
    uint16_t                 numComponentNames;
    /// Version information(component versions)
    char                   **componentVersions;
    /// Number of component versions
    uint16_t                 numComponentVersions;
};

typedef royale_phasedefinition_v1 royale_phasedefinition_v2;
