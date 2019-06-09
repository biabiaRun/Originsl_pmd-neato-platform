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

#include <stdbool.h>
#include <record/v3/FileHeader.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* Opens a file for read out.
* @param handle Handle to identify the RRF reader instance
* @param filename File that should be opened
*/
RRFACCESSAPI royale_rrf_api_error royale_open_input_file (royale_rrf_handle *handle, const char *filename);

/**
* Closes the current file.
* This will reset the handle.
* @param handle Handle to identify the RRF reader instance
*/
RRFACCESSAPI royale_rrf_api_error royale_close_input_file (const royale_rrf_handle handle);

/**
* Retrieves information of the current file.
* This will return a pointer to the internal file information struct \see{struct royale_fileinformation_v3}.
* @param handle Handle to identify the RRF reader instance
*/
RRFACCESSAPI const struct royale_fileinformation_v3 *royale_get_fileinformation (const royale_rrf_handle handle);

/**
* Seeks the recording to the specified frame.
* @param handle Handle to identify the RRF reader instance
* @param framenumber Frame that will be returned next
*/
RRFACCESSAPI royale_rrf_api_error royale_seek (const royale_rrf_handle handle, const uint32_t framenumber);

/**
* Retrieves the current frame from the recording \see{struct royale_frame_v3}.
* The frame should be freed with royale_freeFrame afterwards.
* @param handle Handle to identify the RRF reader instance
* @param outframe Frame that is read
*/
RRFACCESSAPI royale_rrf_api_error royale_get_frame (const royale_rrf_handle handle, struct royale_frame_v3 **outframe);

/**
* Frees the memory occupied by the given frame.
* @param inframe Frame that should be freed
*/
RRFACCESSAPI royale_rrf_api_error royale_free_frame (struct royale_frame_v3 **inframe);

/**
* Retrieves the number of frames in the recording.
* @param handle Handle to identify the RRF reader instance
*/
RRFACCESSAPI uint32_t royale_get_num_frames (const royale_rrf_handle handle);

/**
* Retrieves the current frame number.
* @param handle Handle to identify the RRF reader instance
*/
RRFACCESSAPI uint32_t royale_get_current_frame (const royale_rrf_handle handle);

/**
* Retrieves the maximum width of the recording (the maximum
* number of columns that are used).
* @param handle Handle to identify the RRF reader instance
*/
RRFACCESSAPI uint16_t royale_get_max_width (const royale_rrf_handle handle);

/**
* Retrieves the maximum height of the recording (the maximum
* number of rows that are used).
* @param handle Handle to identify the RRF reader instance
*/
RRFACCESSAPI uint16_t royale_get_max_height (const royale_rrf_handle handle);

/**
* Checks if a file is open.
* @param handle Handle to identify the RRF reader instance
*/
RRFACCESSAPI bool royale_is_open (const royale_rrf_handle handle);

/**
* Retrieves the file format version.
* @param handle Handle to identify the RRF reader instance
*/
RRFACCESSAPI uint16_t royale_get_version (const royale_rrf_handle handle);

#ifdef __cplusplus
}
#endif
