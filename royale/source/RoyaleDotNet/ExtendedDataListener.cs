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
    public interface IExtendedDataListener
    {
        /// <summary>
        /// Will be called on every frame update by the royale framework
        /// </summary>
        /// <param name="data">ExtendedData of the captured frame.</param>
        void OnNewData (ExtendedData data);
    }

    //\cond HIDDEN_SYMBOLS
    internal class ExtendedDataListener
    {
        [UnmanagedFunctionPointer (CallingConvention.Cdecl)]
        internal delegate void NativeOnNewExtendedDataCallback (IntPtr nativeExtendedData);
        internal delegate void ManagedOnNewExtendedDataCallback (ExtendedData data);

        internal NativeOnNewExtendedDataCallback nativeCallback;
        internal ManagedOnNewExtendedDataCallback managedCallback;


        internal ExtendedDataListener (ManagedOnNewExtendedDataCallback managedOnNewData)
        {
            nativeCallback = new NativeOnNewExtendedDataCallback (NativeOnNewData);
            managedCallback = managedOnNewData;
        }

        internal void NativeOnNewData (IntPtr nativeExtendedData)
        {
            ExtendedData extendedData = new ExtendedData (nativeExtendedData);
            managedCallback (extendedData);
        }
    }
    //\endcond
}
