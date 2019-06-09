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

#include <record/v3/FileHeader.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* Opens a file for recording.
* @param handle Handle to identify the RRF writer instance
* @param filename Filename which should be used
* @param fileinfo File information \see{struct royale_fileinformation_v3}
*/
RRFACCESSAPI royale_rrf_api_error royale_open_output_file (royale_rrf_handle *handle, const char *filename,
        const struct royale_fileinformation_v3 *fileinfo);

/**
* Closes the current recording and writes the final FileHeader.
* @param handle Handle to identify the RRF writer instance
*/
RRFACCESSAPI royale_rrf_api_error royale_close_output_file (const royale_rrf_handle handle);


/**
* Writes a frame to the currently opened file.
* @param handle Handle to identify the RRF writer instance
* @param frame Royale frame
*/
RRFACCESSAPI royale_rrf_api_error royale_output_data (const royale_rrf_handle handle, struct royale_frame_v3 *frame);

#ifdef __cplusplus
}
#endif
