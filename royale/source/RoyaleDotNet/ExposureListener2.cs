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
    public interface IExposureListener2
    {
        void OnNewExposure (UInt32 exposureTime, UInt16 streamId);
    }

    //\cond HIDDEN_SYMBOLS
    internal class ExposureListener2
    {
        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void copyDataBlocked (IntPtr dest, IntPtr src, UInt32 length);

        [UnmanagedFunctionPointer (CallingConvention.Cdecl)]
        internal delegate void NativeOnNewExposureCallback (UInt32 exposureTime, UInt16 streamId);
        internal delegate void ManagedOnNewExposureCallback (UInt32 exposureTime, UInt16 streamId);

        internal NativeOnNewExposureCallback nativeCallback;
        internal ManagedOnNewExposureCallback managedCallback;

        internal ExposureListener2 (ManagedOnNewExposureCallback managedOnNewData)
        {
            nativeCallback = new NativeOnNewExposureCallback (NativeOnNewData);
            managedCallback = managedOnNewData;
        }

        internal void NativeOnNewData (UInt32 exposureTime, UInt16 streamId)
        {
            managedCallback (exposureTime, streamId);
        }
    }
    //\endcond
}
