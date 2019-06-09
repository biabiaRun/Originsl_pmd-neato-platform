/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace RoyaleDotNet
{
    /// <summary>
    /// StreamIds are represented by UInt16s.  The StreamId class itself is only for handling
    /// the conversion of native data.
    /// </summary>
    public sealed class StreamId
    {
        [Obsolete ("StreamIds are represented by UInt16, not by a StreamId class")]
        private StreamId()
        {
        }

        [Obsolete ("StreamIds are represented by UInt16, not by a StreamId class")]
        private StreamId (UInt16 streamId)
        {
        }

        [StructLayout (LayoutKind.Sequential)]
        internal struct NativeVectorStreamId
        {
            public IntPtr values;
            public UInt32 numValues;
        }

        static internal UInt16[] convertNative (NativeVectorStreamId nativeVector)
        {
            UInt16[] result = new UInt16[nativeVector.numValues];
            short[] signedResult = new short[nativeVector.numValues];
            Marshal.Copy (nativeVector.values, signedResult, 0, (int) nativeVector.numValues);
            for (int i = 0; i < nativeVector.numValues; i++)
            {
                result[i] = unchecked ((UInt16) signedResult[i]);
            }
            return result;
        }
    }
}
