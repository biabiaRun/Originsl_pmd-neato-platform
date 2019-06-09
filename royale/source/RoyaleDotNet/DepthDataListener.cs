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
    /// Provides the listener interface for consuming depth data from royale. A listener needs
    /// to implement this interface and register itself as a listener to the CameraDevice.
    /// </summary>
    public interface IDepthDataListener
    {
        /// <summary>
        /// Will be called on every frame update by the royale framework
        ///
        /// NOTICE:
        /// Calling other framework functions within the data callback
        /// can lead to undefined behavior and is therefore unsupported.
        /// Call these framework functions from another thread to avoid problems.
        /// </summary>
        /// <param name="data">DepthData of the captured frame.</param>
        void OnNewData (DepthData data);
    }

    //\cond HIDDEN_SYMBOLS
    internal class DepthDataListener
    {
        [UnmanagedFunctionPointer (CallingConvention.Cdecl)]
        internal delegate void NativeOnNewDataCallback (IntPtr data);
        internal delegate void ManagedOnNewDataCallback (DepthData data);

        internal NativeOnNewDataCallback nativeCallback;
        internal ManagedOnNewDataCallback managedCallback;


        internal DepthDataListener (ManagedOnNewDataCallback managedOnNewData)
        {
            nativeCallback = new NativeOnNewDataCallback (NativeOnNewData);
            managedCallback = managedOnNewData;
        }

        internal void NativeOnNewData (IntPtr data)
        {
            DepthData depthData = DepthDataConverter.FromNativeData (data);
            managedCallback (depthData);
        }
    }
    //\endcond
}
