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
* File header for version 1.
* https://git.ifm.com/projects/ROYALE/repos/royaledoc/browse/content/rrf_file_format_v1.md
*/

#include <record/CommonHeader.h>

#define ROYALE_FILEHEADER_V1_SERIAL_LENGTH 19
#define ROYALE_FILEHEADER_V1_CHECKSUM_LENGTH 32
#define ROYALE_FILEHEADER_V1_USE_CASE_LENGTH 26

#pragma pack(push, 1)

typedef struct
{
    char            magic[ROYALE_FILEHEADER_MAGIC_LENGTH];
    uint8_t         version;
    uint8_t         platform;
    uint32_t        cameraType;
    uint16_t        imagerType;
    uint32_t        royaleMajor;
    uint32_t        royaleMinor;
    uint32_t        royalePatch;
    char            imagerSerial[ROYALE_FILEHEADER_V1_SERIAL_LENGTH];
    uint32_t        numFrames;
    uint8_t         compressionType;
    uint64_t        calibrationOffset;
    uint32_t        calibrationSize;
    uint64_t        framesOffset;
    uint64_t        framesSize;
    uint8_t         checksum[ROYALE_FILEHEADER_V1_CHECKSUM_LENGTH];
    uint8_t         reserved[139];
} royale_fileheader_v1;

typedef struct
{
    uint32_t        processingFlag;
    uint8_t         dataType;
    uint32_t        value;
} royale_processingparameter_v1;

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
    char            useCaseName[ROYALE_FILEHEADER_V1_USE_CASE_LENGTH];
} royale_frameheader_v1;

typedef struct
{
    uint32_t        modFreq;
    uint32_t        useCaseExpTime;
    uint32_t        capturedExpTime;
    uint8_t         phaseDefinition;
    uint16_t        numRawFrames;
    uint32_t        rawFrameSize;
    uint8_t         active;
} royale_rawframesetheader_v1;

typedef struct
{
    uint16_t        rawFrameIndex;
    uint16_t        frameRate;
    uint16_t        phaseAngle;
} royale_rawframeheader_v1;

#pragma pack(pop)

typedef enum
{
    MODULATED,
    GREYSCALE
} royale_phasedefinition_v1;
