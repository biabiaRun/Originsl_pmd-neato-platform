/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

/**
 * \addtogroup RoyaleDotNet
 * @{
 */

using System;
using System.Collections.Generic;

namespace RoyaleDotNet
{
    //\cond HIDDEN_SYMBOLS
    internal static class RoyaleDotNET
    {
#if DEBUG
        internal const string royaleCAPI_DLL = "royaleCAPI-d";
#else
        internal const string royaleCAPI_DLL = "royaleCAPI";
#endif
    }
    //\endcond
}
/** @}*/
