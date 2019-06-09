/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <ProcessingParametersCAPI.h>
#include <private/ProcessingParametersConverterCAPI.hpp>
#include <private/HelperFunctionsCAPI.hpp>

using namespace royale;

static_assert (ProcessingFlag::NUM_FLAGS == static_cast<ProcessingFlag> (ROYALE_PROC_FLAG_NUM_FLAGS),
               "The C++, CAPI and probably the DotNet ProcessingFlag enums are out of sync");

ROYALE_CAPI_LINKAGE_TOP
void ProcessingParametersConverterCAPI::fromRoyaleProcessingParameter (royale_processing_parameter *dst, const Pair<ProcessingFlag, Variant> src)
{
    dst->flag = (royale_processing_flag) src.first;

    switch (src.second.variantType())
    {
        case VariantType::Bool:
            dst->value.type = ROYALE_VARIANT_TYPE_BOOL;
            dst->value.data.b = src.second.getBool();
            break;
        case VariantType::Int:
            dst->value.type = ROYALE_VARIANT_TYPE_INT;
            dst->value.data.i = src.second.getInt();
            dst->value.int_min = src.second.getIntMin();
            dst->value.int_max = src.second.getIntMax();
            break;
        case VariantType::Float:
            dst->value.type = ROYALE_VARIANT_TYPE_FLOAT;
            dst->value.data.f = src.second.getFloat();
            dst->value.float_min = src.second.getFloatMin();
            dst->value.float_max = src.second.getFloatMax();
            break;
        default:
            break;
    }
}

Pair<ProcessingFlag, Variant> ProcessingParametersConverterCAPI::toRoyaleProcessingParameter (royale_processing_parameter *src)
{
    Pair<ProcessingFlag, Variant> dst;
    dst.first = (ProcessingFlag) src->flag;

    switch (src->value.type)
    {
        case ROYALE_VARIANT_TYPE_BOOL:
            dst.second.setBool (src->value.data.b);
            break;
        case ROYALE_VARIANT_TYPE_INT:
            dst.second = Variant (src->value.data.i, src->value.int_min, src->value.int_max);
            break;
        case ROYALE_VARIANT_TYPE_FLOAT:
            dst.second = Variant (src->value.data.f, src->value.float_min, src->value.float_max);
            break;
        default:
            break;
    }
    return dst;
}

ROYALE_CAPI void royale_proc_flag_get_flag_name (char **name_dst, royale_processing_flag flag)
{
    HelperFunctionsCAPI::copyRoyaleStringToCString (name_dst, getProcessingFlagName ( (ProcessingFlag) flag));
}

ROYALE_CAPI_LINKAGE_BOTTOM
