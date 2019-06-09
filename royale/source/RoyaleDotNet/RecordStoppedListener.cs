/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
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
    /// This interface needs to be implemented if the client wants to get notified when recording stopped after
    /// the specified number of frames.
    /// </summary>
    public interface IRecordStoppedListener
    {
        /// <summary>
        /// Will be called if the recording is stopped.
        /// </summary>
        /// <param name="numFrames">Number of frames that have been recorded.</param>
        void OnRecordStopped (UInt32 numFrames);
    }

    //\cond HIDDEN_SYMBOLS
    internal class RecordStoppedListener
    {
        [UnmanagedFunctionPointer (CallingConvention.Cdecl)]
        internal delegate void NativeOnRecordStoppedCallback (UInt32 numFrames);
        internal delegate void ManagedOnRecordStoppedCallback (UInt32 numFrames);

        internal NativeOnRecordStoppedCallback nativeOnRecordStopped;
        internal ManagedOnRecordStoppedCallback managedOnRecordStopped;

        internal RecordStoppedListener (ManagedOnRecordStoppedCallback onRecordStoppedCB)
        {
            nativeOnRecordStopped = new NativeOnRecordStoppedCallback (NativeOnRecordStopped);
            managedOnRecordStopped = onRecordStoppedCB;
        }

        internal void NativeOnRecordStopped (UInt32 numFrames)
        {
            managedOnRecordStopped (numFrames);
        }
    }
    //\endcond
}