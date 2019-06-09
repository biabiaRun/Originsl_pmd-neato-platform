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
    /// Provides the listener interface for consuming IR images from Royale. A listener needs
    /// to implement this interface and register itself as a listener to the ICameraDevice.
    /// </summary>
    public interface IIRImageListener
    {
        /// <summary>
        /// Will be called on every frame update by the Royale framework
        ///
        /// NOTICE:
        /// Calling other framework functions within the data callback
        /// can lead to undefined behavior and is therefore unsupported.
        /// Call these framework functions from another thread to avoid problems.
        /// </summary>
        /// <param name="data">IRImage of the captured frame.</param>
        void OnNewData (IRImage data);
    }

    //\cond HIDDEN_SYMBOLS
    internal class IRImageListener
    {
        [UnmanagedFunctionPointer (CallingConvention.Cdecl)]
        internal delegate void NativeOnNewDataCallback (IntPtr data);
        internal delegate void ManagedOnNewDataCallback (IRImage data);

        internal NativeOnNewDataCallback nativeCallback;
        internal ManagedOnNewDataCallback managedCallback;


        internal IRImageListener (ManagedOnNewDataCallback managedOnNewData)
        {
            nativeCallback = new NativeOnNewDataCallback (NativeOnNewData);
            managedCallback = managedOnNewData;
        }

        internal void NativeOnNewData (IntPtr data)
        {
            IRImage depthImage = IRImageConverter.FromNativeData (data);
            managedCallback (depthImage);
        }
    }
    //\endcond
}
