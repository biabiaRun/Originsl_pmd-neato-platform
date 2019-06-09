/****************************************************************************\
* Copyright (C) 2015 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <usecase/UseCaseDefinition.hpp>

#include <record/v3/FileHeader.h>

namespace royale
{
    namespace record
    {
        struct RecordUseCaseDefinition
        {
            royale_frameheader_v3                           frameHeader;
            std::vector<royale_streamheader_v3>             streams;
            std::vector<royale_exposuregroupheader_v3>      exposureGroups;
            std::vector<royale_framegroupheader_v3>         frameGroups;
            std::vector<royale_rawframesetheader_v3>        rawFrameSets;
        };

        class UseCaseRecord : public royale::usecase::UseCaseDefinition
        {
        public:
            explicit UseCaseRecord (const royale::record::RecordUseCaseDefinition &definition);
        };
    }
}
