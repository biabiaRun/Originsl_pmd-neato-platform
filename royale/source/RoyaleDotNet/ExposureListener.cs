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
    /// A compatibility wrapper around IExposureListener2, which should be used instead.
    ///
    /// Please note that the callback is only made when autoexposure is enabled; when refactoring
    /// code to use the new interface, consider the possibility that the code you're refactoring has
    /// never been called.
    /// </summary>
    // The RegisterExposureListener(IEL) function is marked obsolete, adding an obsolete attribute
    // here too only creates duplicate warnings.
    public interface IExposureListener
    {
        void OnNewExposure (UInt32[] exposureTimes);
    }

    //\cond HIDDEN_SYMBOLS
    internal class ExposureListener : IExposureListener2
    {
        internal IExposureListener listener;

        internal ExposureListener (IExposureListener l)
        {
            listener = l;
        }

        public void OnNewExposure (UInt32 exposureTime, UInt16 streamId)
        {
            UInt32[] times = {exposureTime};
            listener.OnNewExposure (times);
        }
    }
    //\endcond
}
