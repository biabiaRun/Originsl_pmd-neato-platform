/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale/ProcessingFlag.hpp>
#include <ProcessingParametersCAPI.h>

class ProcessingParametersConverterCAPI
{
public:
    static void fromRoyaleProcessingParameter (royale_processing_parameter *dst, const royale::Pair<royale::ProcessingFlag, royale::Variant> src);
    static royale::Pair<royale::ProcessingFlag, royale::Variant> toRoyaleProcessingParameter (royale_processing_parameter *src);
};