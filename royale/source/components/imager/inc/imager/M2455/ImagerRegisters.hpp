/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag & Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <imager/M2453/ImagerRegisters.hpp>

namespace royale
{
    namespace imager
    {
        namespace M2455
        {
            using namespace royale::imager::same_in_M2453_and_M2455;
        }

        namespace M2455_A11
        {
            const uint16_t ANAIP_EFUSEVAL1 = 0xa097; //EFUSE Value 1
            const uint16_t ANAIP_EFUSEVAL2 = 0xa098; //EFUSE Value 2
            const uint16_t ANAIP_EFUSEVAL3 = 0xa099; //EFUSE Value 3
            const uint16_t ANAIP_EFUSEVAL4 = 0xa09A; //EFUSE Value 4
        }
    }
}
