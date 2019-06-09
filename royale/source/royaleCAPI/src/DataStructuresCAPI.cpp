/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <DataStructuresCAPI.h>
#include <cstdlib>
#include <memory>

ROYALE_CAPI void royale_free_pair_string_string (royale_pair_string_string *pair)
{
    free ( (void *) pair->first);
    free ( (void *) pair->second);
}

ROYALE_CAPI void royale_free_pair_string_string_array (royale_pair_string_string **pair_array, uint32_t nr_elements)
{
    for (uint32_t i = 0; i < nr_elements; i++)
    {
        royale_free_pair_string_string (& (*pair_array) [i]);
    }
    free (*pair_array);
}

ROYALE_CAPI void royale_pair_string_uint64_free (royale_pair_string_uint64 *pair)
{
    free ( (void *) pair->first);
}

ROYALE_CAPI void royale_free_pair_string_uint64_array (royale_pair_string_uint64 **pair_array, uint32_t nr_elements)
{
    for (uint32_t i = 0; i < nr_elements; i++)
    {
        royale_pair_string_uint64_free (pair_array[i]);
    }
    free (pair_array);
}

ROYALE_CAPI void royale_free_vector_float (royale_vector_float *vector)
{
    free (vector->values);
}

ROYALE_CAPI void royale_free_vector_stream_id (royale_vector_stream_id *vector)
{
    free (vector->values);
}

ROYALE_CAPI void royale_free_string_array (char **string_array, uint32_t nr_strings)
{
    royale_free_string_array2 (&string_array, nr_strings);
}

ROYALE_CAPI void royale_free_string_array2 (char ***string_array, uint32_t nr_strings)
{
    for (uint32_t i = 0; i < nr_strings; i++)
    {
        free ( (void *) (*string_array) [i]);
    }
    free ( (void *) *string_array);
}

ROYALE_CAPI void royale_free_string (char *string_ptr)
{
    free ( (void *) string_ptr);
}
