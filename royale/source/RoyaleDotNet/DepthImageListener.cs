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
    /// Provides the listener interface for consuming depth images from Royale. A listener needs
    /// to implement this interface and register itself as a listener to the ICameraDevice.
    /// </summary>
    public interface IDepthImageListener
    {
        /// <summary>
        /// Will be called on every frame update by the Royale framework
        ///
        /// NOTICE:
        /// Calling other framework functions within the data callback
        /// can lead to undefined behavior and is therefore unsupported.
        /// Call these framework functions from another thread to avoid problems.
        /// </summary>
        /// <param name="data">DepthImage of the captured frame.</param>
        void OnNewData (DepthImage data);
    }

    //\cond HIDDEN_SYMBOLS
    internal class DepthImageListener
    {
        [UnmanagedFunctionPointer (CallingConvention.Cdecl)]
        internal delegate void NativeOnNewDataCallback (IntPtr data);
        internal delegate void ManagedOnNewDataCallback (DepthImage data);

        internal NativeOnNewDataCallback nativeCallback;
        internal ManagedOnNewDataCallback managedCallback;


        internal DepthImageListener (ManagedOnNewDataCallback managedOnNewData)
        {
            nativeCallback = new NativeOnNewDataCallback (NativeOnNewData);
            managedCallback = managedOnNewData;
        }

        internal void NativeOnNewData (IntPtr data)
        {
            DepthImage depthImage = DepthImageConverter.FromNativeData (data);
            managedCallback (depthImage);
        }
    }
    //\endcond
}
