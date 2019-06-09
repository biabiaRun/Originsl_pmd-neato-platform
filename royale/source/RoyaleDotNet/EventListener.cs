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
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace RoyaleDotNet
{
    public interface IEventListener
    {
        /// <summary>
        /// Will be called when an event occurs.
        ///
        /// Note there are some constraints on what the user is allowed to do
        /// in the callback.
        /// - Actually the royale API does not claim to be reentrant (and probably isn't),
        ///   so the user is not supposed to call any API function from this callback.
        /// - Deleting the ICameraDevice from the callback will most certainly
        ///   lead to a deadlock.
        ///   This has the interesting side effect that calling exit() or equivalent
        ///   from the callback may cause issues.
        /// </summary>
        /// <param name="royaleEvent">The event.</param>
        void OnEvent (Event royaleEvent);
    }

    //\cond HIDDEN_SYMBOLS
    internal class EventListener
    {
        [UnmanagedFunctionPointer (CallingConvention.Cdecl)]
        internal delegate void NativeEventCallback (UInt32 type, UInt32 severity, IntPtr description);
        internal delegate void ManagedEventCallback (Event royaleEvent);

        internal NativeEventCallback nativeEvent;
        internal ManagedEventCallback managedEvent;

        internal EventListener (ManagedEventCallback onEventCB)
        {
            nativeEvent = new NativeEventCallback (NativeEvent);
            managedEvent = onEventCB;
        }

        internal void NativeEvent (UInt32 type, UInt32 severity, IntPtr description)
        {
            Event royaleEvent = new Event ( (Event.EventType) type, (Event.EventSeverity) severity, Marshal.PtrToStringAnsi (description));
            managedEvent (royaleEvent);
        }
    }
    //\endcond
}
